// RoktService.h
#ifndef ROKTSERVICE_H
#define ROKTSERVICE_H

#include "RoktDataset.h"
#include "EncryptService.h"
#include "RoktResponseService.h"
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

class RoktService {
private:
    std::string baseDir;
    EncryptService* encryptService;
    
    // Variables membres pour les chemins encryptés
    std::string encryptedDatabaseRoot;
    std::string encryptedDataConfigFile;
    
    // Méthodes privées pour lire/écrire la configuration chiffrée
    nlohmann::json loadConfig();
    void writeConfig(const nlohmann::json &configJson);
    
public:
    static const std::string DATABASE_ROOT;  // "shared/datas" n'est plus utilisé directement
    static const std::string DATA_CONFIG_FILENAME; // "datasets.config.json" en clair
    RoktService(const std::string& dir, EncryptService* enc);
    
    // Méthodes publiques
    RoktResponseObject *create(const std::string& dataset, const std::string& type, const std::vector<std::string>& args = {});
    RoktResponseObject *drop(const std::string& dataset);
    RoktDataset from(const std::string& dataset, bool auto_create = false);
};

#endif // ROKTSERVICE_H
