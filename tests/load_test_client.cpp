#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <vector>
#include <iostream>
#include <string>
#include <chrono>
#include <random>
#include <mutex>
#include <atomic>
#include <iomanip>
#include <arpa/inet.h>

// Constantes pour augmenter la charge
const std::string SERVER_IP = "127.0.0.1";
const int SERVER_PORT = 8080;
const int TOTAL_CLIENTS = 5000;        // Plus de clients pour saturer
const int REQUESTS_PER_CLIENT = 100;  // Requêtes par client
const int MAX_DELAY_MS = 10;           // Délai minimal pour maximiser la concurrence

// Commandes complexes
const std::vector<std::string> COMMANDS = {
    "GET name IN users WHERE id IS 1;",
    "GET name IN users WHERE id NOT 1;",
    "GET name IN users WHERE favorite_flavour HAS vanilla;",
    "GET * IN users WHERE age IS 30;",
    "GET * IN users WHERE details.age IS 30;",
    "GET * IN users WHERE details.city IS Paris;",
    "GET name IN users ORDER BY details.age DESC LIMIT 5;",
    "GET name IN users WHERE favorite_flavour HAS vanilla GROUP BY details.city ORDER BY id DESC LIMIT 5;",
    "GET name AS username IN users ORDER BY details.age DESC LIMIT 5;",
    "GET name IN users WHERE age IS 30 AND details.city IS Paris;",
    "UPDATE age = 35 WHERE name IS Alice AND id IS 1 IN users;",
    "UPDATE age = 30 WHERE name IS Alice AND id IS 1 IN users;",
    "GET * IN users WHERE id IS 1;",
    "COUNT users;",
    "COUNT users age:30;",
    "COUNT users details.age:30;"
};

// Statistiques globales
struct Statistics {
    std::atomic<int> successful_requests{0};
    std::atomic<int> failed_requests{0};
    std::atomic<int> rejected_requests{0};
    std::atomic<long long> total_latency_us{0};
    std::mutex stats_mutex;

    void log(const std::string& message) {
        std::lock_guard<std::mutex> lock(stats_mutex);
        std::cout << message << "\n";
    }
};

/**
 * @brief Génère un délai aléatoire pour simuler une charge variable.
 * @return Délai en millisecondes entre 0 et MAX_DELAY_MS.
 */
int randomDelay() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, MAX_DELAY_MS);
    return dis(gen);
}

/**
 * @brief Simule un client envoyant des requêtes au serveur ROKT.
 * @param client_id Identifiant unique du client.
 * @param stats Référence vers les statistiques globales.
 */
void simulateClient(int client_id, Statistics& stats) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, COMMANDS.size() - 1);

    for (int i = 0; i < REQUESTS_PER_CLIENT; ++i) {
        int client_socket = socket(AF_INET, SOCK_STREAM, 0);
        bool socket_creation_failed = (client_socket < 0);
        if (socket_creation_failed) {
            stats.failed_requests++;
            stats.log("Client " + std::to_string(client_id) + " : Échec création socket");
            continue;
        }

        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(SERVER_PORT);
        server_addr.sin_addr.s_addr = inet_addr(SERVER_IP.c_str());

        bool connect_failed = (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0);
        if (connect_failed) {
            stats.failed_requests++;
            stats.log("Client " + std::to_string(client_id) + " : Échec connexion");
            close(client_socket);
            continue;
        }

        std::string command = COMMANDS[dis(gen)];
        
        // Mesure de latence précise
        auto start_time = std::chrono::high_resolution_clock::now();
        bool send_failed = (send(client_socket, command.c_str(), command.size(), 0) < 0);
        if (send_failed) {
            stats.failed_requests++;
            stats.log("Client " + std::to_string(client_id) + " : Échec envoi '" + command + "'");
            close(client_socket);
            continue;
        }

        char buffer[2048] = {0};
        int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        auto end_time = std::chrono::high_resolution_clock::now();
        auto latency_us = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();

        bool receive_failed = (bytes_received <= 0);
        if (receive_failed) {
            stats.failed_requests++;
            stats.log("Client " + std::to_string(client_id) + " : Pas de réponse pour '" + command + "'");
        } else {
            std::string response(buffer, bytes_received);
            bool request_rejected = (response.find("503") != std::string::npos);
            if (request_rejected) {
                stats.rejected_requests++;
            } else {
                stats.successful_requests++;
                stats.total_latency_us += latency_us;
            }
            /*
            stats.log("Client " + std::to_string(client_id) + " : Requête '" + command + "' -> Réponse : " + response + 
                    " (Latence : " + std::to_string(latency_us) + "µs)");
            */
        }

        close(client_socket);
        std::this_thread::sleep_for(std::chrono::milliseconds(randomDelay()));
    }
}

/**
 * @brief Affiche les statistiques finales du test de charge.
 * @param stats Statistiques globales collectées pendant le test.
 */
void printStatistics(const Statistics& stats, double total_duration_ms) {
    int total_requests = stats.successful_requests + stats.failed_requests + stats.rejected_requests;
    double avg_latency_ms = stats.successful_requests > 0 ? static_cast<double>(stats.total_latency_us) / stats.successful_requests / 1000.0 : 0.0;

    std::cout << "\n--- Statistiques du test de charge ---\n";
    std::cout << "Requêtes totales envoyées : " << total_requests << "\n";
    std::cout << "Requêtes réussies : " << stats.successful_requests << "\n";
    std::cout << "Requêtes échouées : " << stats.failed_requests << "\n";
    std::cout << "Requêtes rejetées (surcharge) : " << stats.rejected_requests << "\n";
    std::cout << std::fixed << std::setprecision(10) << "Latence moyenne (ms) : " << avg_latency_ms << "\n";
    std::cout << "Temps total d'exécution (ms) : " << total_duration_ms << "\n";
}

/**
 * @brief Point d'entrée principal pour le test de charge.
 * Lance les clients pour simuler une charge réseau sur le serveur ROKT.
 * @return 0 en cas de succès.
 */
int main() {
    std::cout << "Démarrage du test de charge avec " << TOTAL_CLIENTS << " clients, " 
              << REQUESTS_PER_CLIENT << " requêtes chacun...\n";

    auto start_time = std::chrono::high_resolution_clock::now();

    Statistics stats;
    std::vector<std::thread> clients;

    for (int i = 0; i < TOTAL_CLIENTS; ++i) {
        clients.emplace_back(simulateClient, i, std::ref(stats));
    }

    for (auto& client : clients) {
        client.join();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    double total_duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    // Affichage des statistiques avec le temps total
    printStatistics(stats, total_duration_ms);
    std::cout << "Test de charge terminé.\n";

    return 0;
}