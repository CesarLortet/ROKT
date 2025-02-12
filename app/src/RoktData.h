#ifndef ROKTDATA_H
#define ROKTDATA_H

#include <nlohmann/json.hpp>
#include <string>

using json = nlohmann::json;

class RoktData {
private:
    json data;
public:
    RoktData(const json& d);
    size_t len() const;
    RoktData where(const std::string& key, const std::string& op, const json& compare);
    RoktData at(size_t index);
    RoktData head(size_t limit);
    RoktData last();
    json raw() const;
    RoktData get(const std::string& key);
};

#endif // ROKTDATA_H
