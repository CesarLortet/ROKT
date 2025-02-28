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

RoktData::RoktData(const nlohmann::json& d) : data(d) {

}

size_t RoktData::len() const {
    return data.size();
}

bool RoktData::where(const std::string& key, const std::string& op, const nlohmann::json& compare, RoktData* result) {
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
                this->lastError = "L'opérateur HAS ne s'applique qu'aux tableaux.";
                return false;
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
            else {
                this->lastError = "Opérateur non reconnu";
                return false;
            }
        } else {
            if (op == "<") condition = fieldValue < compare;
            else if (op == "<=") condition = fieldValue <= compare;
            else if (op == ">") condition = fieldValue > compare;
            else if (op == ">=") condition = fieldValue >= compare;
            else if (op == "==") condition = fieldValue == compare;
            else if (op == "!=") condition = fieldValue != compare;
            else {
                this->lastError = "Opérateur non reconnu";
                return false;
            }
        }
        if (condition)
            ret.push_back(item);
    }
    result = new RoktData(ret);
    return true; 
}

bool RoktData::at(size_t index, RoktData* result) {
    if (index > data.size()) {
        this->lastError = "Index out of range";
        return false;
    }
    result = new RoktData(data[index]);
    return true;
}

bool RoktData::head(size_t limit, RoktData* result) {
    nlohmann::json ret = nlohmann::json::array();
    for (size_t i = 0; i < std::min(limit, data.size()); i++)
        ret.push_back(data[i]);
    result = new RoktData(ret);
    return true;
}

bool RoktData::last(RoktData* result) {
    if (data.empty()) {
        this->lastError = "Data is empty";
        return false;
    }
    result = new RoktData(data.back());
    return true;
}

nlohmann::json RoktData::raw() const {
    return data;
}

bool RoktData::get(const std::string& key, RoktData* result) {
    nlohmann::json ret = nlohmann::json::array();
    for (auto& item : data)
        if (item.contains(key))
            ret.push_back(item[key]);
    result = new RoktData(ret);
    return true;
}