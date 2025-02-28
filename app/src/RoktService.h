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
    std::shared_ptr<EncryptService> encryptService;
    
    // Variables membres pour les chemins encryptés
    std::string encryptedDatabaseRoot;
    std::string encryptedDataConfigFile;
    
    // Méthodes privées pour lire/écrire la configuration chiffrée
    nlohmann::json loadConfig();
    void writeConfig(const nlohmann::json &configJson);
    
public:
    static const std::string DATABASE_ROOT;  // "shared/datas" n'est plus utilisé directement
    static const std::string DATA_CONFIG_FILENAME; // "datasets.config.json" en clair
    RoktService(const std::string& dir, std::shared_ptr<EncryptService> enc);
    
    // Méthodes publiques
    std::unique_ptr<ROKT::ResponseObject> create(const std::string& dataset, const std::string& type, const std::vector<std::string>& args = {});
    std::unique_ptr<ROKT::ResponseObject> drop(const std::string& dataset);
    std::unique_ptr<ROKT::ResponseObject> from(const std::string& dataset, std::shared_ptr<RoktDataset>& result);
};

#endif // ROKTSERVICE_H
