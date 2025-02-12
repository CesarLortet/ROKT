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

using json = nlohmann::json;

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
    
    // Parse la commande GET en gérant plusieurs conditions (AND/OR)
    GetParams parseCommand(const std::string &command) {
        GetParams params;
        params.orderDesc = false;
        params.limit = -1;
        std::istringstream iss(command);
        std::string token;
        
        // Lecture initiale : on attend "GET <fields>"
        std::string getKeyword;
        if (!(iss >> getKeyword))
            throw std::runtime_error("Commande vide.");
        getKeyword = trim(getKeyword);
        if (getKeyword != "GET")
            throw std::runtime_error("La commande doit commencer par GET.");
        if (!(iss >> params.fields))
            throw std::runtime_error("Champ manquant après GET.");
        params.fields = trim(params.fields);
        
        // Optionnel : alias avec AS
        if (iss >> token) {
            token = trim(token);
            if (token == "AS") {
                if (!(iss >> params.alias))
                    throw std::runtime_error("Alias manquant après AS.");
                params.alias = trim(params.alias);
                if (!(iss >> token))
                    throw std::runtime_error("Mot-clé IN manquant après alias.");
                token = trim(token);
                if (token != "IN")
                    throw std::runtime_error("IN manquant après alias.");
            } else if (token != "IN") {
                throw std::runtime_error("Syntaxe invalide : attendu AS ou IN après le champ.");
            }
        } else {
            throw std::runtime_error("Syntaxe invalide : IN manquant.");
        }
        
        // Lecture du dataset
        if (!(iss >> params.dataset))
            throw std::runtime_error("Nom du dataset manquant.");
        params.dataset = trim(params.dataset);
        
        // Analyse des clauses optionnelles
        // On supporte WHERE avec conditions multiples (séparées par AND ou OR),
        // GROUP BY, ORDER BY et LIMIT.
        while (iss >> token) {
            token = trim(token);
            if (token == "WHERE") {
                // Lecture de la première condition
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
                cond.logic = ""; // première condition, pas de logique
                params.conditions.push_back(cond);
                // Lecture de conditions supplémentaires, le cas échéant
                while (iss >> token) {
                    token = trim(token);
                    if (token == "AND" || token == "OR") {
                        Condition cond2;
                        cond2.logic = token; // logique reliant à la condition précédente
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
                if (!(iss >> by))
                    throw std::runtime_error("Clé manquante après GROUP.");
                by = trim(by);
                if (by != "BY")
                    throw std::runtime_error("Syntaxe invalide pour GROUP BY.");
                if (!(iss >> params.groupByKey))
                    throw std::runtime_error("Clé manquante après GROUP BY.");
                params.groupByKey = trim(params.groupByKey);
            } else if (token == "ORDER") {
                std::string by;
                if (!(iss >> by))
                    throw std::runtime_error("Clé manquante après ORDER.");
                by = trim(by);
                if (by != "BY")
                    throw std::runtime_error("Syntaxe invalide pour ORDER BY.");
                if (!(iss >> params.orderByKey))
                    throw std::runtime_error("Clé manquante après ORDER BY.");
                params.orderByKey = trim(params.orderByKey);
                std::string orderOpt;
                if (iss >> orderOpt) {
                    orderOpt = trim(orderOpt);
                    if (orderOpt == "DESC")
                        params.orderDesc = true;
                    else if (orderOpt == "ASC")
                        params.orderDesc = false;
                }
            } else if (token == "LIMIT") {
                std::string limitStr;
                if (!(iss >> limitStr))
                    throw std::runtime_error("Valeur manquante pour LIMIT.");
                limitStr = trim(limitStr);
                try {
                    params.limit = std::stoi(limitStr);
                } catch(...) {
                    throw std::runtime_error("Valeur invalide pour LIMIT.");
                }
            } else {
                // Ignorer les tokens non reconnus
            }
        }
        return params;
    }
    
    // Filtre le tableau selon les conditions multiples.
    json applyConditions(const json &data, const std::vector<Condition> &conds) {
        json filtered = json::array();
        for (auto &item : data) {
            if (evaluateConditions(item, conds))
                filtered.push_back(item);
        }
        return filtered;
    }
    
    // Fonction groupBy : regroupe les objets par la clé donnée.
    json groupBy(const json &data, const std::string &groupKey) {
        json groups = json::object();
        for (auto &item : data) {
            json groupValue;
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
                groups[groupStr] = json::array();
            }
            groups[groupStr].push_back(item);
        }
        return groups;
    }
    
    // Fonction orderBy : trie le tableau selon orderKey en ignorant les éléments sans cette clé.
    // Renvoie le tableau trié et met à jour ignoredCount.
    json orderBy(const json &data, const std::string &orderKey, bool desc, int &ignoredCount) {
        std::vector<json> vec;
        ignoredCount = 0;
        for (auto &item : data) {
            json val;
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
        std::sort(vec.begin(), vec.end(), [&](const json &a, const json &b) {
            json valA, valB;
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
    
    // Fonction applyLimit : limite le nombre d'éléments d'un tableau JSON.
    json applyLimit(const json &data, int limit) {
        if (!data.is_array() || limit < 0 || static_cast<int>(data.size()) <= limit)
            return data;
        json limited = json::array();
        for (int i = 0; i < limit; i++) {
            limited.push_back(data[i]);
        }
        return limited;
    }
    
    // Fonction applyProjection : extrait le champ spécifié de chaque objet.
    json applyProjection(const json &data, const std::string &fields) {
        if (!data.is_array() || fields == "*")
            return data;
        json projected = json::array();
        for (auto &item : data) {
            if (item.contains(fields))
                projected.push_back(item[fields]);
        }
        return projected;
    }
    
    // Fonction applyAlias : enveloppe chaque élément dans un objet avec la clé alias.
    json applyAlias(const json &data, const std::string &alias) {
        if (!data.is_array())
            return data;
        json aliased = json::array();
        for (auto &item : data) {
            json obj;
            obj[alias] = item;
            aliased.push_back(obj);
        }
        return aliased;
    }
    
public:
    GetCommandHandler(RoktService *service) : CommandHandler(service) {}
    RoktResponseObject *handle(const std::string &command) override {
        if (command.find("GET") != 0)
            return CommandHandler::handle(command);
        try {
            GetParams params = parseCommand(command);
            int ignoredCount = 0;
            // Récupérer l'objet complet du dataset
            auto fullData = this->service->from(params.dataset).select({"*"});
            auto data = fullData;
            // Appliquer les conditions si présentes
            if (!params.conditions.empty()) {
                data = data.where("", "==", "dummy"); // On ignore le where() existant
                // Nous filtrons manuellement avec evaluateConditions
                json filtered = json::array();
                for (auto &item : fullData.raw()) {
                    if (evaluateConditions(item, params.conditions))
                        filtered.push_back(item);
                }
                data = RoktData(filtered);
            }
            json result = data.raw();
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
            json responseObj;
            if (extraClauseUsed) {
                responseObj["data"] = result;
                if (ignoredCount != 0)
                    responseObj["ignored"] = ignoredCount;
            } else {
                responseObj = result;
            }
            return RoktResponseService::response(0, "OK", responseObj.dump(4));
        } catch (std::exception &e) {
            return RoktResponseService::response(1, std::string("Erreur de traitement de la commande GET: ") + e.what());
        }
    }
};

#endif // GET_COMMAND_HANDLER_H
