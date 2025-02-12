#ifndef CREATE_TABLE_COMMAND_HANDLER_H
#define CREATE_TABLE_COMMAND_HANDLER_H

#include "CommandHandler.h"
#include <sstream>
#include "RoktResponseService.h"
#include "Utils.h" // pour trim()

/**
 * @brief Gère la commande "CREATE TABLE <dataset>;".
 */
class CreateTableCommandHandler : public CommandHandler {
public:
    CreateTableCommandHandler(RoktService *service) : CommandHandler(service) {}
    virtual RoktResponseObject* handle(const std::string &command) override {
        if (command.find("CREATE TABLE") == 0) {
            std::istringstream iss(command);
            std::string token, dataset;
            // On s'attend à "CREATE TABLE <dataset>;"
            iss >> token; // CREATE
            iss >> token; // TABLE
            iss >> dataset;
            dataset = trim(dataset);
            // Appel de la méthode create() du service pour créer un dataset SIMPLE
            return this->service->create(dataset, "SIMPLE");
        }
        return CommandHandler::handle(command);
    }
};

#endif // CREATE_TABLE_COMMAND_HANDLER_H
