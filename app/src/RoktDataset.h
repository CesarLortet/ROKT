#ifndef ROKTDATASET_H
#define ROKTDATASET_H

#include "RoktData.h"
#include "EncryptService.h"
#include "RoktResponseService.h"
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

enum class DatasetConfigType {
    ROTATE,
    DATASET
};

class RoktDataset {
private:
    DatasetConfigType type;
    std::string path; // chemin vers le dossier du dataset
    std::vector<std::string> datasetFiles; // pour DATASET, un seul fichier ; pour ROTATE, plusieurs fichiers
    std::shared_ptr<EncryptService> encryptService;
    std::string lastError;

    // Fonction interne pour lire et déchiffrer le fichier dataset
    bool readDataset(const std::string &filename, nlohmann::json *json);
    // Fonction interne pour chiffrer et écrire le nlohmann::json dans le fichier dataset
    void writeDataset(const std::string &filename, const nlohmann::json &j);
    
public:
    RoktDataset(DatasetConfigType t, const std::string &p, const std::string &ds, std::shared_ptr<EncryptService> enc);
    RoktDataset(DatasetConfigType t, const std::string &p, const std::vector<std::string> &ds, std::shared_ptr<EncryptService> enc);
    
    nlohmann::json readData();
    
    // Méthodes de mise à jour, suppression, insertion et sélection
    std::unique_ptr<ROKT::ResponseObject>update(const nlohmann::json &set, const nlohmann::json &value, const std::vector<nlohmann::json> &where = {});
    std::unique_ptr<ROKT::ResponseObject>remove(const std::string &set, const std::string &op, const nlohmann::json &compare);
    std::unique_ptr<ROKT::ResponseObject>insert(const nlohmann::json &newData);
    RoktData select(const std::vector<std::string> &keys);
    
    std::unique_ptr<ROKT::ResponseObject>clear();
    
    // Nouvelle méthode pour écraser (overwrite) le fichier dataset avec de nouvelles données.
    std::unique_ptr<ROKT::ResponseObject>overwrite(const nlohmann::json &newData);
};

#endif // ROKTDATASET_H
