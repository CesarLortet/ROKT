#ifndef CONDITION_UTILS_H
#define CONDITION_UTILS_H

#include <nlohmann/json.hpp>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <string>

using json = nlohmann::json;

struct Condition {
    std::string field;   // Par exemple, "name" ou "details.city"
    std::string op;      // Opérateur ("==", "!=", "HAS", "<", etc.)
    std::string value;   // Valeur de comparaison (sous forme de chaîne)
    std::string logic;   // "AND" ou "OR" (vide pour la première condition)
};

static json getNestedValue(const json &j, const std::string &compoundKey) {
    std::istringstream iss(compoundKey);
    std::string token;
    json current = j;
    while (std::getline(iss, token, '.')) {
        if (current.contains(token))
            current = current[token];
        else
            return nullptr;
    }
    return current;
}

static bool evaluateCondition(const json &item, const Condition &cond) {
    json fieldValue;
    if (cond.field.find('.') != std::string::npos)
        fieldValue = getNestedValue(item, cond.field);
    else {
        if (item.contains(cond.field))
            fieldValue = item[cond.field];
        else
            return false;
    }
    if (fieldValue.is_null() || fieldValue == nullptr)
        return false;
    
    try {
        double cmpVal = std::stod(cond.value);
        if (fieldValue.is_number()) {
            double val = fieldValue.get<double>();
            if (cond.op == "==") return val == cmpVal;
            else if (cond.op == "!=") return val != cmpVal;
            else if (cond.op == "<") return val < cmpVal;
            else if (cond.op == "<=") return val <= cmpVal;
            else if (cond.op == ">") return val > cmpVal;
            else if (cond.op == ">=") return val >= cmpVal;
            else throw std::runtime_error("Opérateur non reconnu pour valeurs numériques");
        }
    } catch (...) {
        // Conversion échouée, on compare en chaîne.
    }
    if (cond.op == "HAS") {
        if (!fieldValue.is_array())
            return false;
        for (auto &elem : fieldValue) {
            if (elem == cond.value)
                return true;
        }
        return false;
    } else {
        if (fieldValue.is_string()) {
            std::string strVal = fieldValue.get<std::string>();
            if (cond.op == "==") return strVal == cond.value;
            else if (cond.op == "!=") return strVal != cond.value;
            else if (cond.op == "<") return strVal < cond.value;
            else if (cond.op == "<=") return strVal <= cond.value;
            else if (cond.op == ">") return strVal > cond.value;
            else if (cond.op == ">=") return strVal >= cond.value;
            else throw std::runtime_error("Opérateur non reconnu pour chaînes");
        } else {
            std::string strVal = fieldValue.dump();
            if (cond.op == "==") return strVal == cond.value;
            else if (cond.op == "!=") return strVal != cond.value;
            else if (cond.op == "<") return strVal < cond.value;
            else if (cond.op == "<=") return strVal <= cond.value;
            else if (cond.op == ">") return strVal > cond.value;
            else if (cond.op == ">=") return strVal >= cond.value;
            else throw std::runtime_error("Opérateur non reconnu");
        }
    }
}

static bool evaluateConditions(const json &item, const std::vector<Condition> &conds) {
    if (conds.empty()) return true;
    bool result = evaluateCondition(item, conds[0]);
    for (size_t i = 1; i < conds.size(); i++) {
        const std::string &logic = conds[i].logic;
        bool current = evaluateCondition(item, conds[i]);
        if (logic == "AND")
            result = result && current;
        else if (logic == "OR")
            result = result || current;
        else
            throw std::runtime_error("Logique de condition non reconnue");
    }
    return result;
}

#endif // CONDITION_UTILS_H
