#include "SyncService.h"

/**
 * @brief Constructeur de SyncService.
 * Initialise epoll, configure le socket serveur, et lance les threads workers.
 * @param server_fd Descripteur du socket serveur.
 * @param handlers Table de dispatch associant les commandes aux handlers.
 * @param maxWorkers Nombre maximum de threads workers.
 * @param maxTaskQueueSize Taille maximale de la file d'attente des tâches.
 */
SyncService::SyncService(int server_fd, HandlerMap& handlers, int maxWorkers, int maxTaskQueueSize)
    : server_fd_(server_fd), handlers_(handlers), running_(true), maxWorkers_(maxWorkers), maxTaskQueueSize_(maxTaskQueueSize) {
    // Limite le nombre de workers à un maximum raisonnable
    bool max_workers_exceeded = (maxWorkers_ > 64);
    if (max_workers_exceeded) {
        LogService::log("Nombre de workers trop élevé. Limitation à 64.");
        maxWorkers_ = 64;
    }

    // Initialisation de l'instance epoll
    epoll_fd_ = epoll_create1(0);
    bool epoll_create_failed = (epoll_fd_ < 0);
    if (epoll_create_failed) {
        LogService::log("Échec de la création d'epoll dans SyncService.");
        return;
    }

    // Ajout du socket serveur à epoll
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = server_fd_;
    bool epoll_add_server_failed = (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, server_fd_, &ev) < 0);
    if (epoll_add_server_failed) {
        LogService::log("Échec de l'ajout du socket serveur à epoll dans SyncService.");
        close(epoll_fd_);
        epoll_fd_ = -1;
        return;
    }

    // Lancement des threads workers
    for (int i = 0; i < maxWorkers_; ++i) {
        workers_.emplace_back(&SyncService::workerLoop, this);
    }

    std::ostringstream logMsg;
    logMsg << "SyncService démarré avec " << maxWorkers_ << " workers et une file max de " << maxTaskQueueSize_ << " tâches.";
    LogService::log(logMsg.str());
}

/**
 * @brief Destructeur de SyncService.
 * Arrête proprement les workers et libère les ressources epoll.
 */
SyncService::~SyncService() {
    stop();
    if (epoll_fd_ >= 0) {
        close(epoll_fd_);
    }
}

/**
 * @brief Démarre le service SyncService.
 * Vérifie l'initialisation d'epoll avant de lancer la surveillance des événements.
 */
void SyncService::start() {
    bool epoll_fd_invalid = (epoll_fd_ < 0);
    if (epoll_fd_invalid) {
        LogService::log("SyncService non initialisé correctement. Arrêt.");
        return;
    }
    processEpollEvents();
}

/**
 * @brief Arrête proprement le service SyncService.
 * Signale aux threads workers de s'arrêter et les attend pour terminer.
 */
void SyncService::stop() {
    running_ = false;
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        queueCond_.notify_all();
    }
    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

/**
 * @brief Boucle de travail exécutée par chaque thread worker.
 * Récupère les tâches de la file prioritaire et les traite avec le handler approprié.
 */
void SyncService::workerLoop() {
    while (running_) {
        Task task;
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            bool queue_is_empty = taskQueue_.empty();
            if (queue_is_empty) {
                queueCond_.wait(lock, [this] { return !taskQueue_.empty() || !running_; });
                bool should_stop = (!running_ && taskQueue_.empty());
                if (should_stop) break;
            }
            if (!taskQueue_.empty()) {
                task = taskQueue_.top();
                taskQueue_.pop();
            }
        }
        bool task_is_valid = (task.socket >= 0);
        if (task_is_valid) {
            std::ostringstream logMsg;
            logMsg << "Traitement de la commande dans thread: " << task.request;
            LogService::log(logMsg.str());

            // Mesure du temps de traitement
            auto startTime = std::chrono::steady_clock::now();
            std::string command = task.request.substr(0, task.request.find(' '));
            auto it = handlers_.find(command);
            bool handler_found = (it != handlers_.end());
            std::unique_ptr<ROKT::ResponseObject> response;
            if (handler_found) {
                response = it->second->handle(task.request);
            } else {
                response = ROKT::ResponseService::response(1, "Commande non reconnue : " + command);
            }
            auto endTime = std::chrono::steady_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

            bool processing_timeout_exceeded = (duration > PROCESSING_TIMEOUT_MS);
            if (processing_timeout_exceeded) {
                LogService::log("Traitement trop long (> " + std::to_string(PROCESSING_TIMEOUT_MS) + "ms).");
                response = ROKT::ResponseService::response(504, "Request timeout");
            }

            std::string responseStr = response->getResponse();
            logMsg.str("");
            logMsg << "Réponse générée dans thread: " << responseStr;
            LogService::log(logMsg.str());

            bool send_failed = (send(task.socket, responseStr.c_str(), responseStr.size(), 0) < 0);
            if (send_failed) {
                LogService::log("Erreur lors de l'envoi de la réponse dans thread.");
            }
            close(task.socket);
        }
    }
}

