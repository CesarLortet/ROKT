#ifndef SYNC_SERVICE_H
#define SYNC_SERVICE_H

#include <sys/epoll.h>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <memory>
#include <unordered_map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sstream>
#include <chrono>
#include <functional>
#include "CommandHandler.h"
#include "LogService.h"
#include "RoktResponseService.h"

// Définition des constantes absolues pour la configuration du service
#define DEFAULT_MAX_WORKERS 8              // Nombre maximum de workers par défaut
#define DEFAULT_MAX_TASK_QUEUE_SIZE 100    // Taille maximale par défaut de la file d'attente des tâches
#define BACKPRESSURE_THRESHOLD_FACTOR 2    // Facteur pour calculer le seuil de temporisation (maxWorkers_ * FACTOR)
#define SOCKET_TIMEOUT_SEC 10             // Timeout en secondes pour les opérations de lecture/écriture sur socket
#define PROCESSING_TIMEOUT_MS 5000        // Timeout maximal en millisecondes pour traiter une commande
#define BACKPRESSURE_DELAY_MS 100         // Délai en millisecondes appliqué lors de la temporisation en cas de surcharge

/**
 * @brief Classe SyncService gérant la synchronisation des connexions réseau et le traitement des commandes.
 *
 * Cette classe utilise epoll pour surveiller les sockets et un pool de threads workers pour traiter les
 * commandes en parallèle selon leurs priorités. Elle associe chaque commande à un handler via une
 * table de dispatch (HandlerMap) pour une exécution rapide.
 */
class SyncService {
public:
    // Structure représentant une tâche dans la file d'attente
    struct Task {
        int socket;           // Socket client associé à la tâche
        std::string request;  // Requête complète à traiter
        int priority;         // Priorité de la tâche (plus élevé = traité en premier)
        
        /**
         * @brief Constructeur personnalisé pour une tâche.
         * @param s Socket client.
         * @param r Requête à traiter.
         * @param p Priorité de la tâche.
         */
        Task(int s, const std::string& r, int p) : socket(s), request(r), priority(p) {}
        
        /**
         * @brief Constructeur par défaut pour une tâche vide.
         */
        Task() : socket(-1), request(""), priority(0) {}
    };

    // Comparateur pour trier les tâches par priorité décroissante dans la priority_queue
    struct TaskComparator {
        /**
         * @brief Compare deux tâches pour déterminer leur ordre dans la file.
         * @param a Première tâche.
         * @param b Deuxième tâche.
         * @return true si a doit être traité après b (priorité plus basse).
         */
        bool operator()(const Task& a, const Task& b) {
            return a.priority < b.priority;
        }
    };

    // Définition du type HandlerMap pour associer les commandes aux handlers
    using HandlerMap = std::unordered_map<std::string, std::unique_ptr<CommandHandler>>;

    /**
     * @brief Constructeur de SyncService.
     * @param server_fd Descripteur du socket serveur.
     * @param handlers Table de dispatch associant les commandes aux handlers.
     * @param maxWorkers Nombre maximum de threads workers (par défaut DEFAULT_MAX_WORKERS).
     * @param maxTaskQueueSize Taille maximale de la file d'attente (par défaut DEFAULT_MAX_TASK_QUEUE_SIZE).
     */
    SyncService(int server_fd, HandlerMap& handlers, int maxWorkers = DEFAULT_MAX_WORKERS, int maxTaskQueueSize = DEFAULT_MAX_TASK_QUEUE_SIZE);

    /**
     * @brief Destructeur de SyncService.
     * Arrête proprement les workers et libère les ressources.
     */
    ~SyncService();

    /**
     * @brief Démarre le service, lançant la surveillance des sockets avec epoll et le traitement des tâches.
     */
    void start();

    /**
     * @brief Arrête proprement le service, stoppant les workers et libérant les ressources réseau.
     */
    void stop();

private:
    int server_fd_;                                                  // Descripteur du socket serveur
    int epoll_fd_;                                                  // Descripteur de l'instance epoll
    HandlerMap& handlers_;                                          // Référence à la table de dispatch des handlers
    std::vector<std::thread> workers_;                              // Pool de threads workers
    std::priority_queue<Task, std::vector<Task>, TaskComparator> taskQueue_; // File d'attente prioritaire des tâches
    std::mutex queueMutex_;                                         // Mutex pour synchroniser l'accès à la file
    std::condition_variable queueCond_;                             // Condition variable pour signaler les tâches disponibles
    volatile bool running_;                                         // Indicateur de l'état d'exécution du service
    int maxWorkers_;                                                // Nombre maximum de threads workers
    int maxTaskQueueSize_;                                          // Taille maximale de la file d'attente

    /**
     * @brief Boucle de traitement exécutée par chaque thread worker.
     * Récupère et traite les tâches de la file d'attente selon leur priorité.
     */
    void workerLoop();

    /**
     * @brief Surveille les événements réseau avec epoll et dispatche les tâches aux workers.
     */
    void processEpollEvents();

    /**
     * @brief Gère l'acceptation et la configuration d'une nouvelle connexion client.
     * @param address Structure sockaddr_in pour stocker l'adresse du client.
     * @param addrlen Longueur de l'adresse (passée à accept).
     */
    void handleNewConnection(struct sockaddr_in& address, socklen_t addrlen);

    /**
     * @brief Lit et traite les données reçues d'un socket client.
     * @param client_socket Descripteur du socket client.
     */
    void handleClientData(int client_socket);

    /**
     * @brief Détermine la priorité d'une commande à partir de sa chaîne de caractères.
     * @param command Requête complète reçue.
     * @return Priorité numérique (ex. 10 pour CREATE, 5 pour ADD, 1 pour GET).
     */
    int getCommandPriority(const std::string& command);
};

#endif // SYNC_SERVICE_H