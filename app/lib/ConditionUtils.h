#ifndef CONDITION_UTILS_H
#define CONDITION_UTILS_H

#include <nlohmann/json.hpp>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <string>



struct Condition
{
    std::string field; // Par exemple, "name" ou "details.city"
    std::string op;    // Opérateur ("==", "!=", "HAS", "<", etc.)
    std::string value; // Valeur de comparaison (sous forme de chaîne)
    std::string logic; // "AND" ou "OR" (vide pour la première condition)
};

static nlohmann::json getNestedValue(const nlohmann::json &j, const std::string &compoundKey)
{
    std::istringstream iss(compoundKey);
    std::string token;
    nlohmann::json current = j;
    while (std::getline(iss, token, '.'))
    {
        if (current.contains(token))
            current = current[token];
        else
            return nullptr;
    }
    return current;
}

static bool evaluateCondition(const nlohmann::json &item, const Condition &cond, bool *res)
{
    nlohmann::json fieldValue;
    if (cond.field.find('.') != std::string::npos)
        fieldValue = getNestedValue(item, cond.field);
    else
    {
        if (item.contains(cond.field))
            fieldValue = item[cond.field];
        else
            return false;
    }
    if (fieldValue.is_null() || fieldValue == nullptr)
        return false;

    try
    {
        double cmpVal = std::stod(cond.value);
        if (fieldValue.is_number())
        {
            double val = fieldValue.get<double>();
            if (cond.op == "==")
            {
                *res = val == cmpVal;
                return true;
            }
            else if (cond.op == "!=")
            {
                *res = val != cmpVal;
                return true;
            }
            else if (cond.op == "<")
            {
                *res = val < cmpVal;
                return true;
            }
            else if (cond.op == "<=")
            {
                *res = val <= cmpVal;
                return true;
            }
            else if (cond.op == ">")
            {
                *res = val > cmpVal;
                return true;
            }
            else if (cond.op == ">=")
            {
                *res = val >= cmpVal;
                return true;
            }
            else
            {
                // "Opérateur non reconnu pour valeurs numériques"
                return false;
            }
        }
    }
    catch (...)
    {
        // Conversion échouée, on compare en chaîne.
    }
    if (cond.op == "HAS")
    {
        if (!fieldValue.is_array())
        {
            *res = false;
            return true;
        }
        for (auto &elem : fieldValue)
        {
            if (elem == cond.value)
            {
                *res = true;
                return true;
            }
        }
        *res = false;
        return true;
    }
    else
    {
        if (fieldValue.is_string())
        {
            std::string strVal = fieldValue.get<std::string>();
            if (cond.op == "==")
            {
                *res = strVal == cond.value;
                return true;
            }
            else if (cond.op == "!=")
            {
                *res = strVal != cond.value;
                return true;
            }
            else if (cond.op == "<")
            {
                *res = strVal < cond.value;
                return true;
            }
            else if (cond.op == "<=")
            {
                *res = strVal <= cond.value;
                return true;
            }
            else if (cond.op == ">")
            {
                *res = strVal > cond.value;
                return true;
            }
            else if (cond.op == ">=")
            {
                *res = strVal >= cond.value;
                return true;
            }
            else
            {
                // "Opérateur non reconnu pour chaînes";
                return false;
            }
        }
        else
        {
            std::string strVal = fieldValue.dump();
            if (cond.op == "==")
            {
                *res = strVal == cond.value;
                return true;
            }
            else if (cond.op == "!=")
            {
                *res = strVal != cond.value;
                return true;
            }
            else if (cond.op == "<")
            {
                *res = strVal < cond.value;
                return true;
            }
            else if (cond.op == "<=")
            {
                *res = strVal <= cond.value;
                return true;
            }
            else if (cond.op == ">")
            {
                *res = strVal > cond.value;
                return true;
            }
            else if (cond.op == ">=")
            {
                *res = strVal >= cond.value;
                return true;
            }
            else
            {
                // "Opérateur non reconnu");
                return false;
            }
        }
    }
}

static bool evaluateConditions(const nlohmann::json &item, const std::vector<Condition> &conds, bool* result)
{
    if (conds.empty())
        return true;
    *result = evaluateCondition(item, conds[0], result);
    for (size_t i = 1; i < conds.size(); i++)
    {
        const std::string &logic = conds[i].logic;
        bool current = evaluateCondition(item, conds[i], result);
        if (logic == "AND")
            *result = *result && current;
        else if (logic == "OR")
            *result = *result || current;
        else {
            // "Logique de condition non reconnue"
            return false;
        }
    }
    return true;
}

#endif // CONDITION_UTILS_H
