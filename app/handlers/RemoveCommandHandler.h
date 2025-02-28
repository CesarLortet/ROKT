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

class RemoveCommandHandler : public CommandHandler
{
private:

    std::string lastError;

    struct RemoveParams
    {
        std::vector<Condition> conditions;
        std::string dataset;
    };

    bool parseCommand(const std::string &command, RemoveParams* params)
    {
        std::istringstream iss(command);
        std::string keyword;
        if (!(iss >> keyword))
        {
            this->lastError = "Commande vide.";
            return false;
        }
        keyword = trim(keyword);
        if (keyword != "REMOVE")
        {
            this->lastError = "La commande doit commencer par REMOVE.";
            return false;
        }
        std::string token;
        if (!(iss >> token))
        {
            this->lastError = "Syntaxe invalide pour REMOVE.";
            return false;
        }
        token = trim(token);
        if (token == "WHERE")
        {
            Condition cond;
            if (!(iss >> cond.field >> cond.op >> cond.value))
            {
                this->lastError = "Clause WHERE incomplète.";
                return false;
            }
            cond.field = trim(cond.field);
            cond.op = trim(cond.op);
            cond.value = trim(cond.value);
            if (cond.op == "IS")
                cond.op = "==";
            else if (cond.op == "NOT")
                cond.op = "!=";
            else if (cond.op != "==" && cond.op != "!=" && cond.op != "HAS")
            {
                this->lastError = "Opérateur invalide dans WHERE.";
                return false;
            }
            cond.logic = "";
            params->conditions.push_back(cond);
            while (iss >> token)
            {
                token = trim(token);
                if (token == "AND" || token == "OR")
                {
                    Condition cond2;
                    cond2.logic = token;
                    if (!(iss >> cond2.field >> cond2.op >> cond2.value))
                    {
                        this->lastError = "Clause WHERE incomplète après " + token;
                        return false;
                    }
                    cond2.field = trim(cond2.field);
                    cond2.op = trim(cond2.op);
                    cond2.value = trim(cond2.value);
                    if (cond2.op == "IS")
                        cond2.op = "==";
                    else if (cond2.op == "NOT")
                        cond2.op = "!=";
                    else if (cond2.op != "==" && cond2.op != "!=" && cond2.op != "HAS")
                    {
                        this->lastError = "Opérateur invalide dans WHERE.";
                        return false;
                    }
                    params->conditions.push_back(cond2);
                }
                else if (trim(token) == "IN")
                {
                    break;
                }
                else
                {
                    break;
                }
            }
            if (trim(token) != "IN")
            {
                this->lastError = "Syntaxe invalide pour REMOVE, attendu IN après WHERE clause.";
                return false;
            }
            if (!(iss >> params->dataset))
            {
                this->lastError = "Nom du dataset manquant.";
                return false;
            }
            params->dataset = trim(params->dataset);
        }
        else
        {
            // Forme simple : token est la valeur à comparer pour le champ par défaut "name"
            Condition cond;
            cond.field = "name";
            cond.op = "==";
            cond.value = token;
            cond.logic = "";
            params->conditions.push_back(cond);
            if (!(iss >> token))
            {
                this->lastError = "Syntaxe invalide pour REMOVE, attendu IN.";
                return false;
            }
            token = trim(token);
            if (token != "IN")
            {
                this->lastError = "Syntaxe invalide pour REMOVE, attendu IN.";
                return false;
            }
            if (!(iss >> params->dataset))
            {
                this->lastError = "Nom du dataset manquant.";
                return false;
            }
            params->dataset = trim(params->dataset);
        }
        return params;
    }

public:
    RemoveCommandHandler(RoktService *service) : CommandHandler(service) {}
    virtual std::unique_ptr<ROKT::ResponseObject>handle(const std::string &command) override
    {
        try
        {
            RemoveParams params;
            if(!parseCommand(command, &params)) {
                return CommandHandler::handle(command);
            }

            std::shared_ptr<RoktDataset> datasetObj;
            if(this->service->from(params.dataset, datasetObj)->hasError()) {
                    return ROKT::ResponseService::response(1, "Can't get dataset");
            }
            nlohmann::json fullData = datasetObj->select({"*"}).raw();
            nlohmann::json newData = nlohmann::json::array();
            int removedCount = 0;
            for (auto &item : fullData)
            {
                bool cond;
                if(!evaluateConditions(item, params.conditions, &cond)) {
                    return ROKT::ResponseService::response(3, "Can't verify condition");
                }
                if (cond)
                    removedCount++;
                else
                    newData.push_back(item);
            }
            datasetObj->overwrite(newData);
            return ROKT::ResponseService::response(0, "OK, supprimé " + std::to_string(removedCount) + " ligne(s).");
        }
        catch (std::exception &e)
        {
            return CommandHandler::handle(command);
        }
    }
};

#endif // REMOVE_COMMAND_HANDLER_H
