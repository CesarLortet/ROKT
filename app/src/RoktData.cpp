// RoktData.cpp
#include "RoktData.h"
#include <sstream>
#include <stdexcept>
#include <algorithm>

// Fonction utilitaire pour obtenir une valeur imbriquée à partir d'une clé composée (ex: "details.age")
static nlohmann::json getNestedValue(const nlohmann::json &j, const std::string &compoundKey) {
    std::istringstream iss(compoundKey);
    std::string token;
    nlohmann::json current = j;
    while (std::getline(iss, token, '.')) {
        if (current.contains(token))
            current = current[token];
        else
            return nullptr; // La clé n'existe pas
    }
    return current;
}

RoktData::RoktData(const nlohmann::json& d) : data(d) {}

size_t RoktData::len() const {
    return data.size();
}

RoktData RoktData::where(const std::string& key, const std::string& op, const nlohmann::json& compare) {
    nlohmann::json ret = nlohmann::json::array();
    for (auto& item : data) {
        nlohmann::json fieldValue;
        if (key.find('.') != std::string::npos) {
            fieldValue = getNestedValue(item, key);
        } else {
            if (item.contains(key))
                fieldValue = item[key];
            else
                continue;
        }
        // Si la valeur n'est pas présente, on ignore cet item.
        if (fieldValue.is_null())
            continue;
        bool condition = false;
        if (op == "HAS") {
            if (!fieldValue.is_array()) {
                throw std::runtime_error("L'opérateur HAS ne s'applique qu'aux tableaux.");
            }
            for (auto& element : fieldValue) {
                if (element == compare) {
                    condition = true;
                    break;
                }
            }
        } else if (fieldValue.is_number() && compare.is_number()) {
            double a = fieldValue.get<double>();
            double b = compare.get<double>();
            if (op == "<") condition = a < b;
            else if (op == "<=") condition = a <= b;
            else if (op == ">") condition = a > b;
            else if (op == ">=") condition = a >= b;
            else if (op == "==") condition = a == b;
            else if (op == "!=") condition = a != b;
            else throw std::runtime_error("Opérateur non reconnu");
        } else {
            if (op == "<") condition = fieldValue < compare;
            else if (op == "<=") condition = fieldValue <= compare;
            else if (op == ">") condition = fieldValue > compare;
            else if (op == ">=") condition = fieldValue >= compare;
            else if (op == "==") condition = fieldValue == compare;
            else if (op == "!=") condition = fieldValue != compare;
            else throw std::runtime_error("Opérateur non reconnu");
        }
        if (condition)
            ret.push_back(item);
    }
    return RoktData(ret);
}

RoktData RoktData::at(size_t index) {
    if (index < data.size())
        return RoktData(data[index]);
    throw std::out_of_range("Index out of range");
}

RoktData RoktData::head(size_t limit) {
    nlohmann::json ret = nlohmann::json::array();
    for (size_t i = 0; i < std::min(limit, data.size()); i++)
        ret.push_back(data[i]);
    return RoktData(ret);
}

RoktData RoktData::last() {
    if (!data.empty())
        return RoktData(data.back());
    throw std::runtime_error("Data is empty");
}

nlohmann::json RoktData::raw() const {
    return data;
}

RoktData RoktData::get(const std::string& key) {
    nlohmann::json ret = nlohmann::json::array();
    for (auto& item : data)
        if (item.contains(key))
            ret.push_back(item[key]);
    return RoktData(ret);
}
