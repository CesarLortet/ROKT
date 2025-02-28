#ifndef CHANGE_COMMAND_HANDLER_H
#define CHANGE_COMMAND_HANDLER_H

#include "CommandHandler.h"
#include <sstream>
#include <nlohmann/json.hpp>
#include "RoktResponseService.h"
#include "RoktDataset.h"
#include "RoktService.h"
#include "ConditionUtils.h"

class ChangeCommandHandler : public CommandHandler
{
private:
    std::string lastError;

    struct ChangeParams
    {
        std::string field;
        std::string newValue;
        std::vector<Condition> conditions;
        std::string dataset;
    };

    bool parseCommand(const std::string &command, ChangeParams *params)
    {
        std::istringstream iss(command);
        std::string token, keyword;
        iss >> keyword;
        if (keyword != "CHANGE")
        {

            this->lastError = "Commande CHANGE invalide";
            return false;
        }
        if (!(iss >> params->field))
        {
            this->lastError = "Champ manquant";
            return false;
        }
        params->field = trim(params->field);
        if (!(iss >> token) || trim(token) != "=")
        {
            this->lastError = "Symbole '=' manquant";
            return false;
        }
        if (!(iss >> params->newValue))
        {
            this->lastError = "Nouvelle valeur manquante";
            return false;
        }
        params->newValue = trim(params->newValue);
        if (!(iss >> token))
        {
            this->lastError = "Syntaxe invalide";
            return false;
        }
        token = trim(token);
        if (token == "WHERE")
        {
            Condition cond;
            if (!(iss >> cond.field >> cond.op >> cond.value))
            {
                this->lastError = "Clause WHERE incomplète";
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
                this->lastError = "Opérateur invalide";
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
                        this->lastError = "Opérateur invalide";
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
                this->lastError = "IN attendu après WHERE";
                return false;
            }
            if (!(iss >> params->dataset))
            {
                this->lastError = "Dataset manquant";
                return false;
            }
            params->dataset = trim(params->dataset);
        }
        else if (token == "IN")
        {
            if (!(iss >> params->dataset))
            {
                this->lastError = "Dataset manquant";
                return false;
            }
            params->dataset = trim(params->dataset);
        }
        else
        {
            {
                this->lastError = "Syntaxe CHANGE invalide";
                return false;
            }
        }
        return true;
    }

public:
    ChangeCommandHandler(RoktService *service) : CommandHandler(service) {}
    virtual std::unique_ptr<ROKT::ResponseObject>handle(const std::string &command) override
    {
        try
        {
            ChangeParams params;
            if(!parseCommand(command, &params))
            {
                return CommandHandler::handle(command);
            }
            std::shared_ptr<RoktDataset> datasetObj;
            if(this->service->from(params.dataset, datasetObj)->hasError()) {
                    return ROKT::ResponseService::response(1, "Can't get dataset");
            }
            nlohmann::json data = datasetObj->select({"*"}).raw();
            int changedCount = 0;
            for (auto &item : data)
            {
                
                bool cond;
                if(!evaluateConditions(item, params.conditions, &cond)) {
                    return ROKT::ResponseService::response(3, "Can't verify condition");
                }
                bool ok = params.conditions.empty() ? true : cond;
                if (ok)
                {
                    item[params.field] = params.newValue;
                    changedCount++;
                }
            }
            datasetObj->overwrite(data);
            return ROKT::ResponseService::response(0, std::string("OK, mis à jour ") + std::to_string(changedCount) + " ligne(s).");
        }
        catch (std::exception &e)
        {
            return ROKT::ResponseService::response(423, std::string("Erreur CHANGE: ") + e.what());
        }
    }
};

#endif
