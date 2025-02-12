#ifndef REMOVE_COMMAND_HANDLER_H
#define REMOVE_COMMAND_HANDLER_H

#include "CommandHandler.h"
#include <sstream>
#include <nlohmann/json.hpp>
#include "RoktResponseService.h"
#include "RoktDataset.h"
#include "RoktService.h"
#include "ConditionUtils.h"
#include "Utils.h" // pour trim()
#include <vector>
#include <stdexcept>

class RemoveCommandHandler : public CommandHandler {
private:
    struct RemoveParams {
        std::vector<Condition> conditions;
        std::string dataset;
    };

    RemoveParams parseCommand(const std::string &command) {
        RemoveParams params;
        std::istringstream iss(command);
        std::string keyword;
        if (!(iss >> keyword))
            throw std::runtime_error("Commande vide.");
        keyword = trim(keyword);
        if (keyword != "REMOVE")
            throw std::runtime_error("La commande doit commencer par REMOVE.");
        std::string token;
        if (!(iss >> token))
            throw std::runtime_error("Syntaxe invalide pour REMOVE.");
        token = trim(token);
        if (token == "WHERE") {
            Condition cond;
            if (!(iss >> cond.field >> cond.op >> cond.value))
                throw std::runtime_error("Clause WHERE incomplète.");
            cond.field = trim(cond.field);
            cond.op = trim(cond.op);
            cond.value = trim(cond.value);
            if (cond.op == "IS")
                cond.op = "==";
            else if (cond.op == "NOT")
                cond.op = "!=";
            else if (cond.op != "==" && cond.op != "!=" && cond.op != "HAS")
                throw std::runtime_error("Opérateur invalide dans WHERE.");
            cond.logic = "";
            params.conditions.push_back(cond);
            while (iss >> token) {
                token = trim(token);
                if (token == "AND" || token == "OR") {
                    Condition cond2;
                    cond2.logic = token;
                    if (!(iss >> cond2.field >> cond2.op >> cond2.value))
                        throw std::runtime_error("Clause WHERE incomplète après " + token);
                    cond2.field = trim(cond2.field);
                    cond2.op = trim(cond2.op);
                    cond2.value = trim(cond2.value);
                    if (cond2.op == "IS")
                        cond2.op = "==";
                    else if (cond2.op == "NOT")
                        cond2.op = "!=";
                    else if (cond2.op != "==" && cond2.op != "!=" && cond2.op != "HAS")
                        throw std::runtime_error("Opérateur invalide dans WHERE.");
                    params.conditions.push_back(cond2);
                } else if (trim(token) == "IN") {
                    break;
                } else {
                    break;
                }
            }
            if (trim(token) != "IN")
                throw std::runtime_error("Syntaxe invalide pour REMOVE, attendu IN après WHERE clause.");
            if (!(iss >> params.dataset))
                throw std::runtime_error("Nom du dataset manquant.");
            params.dataset = trim(params.dataset);
        } else {
            // Forme simple : token est la valeur à comparer pour le champ par défaut "name"
            Condition cond;
            cond.field = "name";
            cond.op = "==";
            cond.value = token;
            cond.logic = "";
            params.conditions.push_back(cond);
            if (!(iss >> token))
                throw std::runtime_error("Syntaxe invalide pour REMOVE, attendu IN.");
            token = trim(token);
            if (token != "IN")
                throw std::runtime_error("Syntaxe invalide pour REMOVE, attendu IN.");
            if (!(iss >> params.dataset))
                throw std::runtime_error("Nom du dataset manquant.");
            params.dataset = trim(params.dataset);
        }
        return params;
    }
public:
    RemoveCommandHandler(RoktService *service) : CommandHandler(service) {}
    virtual RoktResponseObject* handle(const std::string &command) override {
        try {
            RemoveParams params = parseCommand(command);
            RoktDataset datasetObj = this->service->from(params.dataset);
            nlohmann::json fullData = datasetObj.select({"*"}).raw();
            nlohmann::json newData = nlohmann::json::array();
            int removedCount = 0;
            for (auto &item : fullData) {
                if (evaluateConditions(item, params.conditions))
                    removedCount++;
                else
                    newData.push_back(item);
            }
            datasetObj.overwrite(newData);
            return RoktResponseService::response(0, "OK, supprimé " + std::to_string(removedCount) + " ligne(s).");
        } catch (std::exception &e) {
            return CommandHandler::handle(command);
        }
    }
};

#endif // REMOVE_COMMAND_HANDLER_H
