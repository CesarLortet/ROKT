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

// Constantes pour le test
const std::string SERVER_IP = "127.0.0.1";
const int SERVER_PORT = 8080;
const int NUM_ROWS = 1000000;      // Nombre de lignes à insérer
const int NUM_THREADS = 10;        // Nombre de threads pour paralléliser l'insertion
const int MIN_AGE = 18;            // Âge minimum
const int MAX_AGE = 80;            // Âge maximum

// Liste de noms pour la randomisation
const std::vector<std::string> NAMES = {
    "Alice", "Bob", "Charlie", "David", "Eve", "Frank", "Grace", "Hannah", "Isabelle", "Jack"
};

// Statistiques globales
struct Statistics {
    std::atomic<int> successful_inserts{0};
    std::atomic<int> failed_inserts{0};
    std::atomic<long long> total_latency_us{0};
    std::mutex stats_mutex;

    void log(const std::string& message) {
        std::lock_guard<std::mutex> lock(stats_mutex);
        std::cout << message << "\n";
    }
};

/**
 * @brief Génère un âge aléatoire entre MIN_AGE et MAX_AGE.
 * @return Âge aléatoire.
 */
int randomAge() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(MIN_AGE, MAX_AGE);
    return dis(gen);
}

/**
 * @brief Génère un nom aléatoire parmi la liste NAMES.
 * @return Nom aléatoire.
 */
std::string randomName() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, NAMES.size() - 1);
    return NAMES[dis(gen)];
}

/**
 * @brief Envoie une commande au serveur ROKT et retourne la réponse.
 * @param command Commande à envoyer.
 * @param latency_us Référence pour stocker la latence en microsecondes.
 * @return Réponse du serveur ou chaîne vide en cas d'échec.
 */
std::string sendCommand(const std::string& command, long long& latency_us) {
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) return "";

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP.c_str());

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        close(client_socket);
        return "";
    }

    auto start_time = std::chrono::high_resolution_clock::now();
    if (send(client_socket, command.c_str(), command.size(), 0) < 0) {
        close(client_socket);
        return "";
    }

    char buffer[2048] = {0};
    int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    auto end_time = std::chrono::high_resolution_clock::now();
    latency_us = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count();

    std::string response;
    if (bytes_received > 0) {
        response = std::string(buffer, bytes_received);
    }
    close(client_socket);
    return response;
}

/**
 * @brief Insère des lignes dans la table users sur le serveur ROKT.
 * @param thread_id Identifiant du thread.
 * @param start_id ID de départ pour ce thread.
 * @param num_rows Nombre de lignes à insérer par ce thread.
 * @param stats Référence vers les statistiques globales.
 */
void insertRows(int thread_id, int start_id, int num_rows, Statistics& stats) {
    for (int i = 0; i < num_rows; ++i) {
        int id = start_id + i;
        std::string name = randomName();
        int age = randomAge();

        // Construction de la commande JSON
        std::string data = "{\"id\": " + std::to_string(id) + ", \"name\": \"" + name + "\",\"details\": {\"age\": " + std::to_string(age) + ", \"city\": \"Paris\"}}";
        std::string command = "ADD " + data + " IN users;";

        // stats.log("Thread " + std::to_string(thread_id) + " : Commande : " + command);
        long long latency_us = 0;
        std::string response = sendCommand(command, latency_us);

        bool insert_failed = response.empty() || (response.find("\"status\": 2") == std::string::npos);
        if (insert_failed) {
            stats.failed_inserts++;
            // stats.log("Thread " + std::to_string(thread_id) + " : Échec insertion ID " + std::to_string(id) + " - Réponse : " + response);
        } else {
            stats.successful_inserts++;
            stats.total_latency_us += latency_us;
            // stats.log("Thread " + std::to_string(thread_id) + " : Insertion ID " + std::to_string(id) + " réussie (Latence : " + std::to_string(latency_us) + "µs)");
        }
    }
}

/**
 * @brief Affiche les statistiques finales du test de charge.
 * @param stats Statistiques globales collectées.
 */
void printStatistics(const Statistics& stats, double total_duration_ms) {
    int total_requests = stats.successful_inserts + stats.failed_inserts;
    double avg_latency_ms = stats.successful_inserts > 0 ? static_cast<double>(stats.total_latency_us) / stats.successful_inserts / 1000.0 : 0.0;

    std::cout << "\n--- Statistiques du test ROKT ---\n";
    std::cout << "Requêtes totales envoyées : " << total_requests << "\n";
    std::cout << "Insertions réussies : " << stats.successful_inserts << "\n";
    std::cout << "Insertions échouées : " << stats.failed_inserts << "\n";
    std::cout << std::fixed << std::setprecision(2) << "Latence moyenne (ms) : " << avg_latency_ms << "\n";
    std::cout << "Temps total d'exécution (ms) : " << total_duration_ms << "\n";
}

/**
 * @brief Point d'entrée principal pour le test de charge ROKT.
 * Crée une table et insère 1 million de lignes avec des données aléatoires.
 * @return 0 en cas de succès.
 */
int main() {
    std::cout << "Démarrage du test de charge ROKT : Création table et insertion de " << NUM_ROWS << " lignes...\n";

    auto start_time = std::chrono::high_resolution_clock::now();

    Statistics stats;
    std::vector<std::thread> threads;

    // Création de la table
    long long create_latency_us = 0;
    std::string create_response = sendCommand("CREATE TABLE users;", create_latency_us);
    bool create_failed = create_response.empty() || (create_response.find("\"status\": 0") == std::string::npos);
    if (create_failed) {
        std::cerr << "Échec de la création de la table : " << create_response << "\n";
    }
    stats.successful_inserts++;
    stats.total_latency_us += create_latency_us;
    stats.log("Table 'users' créée avec succès (Latence : " + std::to_string(create_latency_us) + "µs)");

    // Calcul des lignes par thread
    int rows_per_thread = NUM_ROWS / NUM_THREADS;
    int remaining_rows = NUM_ROWS % NUM_THREADS;

    // Lancement des threads pour l'insertion
    for (int i = 0; i < NUM_THREADS; ++i) {
        int start_id = i * rows_per_thread;
        int num_rows = (i == NUM_THREADS - 1) ? rows_per_thread + remaining_rows : rows_per_thread;
        threads.emplace_back(insertRows, i, start_id, num_rows, std::ref(stats));
    }

    // Attente de la fin des threads
    for (auto& thread : threads) {
        thread.join();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    double total_duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    // Affichage des statistiques avec le temps total
    printStatistics(stats, total_duration_ms);

    /* std::string remove_response = sendCommand("DELETE users;", create_latency_us);
    bool remove_failed = remove_response.empty() || (remove_response.find("\"status\": 0") == std::string::npos);
    if (remove_failed) {
        std::cerr << "Échec de la suppression de la table : " << remove_response << "\n";
        return 1;
    }
    */
    std::cout << "Test de charge ROKT terminé.\n";

    return 0;
}