/**
 * @brief Surveille les événements réseau avec epoll et dispatche les tâches aux workers.
 * Gère les nouvelles connexions et les données des clients existants.
 */
void SyncService::processEpollEvents() {
    struct epoll_event events[128];
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);

    while (running_) {
        int nfds = epoll_wait(epoll_fd_, events, 128, 1000);
        bool epoll_wait_failed = (nfds < 0);
        if (epoll_wait_failed) {
            bool interrupted_by_signal = (errno == EINTR);
            if (interrupted_by_signal) continue;
            LogService::log("Erreur dans epoll_wait. Passage au cycle suivant.");
            continue;
        }

        for (int i = 0; i < nfds; ++i) {
            int fd = events[i].data.fd;
            bool is_server_socket = (fd == server_fd_);
            if (is_server_socket) {
                handleNewConnection(address, addrlen);
            } else {
                handleClientData(fd);
            }
        }
    }
}

/**
 * @brief Gère l'acceptation et la configuration d'une nouvelle connexion client.
 * @param address Structure sockaddr_in pour stocker l'adresse du client.
 * @param addrlen Longueur de l'adresse (passée à accept).
 */
void SyncService::handleNewConnection(struct sockaddr_in& address, socklen_t addrlen) {
    int new_socket = accept(server_fd_, (struct sockaddr*)&address, &addrlen);
    bool accept_failed = (new_socket < 0);
    if (accept_failed) {
        LogService::log("Erreur lors de l'acceptation d'une connexion.");
        return;
    }

    // Ajout de timeout au socket
    struct timeval tv;
    tv.tv_sec = SOCKET_TIMEOUT_SEC;
    tv.tv_usec = 0;
    setsockopt(new_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(new_socket, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLONESHOT;
    ev.data.fd = new_socket;
    bool epoll_add_failed = (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, new_socket, &ev) < 0);
    if (epoll_add_failed) {
        LogService::log("Échec de l'ajout du nouveau socket à epoll.");
        close(new_socket);
    }
}

/**
 * @brief Lit et traite les données reçues d'un socket client.
 * Ajoute les tâches à la file d'attente prioritaire si possible.
 * @param client_socket Descripteur du socket client.
 */
void SyncService::handleClientData(int client_socket) {
    char buffer[2048] = {0};
    int valread = read(client_socket, buffer, 2048);
    bool read_failed_or_timeout = (valread <= 0);
    if (read_failed_or_timeout) {
        close(client_socket);
        return;
    }

    std::string request(buffer, valread);
    int priority = getCommandPriority(request);

    std::lock_guard<std::mutex> lock(queueMutex_);
    size_t queueSize = taskQueue_.size();
    bool queue_is_full = ((int)queueSize >= maxTaskQueueSize_);
    if (queue_is_full) {
        LogService::log("File d'attente pleine. Rejet de la requête.");
        std::string response = ROKT::ResponseService::response(503, "Server overloaded")->getResponse();
        send(client_socket, response.c_str(), response.size(), 0);
        close(client_socket);
        return;
    }

    const size_t BACKPRESSURE_THRESHOLD = maxWorkers_ * BACKPRESSURE_THRESHOLD_FACTOR;
    bool backpressure_threshold_exceeded = (queueSize >= BACKPRESSURE_THRESHOLD);
    if (backpressure_threshold_exceeded) {
        LogService::log("File dépasse le seuil de temporisation. Attente de " + std::to_string(BACKPRESSURE_DELAY_MS) + "ms.");
        std::this_thread::sleep_for(std::chrono::milliseconds(BACKPRESSURE_DELAY_MS));
    }

    taskQueue_.push(Task(client_socket, request, priority));
    queueCond_.notify_one();

    bool queue_is_long = (queueSize > BACKPRESSURE_THRESHOLD);
    if (queue_is_long) {
        std::ostringstream logMsg;
        logMsg << "Attention : file d'attente longue (" << queueSize << " tâches). Possible surcharge.";
        LogService::log(logMsg.str());
    }
}

/**
 * @brief Détermine la priorité d'une commande à partir de sa chaîne de caractères.
 * @param command Requête complète reçue.
 * @return Priorité numérique (10 pour CREATE/DELETE, 5 pour ADD/REMOVE/CHANGE, 1 pour GET/COUNT/EMPTY).
 */
int SyncService::getCommandPriority(const std::string& command) {
    size_t spacePos = command.find(' ');
    std::string cmd = (spacePos != std::string::npos) ? command.substr(0, spacePos) : command;

    if (cmd == "CREATE" || cmd == "DELETE") return 10; // Haute priorité
    if (cmd == "ADD" || cmd == "REMOVE" || cmd == "CHANGE") return 5; // Moyenne
    if (cmd == "GET" || cmd == "COUNT" || cmd == "EMPTY") return 1; // Basse
    return 0; // Par défaut
}