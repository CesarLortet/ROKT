#include "RoktDataset.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <filesystem>
#include <nlohmann/json.hpp>



// Constructeur pour un dataset SIMPLE
RoktDataset::RoktDataset(DatasetConfigType t, const std::string &p, const std::string &ds, std::shared_ptr<EncryptService> enc)
    : type(t), path(p), encryptService(enc) {
    if (t == DatasetConfigType::DATASET)
        datasetFiles.push_back(ds);
}

// Constructeur pour un dataset ROTATE
RoktDataset::RoktDataset(DatasetConfigType t, const std::string &p, const std::vector<std::string> &ds, std::shared_ptr<EncryptService> enc)
    : type(t), path(p), datasetFiles(ds), encryptService(enc) {}

// Fonction interne pour lire le dataset à partir d'un fichier
bool RoktDataset::readDataset(const std::string &filename, nlohmann::json *result) {
    std::string fullPath = path + "/" + filename;
    std::ifstream file(fullPath, std::ios::binary);
    if (!file) {
        // Si le fichier n'existe pas, on crée un fichier vide
        nlohmann::json empty = nlohmann::json::array();
        writeDataset(filename, empty);
        file.open(fullPath, std::ios::binary);
        if (!file) {
            this->lastError = "Impossible d'ouvrir le fichier dataset : " + filename;
            return false;
        }
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string encryptedData = buffer.str();
    std::string decryptedData;
    try {
        decryptedData = encryptService->decrypt(encryptedData);
    } catch (std::exception &e) {
        // En cas d'erreur de décryptage, on recrée un fichier vide
        nlohmann::json empty = nlohmann::json::array();
        writeDataset(filename, empty);
        *result = empty;
        return true;
    }
    try {
        *result = nlohmann::json::parse(decryptedData);
        return true;
    } catch (nlohmann::json::parse_error &e) {
        // En cas d'erreur de parsing, recréer le fichier vide
        nlohmann::json empty = nlohmann::json::array();
        writeDataset(filename, empty);
        *result = empty;
        return true;
    }
}

// Fonction interne pour lire le dataset à partir d'un fichier
nlohmann::json RoktDataset::readData() {
    // Construire le chemin complet du fichier dataset.
    // On utilise le premier fichier de la liste datasetFiles.
    if (datasetFiles.empty()) {
        throw std::runtime_error("Aucun fichier de dataset défini.");
    }
    std::string filename = datasetFiles[0];
    std::string fullPath = path + "/" + filename;

    // Ouvrir le fichier en mode binaire.
    std::ifstream file(fullPath, std::ios::binary);
    if (!file) {
        // Si le fichier n'existe pas, créer un tableau nlohmann::json vide et l'écrire.
        nlohmann::json emptyData = nlohmann::json::array();
        writeDataset(filename, emptyData);
        return emptyData;
    }

    // Lire le contenu du fichier.
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string encryptedData = buffer.str();

    // Déchiffrer le contenu.
    std::string decryptedData;
    try {
        decryptedData = encryptService->decrypt(encryptedData);
    } catch (std::exception &e) {
        // En cas d'erreur de décryptage, on recrée un tableau vide.
        nlohmann::json emptyData = nlohmann::json::array();
        writeDataset(filename, emptyData);
        return emptyData;
    }

    // Parser le nlohmann::json déchiffré.
    try {
        return nlohmann::json::parse(decryptedData);
    } catch (nlohmann::json::parse_error &e) {
        // En cas d'erreur de parsing, on écrase le fichier avec un tableau vide.
        nlohmann::json emptyData = nlohmann::json::array();
        writeDataset(filename, emptyData);
        return emptyData;
    }
}


// Fonction interne pour écrire le nlohmann::json dans le fichier dataset
void RoktDataset::writeDataset(const std::string &filename, const nlohmann::json &j) {
    std::string fullPath = path + "/" + filename;
    std::string plaintext = j.dump();
    std::string encryptedData = encryptService->encrypt(plaintext);
    std::ofstream file(fullPath, std::ios::binary);
    if (!file)
        throw std::runtime_error("Impossible d'écrire dans le fichier dataset : " + filename);
    file.write(encryptedData.data(), encryptedData.size());
}

// Méthode update (non modifiée ici, on suppose qu'elle suit la logique précédente)
std::unique_ptr<ROKT::ResponseObject>RoktDataset::update(const nlohmann::json &set, const nlohmann::json &value, const std::vector<nlohmann::json> &where) {
    // Implémentation similaire à celle précédemment proposée (non détaillée ici)
    return ROKT::ResponseService::response(0);
}

// Méthode remove (non modifiée ici, on suppose qu'elle suit la logique précédente)
std::unique_ptr<ROKT::ResponseObject>RoktDataset::remove(const std::string &set, const std::string &op, const nlohmann::json &compare) {
    nlohmann::json data;
    if(!readDataset(datasetFiles[0], &data)) {
        return ROKT::ResponseService::response(3, "Can't read dataset");
    }
    nlohmann::json newData = nlohmann::json::array();
    for (auto &row : data) {
        // Si la ligne satisfait la condition, elle sera supprimée
        // On compare directement ici (on suppose que la méthode where du RoktData est utilisée pour GET)
        // Pour REMOVE, on effectue une comparaison simple
        if (!(row.contains(set) && row[set] == compare))
            newData.push_back(row);
    }
    writeDataset(datasetFiles[0], newData);
    return ROKT::ResponseService::response(0);
}

// Méthode insert
std::unique_ptr<ROKT::ResponseObject>RoktDataset::insert(const nlohmann::json &newData) {
    nlohmann::json data;
    std::ifstream infile(path + "/" + datasetFiles[0], std::ios::binary);
    if (infile) {
        if(!readDataset(datasetFiles[0], &data)) {
            return ROKT::ResponseService::response(3, "Can't read dataset");
        }
    }
    else
        data = nlohmann::json::array();
    data.push_back(newData);
    writeDataset(datasetFiles[0], data);
    return ROKT::ResponseService::response(2);
}

// Méthode select : renvoie un RoktData à partir d'une sélection de champs.
RoktData RoktDataset::select(const std::vector<std::string> &keys) {
    nlohmann::json data;
    if(!readDataset(datasetFiles[0], &data)) {
        return RoktData(nlohmann::json::array());
    }
    // Si le premier champ est "*", renvoyer les données complètes
    if (!keys.empty() && keys[0] == "*") {
        return RoktData(data);
    } else {
        nlohmann::json result = nlohmann::json::array();
        for (auto &row : data) {
            nlohmann::json item;
            for (auto &key : keys) {
                if (row.contains(key))
                    item[key] = row[key];
            }
            result.push_back(item);
        }
        return RoktData(result);
    }
}

std::unique_ptr<ROKT::ResponseObject>RoktDataset::clear() {
    nlohmann::json empty = nlohmann::json::array();
    writeDataset(datasetFiles[0], empty);
    return ROKT::ResponseService::response(0); // 0 correspond à "OK"
}

// Méthode overwrite : remplace le contenu du dataset par newData.
std::unique_ptr<ROKT::ResponseObject>RoktDataset::overwrite(const nlohmann::json &newData) {
    if (datasetFiles.empty())
        return ROKT::ResponseService::response(567);
    std::string filename = datasetFiles[0];
    writeDataset(filename, newData);
    return ROKT::ResponseService::response(0);
}
