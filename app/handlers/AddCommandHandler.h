#ifndef ADD_COMMAND_HANDLER_H
#define ADD_COMMAND_HANDLER_H

#include "CommandHandler.h"
#include <regex>
#include <nlohmann/json.hpp>
#include "RoktResponseService.h"
#include "RoktDataset.h"
#include "RoktService.h"
#include "Utils.h" // pour trim()
#include "LogService.h"

/**
 * @brief Gère la commande "ADD { ... } [UNIQUE field] IN dataset;".
 *
 * Vérifie si un champ UNIQUE est spécifié, et si oui, assure qu'il n'existe pas déjà.
 */
class AddCommandHandler : public CommandHandler {
public:
    AddCommandHandler(RoktService *service) : CommandHandler(service) {}
    virtual std::unique_ptr<ROKT::ResponseObject> handle(const std::string &command) override {
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
            return ROKT::ResponseService::response(11, "JSON invalide");
        }
        if (!uniqueField.empty()) {
            if (!newData.contains(uniqueField)) {
                return ROKT::ResponseService::response(12, "Champ unique '" + uniqueField + "' absent");
            }
            auto newUniqueValue = newData[uniqueField];
            std::shared_ptr<RoktDataset> datasetObj;
            if(this->service->from(dataset, datasetObj)->hasError()) {
                    return ROKT::ResponseService::response(1, "Can't get dataset");
            }
            nlohmann::json existingData = datasetObj->readData(); // readData() retourne un nlohmann::json (tableau)
            for (const auto &row : existingData) {
                if (row.contains(uniqueField) && row[uniqueField] == newUniqueValue) {
                    return ROKT::ResponseService::response(10, "Already Exists");
                }
            }
        }
        std::shared_ptr<RoktDataset> datasetObj;
        if(this->service->from(dataset, datasetObj)->hasError()) {
                return ROKT::ResponseService::response(1, "Can't get dataset");
        }
        return datasetObj->insert(newData);
    }
};

#endif // ADD_COMMAND_HANDLER_H
