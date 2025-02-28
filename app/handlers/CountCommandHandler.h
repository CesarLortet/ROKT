#ifndef COUNT_COMMAND_HANDLER_H
#define COUNT_COMMAND_HANDLER_H

#include "CommandHandler.h"
#include "RoktResponseService.h"
#include "RoktService.h"
#include "Utils.h"           // pour trim()
#include <sstream>
#include <string>
#include <nlohmann/json.hpp>



/**
 * @brief CountCommandHandler traite la commande COUNT en effectuant le comptage directement.
 *
 * Syntaxe attendue :
 *   COUNT <dataset> [<key>:<value>];
 *
 * Si aucune condition n'est donnée, il compte toutes les lignes du dataset.
 * Sinon, il ne compte que les lignes où la valeur du champ spécifié est égale à la valeur donnée.
 * La réponse est encapsulée dans un ROKT::ResponseObject contenant le nlohmann::json {"count": <nombre>}.
 */
class CountCommandHandler : public CommandHandler {
public:
    CountCommandHandler(RoktService *service) : CommandHandler(service) {}
    virtual std::unique_ptr<ROKT::ResponseObject> handle(const std::string &command) override {
        std::istringstream iss(command);
        std::string keyword, dataset, condition;
        
        // Lecture initiale de la commande : on attend "COUNT <dataset>"
        if (!(iss >> keyword >> dataset)) {
            return CommandHandler::handle(command);
        }
        keyword = trim(keyword);
        dataset = trim(dataset);
        if (keyword != "COUNT") {
            return CommandHandler::handle(command);
        }
        
        // Lire la condition optionnelle (format "key:value")
        if (iss >> condition) {
            condition = trim(condition);
        } else {
            condition = "";
        }
        
        // Charger le dataset via le service et lire toutes les données
        std::shared_ptr<RoktDataset> datasetObj;
        if(this->service->from(dataset, datasetObj)->hasError()) {
                return ROKT::ResponseService::response(1, "Can't get dataset");
        }
        nlohmann::json data = datasetObj->select({"*"}).raw();
        
        size_t count = 0;
        if (condition.empty()) {
            // Si aucune condition n'est donnée, on compte toutes les lignes
            count = data.size();
        } else {
            // Condition attendue : "key:value"
            size_t pos = condition.find(':');
            if (pos == std::string::npos) {
                return ROKT::ResponseService::response(423, "Condition COUNT invalide, format attendu 'key:value'");
            }
            std::string key = trim(condition.substr(0, pos));
            std::string value = trim(condition.substr(pos + 1));
            
            // Compter les lignes qui possèdent le champ et dont la valeur correspond
            for (const auto &row : data) {
                if (row.contains(key)) {
                    try {
                        // Essayer d'obtenir la valeur en tant que chaîne
                        std::string cell = row[key].get<std::string>();
                        if (cell == value) {
                            count++;
                        }
                    } catch (...) {
                        // Si ce n'est pas une chaîne, comparer via dump()
                        if (row[key].dump() == value) {
                            count++;
                        }
                    }
                }
            }
        }
        
        // Construire la réponse au format nlohmann::json {"count": <nombre>}
        nlohmann::json resp;
        resp["count"] = count;
        return ROKT::ResponseService::response(0, "OK", resp.dump());
    }
};

#endif // COUNT_COMMAND_HANDLER_H
