#ifndef COUNT_COMMAND_HANDLER_H
#define COUNT_COMMAND_HANDLER_H

#include "CommandHandler.h"
#include "RoktResponseService.h"
#include "RoktService.h"
#include "Utils.h"           // pour trim()
#include <sstream>
#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

/**
 * @brief CountCommandHandler traite la commande COUNT en effectuant le comptage directement.
 *
 * Syntaxe attendue :
 *   COUNT <dataset> [<key>:<value>];
 *
 * Si aucune condition n'est donnée, il compte toutes les lignes du dataset.
 * Sinon, il ne compte que les lignes où la valeur du champ spécifié est égale à la valeur donnée.
 * La réponse est encapsulée dans un RoktResponseObject contenant le JSON {"count": <nombre>}.
 */
class CountCommandHandler : public CommandHandler {
public:
    CountCommandHandler(RoktService *service) : CommandHandler(service) {}
    virtual RoktResponseObject* handle(const std::string &command) override {
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
        RoktDataset datasetObj = this->service->from(dataset);
        json data = datasetObj.select({"*"}).raw();
        
        size_t count = 0;
        if (condition.empty()) {
            // Si aucune condition n'est donnée, on compte toutes les lignes
            count = data.size();
        } else {
            // Condition attendue : "key:value"
            size_t pos = condition.find(':');
            if (pos == std::string::npos) {
                return RoktResponseService::response(423, "Condition COUNT invalide, format attendu 'key:value'");
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
        
        // Construire la réponse au format JSON {"count": <nombre>}
        json resp;
        resp["count"] = count;
        return RoktResponseService::response(0, "OK", resp.dump());
    }
};

#endif // COUNT_COMMAND_HANDLER_H
