#ifndef ROKTDATA_H
#define ROKTDATA_H

#include <nlohmann/json.hpp>
#include <string>



class RoktData {
private:
    nlohmann::json data;
    std::string lastError;
public:
    RoktData(const nlohmann::json& d);
    size_t len() const;
    bool where(const std::string& key, const std::string& op, const nlohmann::json& compare, RoktData* result);
    bool at(size_t index, RoktData* result);
    bool head(size_t limit, RoktData* result);
    bool last(RoktData* result);
    bool get(const std::string& key, RoktData* result);
    nlohmann::json raw() const;
};

#endif // ROKTDATA_H
