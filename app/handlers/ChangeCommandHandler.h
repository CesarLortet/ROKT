#ifndef CHANGE_COMMAND_HANDLER_H
#define CHANGE_COMMAND_HANDLER_H

#include "CommandHandler.h"
#include <sstream>
#include <nlohmann/json.hpp>
#include "RoktResponseService.h"
#include "RoktDataset.h"
#include "RoktService.h"
#include "ConditionUtils.h"

class ChangeCommandHandler : public CommandHandler {
private:
    struct ChangeParams {
        std::string field;
        std::string newValue;
        std::vector<Condition> conditions;
        std::string dataset;
    };

    ChangeParams parseCommand(const std::string &command) {
        ChangeParams params;
        std::istringstream iss(command);
        std::string token, keyword;
        iss >> keyword;
        if (keyword != "CHANGE")
            throw std::runtime_error("Commande CHANGE invalide");
        if (!(iss >> params.field))
            throw std::runtime_error("Champ manquant");
        params.field = trim(params.field);
        if (!(iss >> token) || trim(token) != "=")
            throw std::runtime_error("Symbole '=' manquant");
        if (!(iss >> params.newValue))
            throw std::runtime_error("Nouvelle valeur manquante");
        params.newValue = trim(params.newValue);
        if (!(iss >> token))
            throw std::runtime_error("Syntaxe invalide");
        token = trim(token);
        if (token == "WHERE") {
            Condition cond;
            if (!(iss >> cond.field >> cond.op >> cond.value))
                throw std::runtime_error("Clause WHERE incomplète");
            cond.field = trim(cond.field);
            cond.op = trim(cond.op);
            cond.value = trim(cond.value);
            if (cond.op == "IS")
                cond.op = "==";
            else if (cond.op == "NOT")
                cond.op = "!=";
            else if (cond.op != "==" && cond.op != "!=" && cond.op != "HAS")
                throw std::runtime_error("Opérateur invalide");
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
                        throw std::runtime_error("Opérateur invalide");
                    params.conditions.push_back(cond2);
                } else if (trim(token) == "IN") {
                    break;
                } else {
                    break;
                }
            }
            if (trim(token) != "IN")
                throw std::runtime_error("IN attendu après WHERE");
            if (!(iss >> params.dataset))
                throw std::runtime_error("Dataset manquant");
            params.dataset = trim(params.dataset);
        } else if (token == "IN") {
            if (!(iss >> params.dataset))
                throw std::runtime_error("Dataset manquant");
            params.dataset = trim(params.dataset);
        } else {
            throw std::runtime_error("Syntaxe CHANGE invalide");
        }
        return params;
    }
public:
    ChangeCommandHandler(RoktService *service) : CommandHandler(service) {}
    virtual RoktResponseObject* handle(const std::string &command) override {
        try {
            ChangeParams params = parseCommand(command);
            RoktDataset datasetObj = this->service->from(params.dataset);
            nlohmann::json data = datasetObj.select({"*"}).raw();
            int changedCount = 0;
            for (auto &item : data) {
                bool ok = params.conditions.empty() ? true : evaluateConditions(item, params.conditions);
                if (ok) {
                    item[params.field] = params.newValue;
                    changedCount++;
                }
            }
            datasetObj.overwrite(data);
            return RoktResponseService::response(0, std::string("OK, mis à jour ") + std::to_string(changedCount) + " ligne(s).");
        } catch (std::exception &e) {
            return RoktResponseService::response(423, std::string("Erreur CHANGE: ") + e.what());
        }
    }
};

#endif
