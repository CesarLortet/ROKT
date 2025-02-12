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
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <sstream>

RoktResponseObject *processRequest(const std::string& req, CommandHandler* handlerChain) {
    LogService::log(std::string("Traitement de la commande: ") + req);
    RoktResponseObject* response = handlerChain->handle(req);
    LogService::log(std::string("Réponse générée: ") + response->getResponse());
    return response;
}

int main() {
    try {
        Config config("config.json");
        RoktService *roktService = new RoktService(".", new EncryptService(config.passphrase, config.iv));

        // Création des handlers pour les commandes
        CreateTableCommandHandler *createHandler = new CreateTableCommandHandler(roktService);
        AddCommandHandler *addHandler = new AddCommandHandler(roktService);
        GetCommandHandler *getHandler = new GetCommandHandler(roktService);
        RemoveCommandHandler *removeHandler = new RemoveCommandHandler(roktService);
        EmptyCommandHandler *emptyHandler = new EmptyCommandHandler(roktService);
        DeleteCommandHandler *deleteHandler = new DeleteCommandHandler(roktService);
        CountCommandHandler *countHandler = new CountCommandHandler(roktService);
        ChangeCommandHandler *changeHandler = new ChangeCommandHandler(roktService);

        // Chaînage des handlers (l'ordre est à ajuster selon vos besoins)
        createHandler->setNext(addHandler);
        addHandler->setNext(getHandler);
        getHandler->setNext(removeHandler);
        removeHandler->setNext(emptyHandler);
        emptyHandler->setNext(deleteHandler);
        deleteHandler->setNext(countHandler);
        countHandler->setNext(changeHandler);
        // La chaîne se termine ici ; si aucune commande n'est reconnue, on renvoie "Commande non reconnue."

        // Démarrage du serveur socket sur le port 8080
        int server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == 0)
            throw std::runtime_error("Échec de la création du socket.");
        int opt = 1;
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
            throw std::runtime_error("setsockopt a échoué.");
        struct sockaddr_in address;
        int addrlen = sizeof(address);
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(8080);
        if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
            throw std::runtime_error("Échec du bind du socket.");
        if (listen(server_fd, 3) < 0)
            throw std::runtime_error("Échec de l'écoute du socket.");

        LogService::log("Serveur socket démarré sur le port 8080. En attente de connexions...");

        while (true) {
            int new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
            if (new_socket < 0) {
                LogService::log("Erreur lors de l'acceptation d'une connexion.");
                continue;
            }
            char buffer[2048] = {0};
            int valread = read(new_socket, buffer, 2048);
            std::string request(buffer, valread);
            LogService::log("Requête reçue: " + request);
            RoktResponseObject *response = processRequest(request, createHandler);
            std::string responseStr = response->getResponse();
            delete response;
            send(new_socket, responseStr.c_str(), responseStr.size(), 0);
            LogService::log("Réponse envoyée.");
            close(new_socket);
        }
        close(server_fd);
    } catch (std::exception& ex) {
        std::cerr << "Exception: " << ex.what() << std::endl;
    }
    return 0;
}
