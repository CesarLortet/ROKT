#include "CreateTableCommandHandler.h"
#include "AddCommandHandler.h"
#include "GetCommandHandler.h"
#include "RemoveCommandHandler.h"
#include "EmptyCommandHandler.h"
#include "DeleteCommandHandler.h"
#include "ChangeCommandHandler.h"
#include "CountCommandHandler.h"
#include "LogService.h"
#include "RoktResponseService.h"
#include "RoktService.h"
#include "Config.h"
#include "EncryptService.h"
#include "CommandHandler.h"
#include "Utils.h"
#include "SyncService.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h> // Pour TCP_NODELAY
#include <unistd.h>
#include <signal.h>
#include <memory>
#include <iostream>
#include <sstream>
#include <condition_variable>
#include <mutex>
#include <unordered_map>
#include <cerrno> // Pour strerror

// Signal de poursuite ou d'arrêt du serveur
volatile sig_atomic_t keep_running = 1;

// Condition variable et mutex pour une attente efficace de l'arrêt
std::condition_variable stop_condition;
std::mutex stop_mutex;

// Définition de la map pour associer les commandes aux handlers
using HandlerMap = std::unordered_map<std::string, std::unique_ptr<CommandHandler>>;

/**
 * @brief Gère les signaux d'arrêt (SIGINT, SIGTERM) pour un arrêt propre du serveur.
 * @param sig Le numéro du signal reçu.
 */
void signal_handler(int sig) {
    keep_running = 0;
    stop_condition.notify_one();
    LogService::log("Signal reçu, arrêt du serveur en cours...");
}

/**
 * @brief Crée et configure la map des handlers pour traiter les différentes commandes.
 * @param roktService Pointeur vers l'instance de RoktService utilisée par les handlers.
 * @return Une HandlerMap contenant les associations commande-handler.
 */
HandlerMap createHandlerMap(RoktService* roktService) {
    HandlerMap handlers;
    handlers["CREATE"] = std::make_unique<CreateTableCommandHandler>(roktService);
    handlers["ADD"] = std::make_unique<AddCommandHandler>(roktService);
    handlers["GET"] = std::make_unique<GetCommandHandler>(roktService);
    handlers["REMOVE"] = std::make_unique<RemoveCommandHandler>(roktService);
    handlers["EMPTY"] = std::make_unique<EmptyCommandHandler>(roktService);
    handlers["DELETE"] = std::make_unique<DeleteCommandHandler>(roktService);
    handlers["COUNT"] = std::make_unique<CountCommandHandler>(roktService);
    handlers["CHANGE"] = std::make_unique<ChangeCommandHandler>(roktService);
    return handlers;
}

/**
 * @brief Point d'entrée principal du programme.
 * Configure et démarre le serveur ROKT, qui écoute les commandes réseau via un socket TCP.
 * @return Code de retour : 0 pour succès, 1 pour échec.
 */
int main() {
    // Enregistrement des gestionnaires de signaux pour SIGINT (Ctrl+C) et SIGTERM
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // Chargement de la configuration depuis config.json avec vérification
    Config config("config.json");
    bool config_invalid = !config.isValid();
    if (config_invalid) {
        LogService::log("Configuration invalide. Utilisation des valeurs par défaut ou arrêt recommandé.");
        // Note : On pourrait retourner 1 ici si une config valide est critique
    }

    // Initialisation du service de chiffrement et de RoktService
    auto encryptService = std::make_shared<EncryptService>(config.encryption.passphrase, config.encryption.iv);
    auto roktService = std::make_unique<RoktService>(".", encryptService);

    // Création de la table de dispatch pour les handlers
    HandlerMap handlers = createHandlerMap(roktService.get());

    // Création du socket serveur
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    bool socket_creation_failed = (server_fd < 0);
    if (socket_creation_failed) {
        LogService::log("Échec de la création du socket : " + std::string(strerror(errno)));
        return 1;
    }

    // Configuration du socket : réutilisation d'adresse/port et désactivation de Nagle
    int reuse_opt = 1;
    bool setsockopt_reuse_failed = (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &reuse_opt, sizeof(reuse_opt)) < 0);
    if (setsockopt_reuse_failed) {
        LogService::log("setsockopt (reuse) a échoué : " + std::string(strerror(errno)));
        close(server_fd);
        return 1;
    }

    int nodelay_flag = 1;
    setsockopt(server_fd, IPPROTO_TCP, TCP_NODELAY, &nodelay_flag, sizeof(nodelay_flag)); // Réduit la latence

    // Configuration de l'adresse du serveur
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(config.network.port);

    // Liaison du socket à l'adresse
    bool bind_failed = (bind(server_fd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0);
    if (bind_failed) {
        LogService::log("Échec du bind du socket : " + std::string(strerror(errno)));
        close(server_fd);
        return 1;
    }

    // Mise en écoute du socket avec le backlog configuré
    bool listen_failed = (listen(server_fd, config.network.backlog) < 0);
    if (listen_failed) {
        LogService::log("Échec de l'écoute du socket : " + std::string(strerror(errno)));
        close(server_fd);
        return 1;
    }

    // Log de démarrage
    std::ostringstream startupMsg;
    startupMsg << "Serveur socket démarré sur le port " << config.network.port << ". En attente de connexions...";
    LogService::log(startupMsg.str());

    // Démarrage du service de synchronisation avec la HandlerMap
    SyncService syncService(server_fd, handlers, config.thread.maxWorkers, config.thread.maxTaskQueueSize);
    syncService.start();

    // Attente efficace de l'arrêt via condition variable
    std::unique_lock<std::mutex> lk(stop_mutex);
    stop_condition.wait(lk, [] { return !keep_running; });

    // Arrêt propre du service et fermeture du socket
    syncService.stop();
    LogService::log("Arrêt propre du serveur. Fermeture du socket principal.");
    close(server_fd);

    return 0;
}