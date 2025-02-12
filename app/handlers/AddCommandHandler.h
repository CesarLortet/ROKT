#ifndef ADD_COMMAND_HANDLER_H
#define ADD_COMMAND_HANDLER_H

#include "CommandHandler.h"
#include <regex>
#include <nlohmann/json.hpp>
#include "RoktResponseService.h"
#include "RoktDataset.h"
#include "RoktService.h"
#include "Utils.h" // pour trim()

/**
 * @brief Gère la commande "ADD { ... } [UNIQUE field] IN dataset;".
 *
 * Vérifie si un champ UNIQUE est spécifié, et si oui, assure qu'il n'existe pas déjà.
 */
class AddCommandHandler : public CommandHandler {
public:
    AddCommandHandler(RoktService *service) : CommandHandler(service) {}
    virtual RoktResponseObject* handle(const std::string &command) override {
        std::regex pattern("^ADD\\s*(\\{.*\\})\\s*(?:UNIQUE\\s+(\\S+))?\\s*IN\\s+(\\S+);$", std::regex::icase);
        std::smatch matches;
        if (!std::regex_match(command, matches, pattern)) {
            return CommandHandler::handle(command);
        }
        std::string jsonBlock = matches[1].str();
        std::string uniqueField = (matches.size() > 2 && matches[2].matched) ? matches[2].str() : "";
        std::string dataset = matches[3].str();
        nlohmann::json newData;
        try {
            newData = nlohmann::json::parse(jsonBlock);
        } catch (...) {
            return RoktResponseService::response(11, "JSON invalide");
        }
        if (!uniqueField.empty()) {
            if (!newData.contains(uniqueField)) {
                return RoktResponseService::response(12, "Champ unique '" + uniqueField + "' absent");
            }
            auto newUniqueValue = newData[uniqueField];
            RoktDataset datasetObj = this->service->from(dataset);
            nlohmann::json existingData = datasetObj.readData(); // readData() retourne un json (tableau)
            for (const auto &row : existingData) {
                if (row.contains(uniqueField) && row[uniqueField] == newUniqueValue) {
                    return RoktResponseService::response(10, "Already Exists");
                }
            }
        }
        RoktDataset datasetObj = this->service->from(dataset);
        return datasetObj.insert(newData);
    }
};

#endif // ADD_COMMAND_HANDLER_H
