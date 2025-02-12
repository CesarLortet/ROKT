#include "Config.h"
#include <fstream>
#include <stdexcept>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

Config::Config(const std::string& configFilePath) {
    std::ifstream file(configFilePath);
    if (!file)
        throw std::runtime_error("Impossible d'ouvrir le fichier de configuration: " + configFilePath);
    json j;
    file >> j;
    passphrase = j["encryption"]["passphrase"].get<std::string>();
    iv = j["encryption"]["iv"].get<std::string>();
    if (iv.size() != 16)
        throw std::runtime_error("Le IV doit être de 16 octets.");
    if (passphrase.size() < 16)
        passphrase.resize(16, '0');
    else
        passphrase = passphrase.substr(0, 16);
}
