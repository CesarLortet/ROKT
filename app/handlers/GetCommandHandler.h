#ifndef GET_COMMAND_HANDLER_H
#define GET_COMMAND_HANDLER_H

#include "CommandHandler.h"
#include "RoktService.h"
#include "RoktDataset.h"
#include "Utils.h"  // contient trim(), etc.
#include "ConditionUtils.h"  // pour Condition, getNestedValue, evaluateConditions
#include <sstream>
#include <string>
#include <nlohmann/json.hpp>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <cstdlib>



class GetCommandHandler : public CommandHandler {
private:
    // Structure pour stocker les paramètres extraits de la commande GET.
    struct GetParams {
        std::string fields;
        std::string dataset;
        std::string alias;
        std::vector<Condition> conditions;  // Conditions de WHERE
        std::string groupByKey;
        std::string orderByKey;
        bool orderDesc;
        int limit;
    };

    std::string lastError;

    // Parse la commande GET en gérant plusieurs conditions (AND/OR)
    bool parseCommand(const std::string &command, GetParams* params) {
        params->orderDesc = false;
        params->limit = -1;
        std::istringstream iss(command);
        std::string token;
        
        // Lecture initiale : on attend "GET <fields>"
        std::string getKeyword;
        if (!(iss >> getKeyword)) {
            this->lastError = "Commande vide.";
            return false;
        }
        getKeyword = trim(getKeyword);
        if (getKeyword != "GET") {
            this->lastError = "La commande doit commencer par GET.";
            return false;
        }
        if (!(iss >> params->fields)) {
            this->lastError = "Champ manquant après GET.";
            return false;
        }
        params->fields = trim(params->fields);
        
        // Optionnel : alias avec AS
        if (iss >> token) {
            token = trim(token);
            if (token == "AS") {
                if (!(iss >> params->alias)) {
                    this->lastError = "Alias manquant après AS.";
                    return false;
                }
                params->alias = trim(params->alias);
                if (!(iss >> token)) {
                    this->lastError = "Mot-clé IN manquant après alias.";
                    return false;
                }
                token = trim(token);
                if (token != "IN") {
                    this->lastError = "IN manquant après alias.";
                    return false;
                }
            } else if (token != "IN") {
                this->lastError = "Syntaxe invalide : attendu AS ou IN après le champ.";
                return false;
            }
        } else {
            this->lastError = "Syntaxe invalide : IN manquant.";
            return false;
        }
        
        // Lecture du dataset
        if (!(iss >> params->dataset)) {
            this->lastError = "Nom du dataset manquant.";
            return false;
        }
        params->dataset = trim(params->dataset);
        
        // Analyse des clauses optionnelles
        // On supporte WHERE avec conditions multiples (séparées par AND ou OR),
        // GROUP BY, ORDER BY et LIMIT.
        while (iss >> token) {
            token = trim(token);
            if (token == "WHERE") {
                // Lecture de la première condition
                Condition cond;
                if (!(iss >> cond.field >> cond.op >> cond.value)) {
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
                else if (cond.op != "==" && cond.op != "!=" && cond.op != "HAS") {
                    this->lastError = "Opérateur invalide dans WHERE.";
                    return false;
                }
                cond.logic = ""; // première condition, pas de logique
                params->conditions.push_back(cond);
                // Lecture de conditions supplémentaires, le cas échéant
                while (iss >> token) {
                    token = trim(token);
                    if (token == "AND" || token == "OR") {
                        Condition cond2;
                        cond2.logic = token; // logique reliant à la condition précédente
                        if (!(iss >> cond2.field >> cond2.op >> cond2.value)){
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
                        else if (cond2.op != "==" && cond2.op != "!=" && cond2.op != "HAS") {
                            this->lastError = "Opérateur invalide dans WHERE.";
                            return false;
                        }
                        params->conditions.push_back(cond2);
                    } else {
                        // Si le token n'est pas AND/OR, on le traite dans les clauses suivantes
                        // On sort de la boucle de conditions.
                        break;
                    }
                }
                // Si le token lu n'était pas AND/OR, il sera traité par la suite
                // (On peut utiliser un putback si nécessaire, mais ici nous continuons)
            } else if (token == "GROUP") {
                std::string by;
                if (!(iss >> by)) {
                    this->lastError = "Clé manquante après GROUP.";
                    return false;
                }
                by = trim(by);
                if (by != "BY") {
                    this->lastError = "Syntaxe invalide pour GROUP BY.";
                    return false;
                }
                if (!(iss >> params->groupByKey)) {
                    this->lastError = "Clé manquante après GROUP BY.";
                    return false;
                }
                params->groupByKey = trim(params->groupByKey);
            } else if (token == "ORDER") {
                std::string by;
                if (!(iss >> by)) {
                    this->lastError = "Clé manquante après ORDER.";
                    return false;
                }   
                by = trim(by);
                if (by != "BY") {
                    this->lastError = "Syntaxe invalide pour ORDER BY.";
                    return false;
                }
                if (!(iss >> params->orderByKey)) {
                    this->lastError = "Clé manquante après ORDER BY.";
                    return false;
                }
                params->orderByKey = trim(params->orderByKey);
                std::string orderOpt;
                if (iss >> orderOpt) {
                    orderOpt = trim(orderOpt);
                    if (orderOpt == "DESC")
                        params->orderDesc = true;
                    else if (orderOpt == "ASC")
                        params->orderDesc = false;
                }
            } else if (token == "LIMIT") {
                std::string limitStr;
                if (!(iss >> limitStr)) {
                    this->lastError = "Valeur manquante pour LIMIT.";
                    return false;
                }
                limitStr = trim(limitStr);
                try {
                    params->limit = std::stoi(limitStr);
                } catch(...) {
                    this->lastError = "Valeur invalide pour LIMIT.";
                    return false;
                }
            } else {
                // Ignorer les tokens non reconnus
            }
        }
        return true;
    }
    
    // Filtre le tableau selon les conditions multiples.
    nlohmann::json applyConditions(const nlohmann::json &data, const std::vector<Condition> &conds) {
        nlohmann::json filtered = nlohmann::json::array();
        for (auto &item : data) {
            bool result = false;
            evaluateConditions(item, conds, &result);
            if (result)
                filtered.push_back(item);
        }
        return filtered;
    }
    
    // Fonction groupBy : regroupe les objets par la clé donnée.
    nlohmann::json groupBy(const nlohmann::json &data, const std::string &groupKey) {
        nlohmann::json groups = nlohmann::json::object();
        for (auto &item : data) {
            nlohmann::json groupValue;
            if (groupKey.find('.') != std::string::npos) {
                groupValue = getNestedValue(item, groupKey);
            } else {
                if (item.contains(groupKey))
                    groupValue = item[groupKey];
                else
                    groupValue = "undefined";
            }
            std::string groupStr = groupValue.dump();
            if (groups.find(groupStr) == groups.end()) {
                groups[groupStr] = nlohmann::json::array();
            }
            groups[groupStr].push_back(item);
        }
        return groups;
    }
    
    // Fonction orderBy : trie le tableau selon orderKey en ignorant les éléments sans cette clé.
    // Renvoie le tableau trié et met à jour ignoredCount.
    nlohmann::json orderBy(const nlohmann::json &data, const std::string &orderKey, bool desc, int &ignoredCount) {
        std::vector<nlohmann::json> vec;
        ignoredCount = 0;
        for (auto &item : data) {
            nlohmann::json val;
            if (orderKey.find('.') != std::string::npos) {
                val = getNestedValue(item, orderKey);
            } else {
                if (item.contains(orderKey))
                    val = item[orderKey];
                else
                    val = nullptr;
            }
            if (val.is_null() || val == nullptr) {
                ignoredCount++;
            } else {
                vec.push_back(item);
            }
        }
        std::sort(vec.begin(), vec.end(), [&](const nlohmann::json &a, const nlohmann::json &b) {
            nlohmann::json valA, valB;
            if (orderKey.find('.') != std::string::npos) {
                valA = getNestedValue(a, orderKey);
                valB = getNestedValue(b, orderKey);
            } else {
                if (a.contains(orderKey))
                    valA = a[orderKey];
                if (b.contains(orderKey))
                    valB = b[orderKey];
            }
            if(valA.is_number() && valB.is_number()){
                return desc ? (valA.get<double>() > valB.get<double>()) : (valA.get<double>() < valB.get<double>());
            }
            return desc ? (valA.dump() > valB.dump()) : (valA.dump() < valB.dump());
        });
        return vec;
    }
    
    // Fonction applyLimit : limite le nombre d'éléments d'un tableau nlohmann::json.
    nlohmann::json applyLimit(const nlohmann::json &data, int limit) {
        if (!data.is_array() || limit < 0 || static_cast<int>(data.size()) <= limit)
            return data;
        nlohmann::json limited = nlohmann::json::array();
        for (int i = 0; i < limit; i++) {
            limited.push_back(data[i]);
        }
        return limited;
    }
    
    // Fonction applyProjection : extrait le champ spécifié de chaque objet.
    nlohmann::json applyProjection(const nlohmann::json &data, const std::string &fields) {
        if (!data.is_array() || fields == "*")
            return data;
        nlohmann::json projected = nlohmann::json::array();
        for (auto &item : data) {
            if (item.contains(fields))
                projected.push_back(item[fields]);
        }
        return projected;
    }
    
    // Fonction applyAlias : enveloppe chaque élément dans un objet avec la clé alias.
    nlohmann::json applyAlias(const nlohmann::json &data, const std::string &alias) {
        if (!data.is_array())
            return data;
        nlohmann::json aliased = nlohmann::json::array();
        for (auto &item : data) {
            nlohmann::json obj;
            obj[alias] = item;
            aliased.push_back(obj);
        }
        return aliased;
    }
    
public:
    GetCommandHandler(RoktService *service) : CommandHandler(service) {}
    std::unique_ptr<ROKT::ResponseObject>handle(const std::string &command) override {
        if (command.find("GET") != 0)
            return CommandHandler::handle(command);
        try {
            GetParams params;
            if(!parseCommand(command, &params)) {
                return CommandHandler::handle(command);
            }
            int ignoredCount = 0;
            // Récupérer l'objet complet du dataset
            std::shared_ptr<RoktDataset> datasetObj;
            if(this->service->from(params.dataset, datasetObj)->hasError()) {
                    return ROKT::ResponseService::response(1, "Can't get dataset");
            }
            auto fullData = datasetObj->select({"*"});
            auto data = fullData;
            // Appliquer les conditions si présentes
            if (!params.conditions.empty()) {

                if(!data.where("", "==", "dummy", &data))  // On ignore le where() existant
                {
                    // Ca n'a pas marche
                    return ROKT::ResponseService::response(3, "Where failed");
                }
                // Nous filtrons manuellement avec evaluateConditions
                nlohmann::json filtered = nlohmann::json::array();
                for (auto &item : fullData.raw()) {
                    bool cond;
                    if(!evaluateConditions(item, params.conditions, &cond)) {
                        return ROKT::ResponseService::response(3, "Can't verify condition");
                    }
                    if (cond)
                        filtered.push_back(item);
                }
                data = RoktData(filtered);
            }
            nlohmann::json result = data.raw();
            // Appliquer GROUP BY si présent
            if (!params.groupByKey.empty()) {
                result = groupBy(result, params.groupByKey);
            }
            // Appliquer ORDER BY si présent (uniquement sur tableaux)
            if (result.is_array() && !params.orderByKey.empty()) {
                result = orderBy(result, params.orderByKey, params.orderDesc, ignoredCount);
            }
            // Appliquer LIMIT si présent
            if (result.is_array() && params.limit > 0)
                result = applyLimit(result, params.limit);
            // Appliquer la projection si nécessaire (si fields n'est pas "*" et sans GROUP BY)
            if (params.fields != "*" && params.groupByKey.empty() && result.is_array())
                result = applyProjection(result, params.fields);
            // Appliquer l'alias si spécifié
            if (!params.alias.empty() && result.is_array())
                result = applyAlias(result, params.alias);
            
            // Construction finale de la réponse en fonction de la logique "never nester"
            bool extraClauseUsed = (!params.conditions.empty() || !params.groupByKey.empty() ||
                                      !params.orderByKey.empty() || params.limit != -1 || !params.alias.empty());
            nlohmann::json responseObj;
            if (extraClauseUsed) {
                responseObj["result"] = result;
                if (ignoredCount != 0)
                    responseObj["ignored"] = ignoredCount;
            } else {
                responseObj = result;
            }
            return ROKT::ResponseService::response(0, "OK", responseObj.dump(4));
        } catch (std::exception &e) {
            return ROKT::ResponseService::response(1, std::string("Erreur de traitement de la commande GET: ") + e.what());
        }
    }
};

#endif // GET_COMMAND_HANDLER_H
