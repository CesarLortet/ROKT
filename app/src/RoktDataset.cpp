#include "RoktDataset.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <filesystem>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Constructeur pour un dataset SIMPLE
RoktDataset::RoktDataset(DatasetConfigType t, const std::string &p, const std::string &ds, EncryptService* enc)
    : type(t), path(p), encryptService(enc) {
    if (t == DatasetConfigType::DATASET)
        datasetFiles.push_back(ds);
}

// Constructeur pour un dataset ROTATE
RoktDataset::RoktDataset(DatasetConfigType t, const std::string &p, const std::vector<std::string> &ds, EncryptService* enc)
    : type(t), path(p), datasetFiles(ds), encryptService(enc) {}

// Fonction interne pour lire le dataset à partir d'un fichier
json RoktDataset::readDataset(const std::string &filename) {
    std::string fullPath = path + "/" + filename;
    std::ifstream file(fullPath, std::ios::binary);
    if (!file) {
        // Si le fichier n'existe pas, on crée un fichier vide
        json empty = json::array();
        writeDataset(filename, empty);
        file.open(fullPath, std::ios::binary);
        if (!file)
            throw std::runtime_error("Impossible d'ouvrir le fichier dataset : " + filename);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string encryptedData = buffer.str();
    std::string decryptedData;
    try {
        decryptedData = encryptService->decrypt(encryptedData);
    } catch (std::exception &e) {
        // En cas d'erreur de décryptage, on recrée un fichier vide
        json empty = json::array();
        writeDataset(filename, empty);
        return empty;
    }
    try {
        return json::parse(decryptedData);
    } catch (json::parse_error &e) {
        // En cas d'erreur de parsing, recréer le fichier vide
        json empty = json::array();
        writeDataset(filename, empty);
        return empty;
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
        // Si le fichier n'existe pas, créer un tableau JSON vide et l'écrire.
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

    // Parser le JSON déchiffré.
    try {
        return nlohmann::json::parse(decryptedData);
    } catch (nlohmann::json::parse_error &e) {
        // En cas d'erreur de parsing, on écrase le fichier avec un tableau vide.
        nlohmann::json emptyData = nlohmann::json::array();
        writeDataset(filename, emptyData);
        return emptyData;
    }
}


// Fonction interne pour écrire le JSON dans le fichier dataset
void RoktDataset::writeDataset(const std::string &filename, const json &j) {
    std::string fullPath = path + "/" + filename;
    std::string plaintext = j.dump();
    std::string encryptedData = encryptService->encrypt(plaintext);
    std::ofstream file(fullPath, std::ios::binary);
    if (!file)
        throw std::runtime_error("Impossible d'écrire dans le fichier dataset : " + filename);
    file.write(encryptedData.data(), encryptedData.size());
}

// Méthode update (non modifiée ici, on suppose qu'elle suit la logique précédente)
RoktResponseObject *RoktDataset::update(const json &set, const json &value, const std::vector<json> &where) {
    // Implémentation similaire à celle précédemment proposée (non détaillée ici)
    return RoktResponseService::response(0);
}

// Méthode remove (non modifiée ici, on suppose qu'elle suit la logique précédente)
RoktResponseObject *RoktDataset::remove(const std::string &set, const std::string &op, const json &compare) {
    json data = readDataset(datasetFiles[0]);
    json newData = json::array();
    for (auto &row : data) {
        // Si la ligne satisfait la condition, elle sera supprimée
        // On compare directement ici (on suppose que la méthode where du RoktData est utilisée pour GET)
        // Pour REMOVE, on effectue une comparaison simple
        if (!(row.contains(set) && row[set] == compare))
            newData.push_back(row);
    }
    writeDataset(datasetFiles[0], newData);
    return RoktResponseService::response(0);
}

// Méthode insert
RoktResponseObject *RoktDataset::insert(const json &newData) {
    json data;
    std::ifstream infile(path + "/" + datasetFiles[0], std::ios::binary);
    if (infile)
        data = readDataset(datasetFiles[0]);
    else
        data = json::array();
    data.push_back(newData);
    writeDataset(datasetFiles[0], data);
    return RoktResponseService::response(2);
}

// Méthode select : renvoie un RoktData à partir d'une sélection de champs.
RoktData RoktDataset::select(const std::vector<std::string> &keys) {
    json data = readDataset(datasetFiles[0]);
    // Si le premier champ est "*", renvoyer les données complètes
    if (!keys.empty() && keys[0] == "*") {
        return RoktData(data);
    } else {
        json result = json::array();
        for (auto &row : data) {
            json item;
            for (auto &key : keys) {
                if (row.contains(key))
                    item[key] = row[key];
            }
            result.push_back(item);
        }
        return RoktData(result);
    }
}

RoktResponseObject *RoktDataset::clear() {
    json empty = json::array();
    writeDataset(datasetFiles[0], empty);
    return RoktResponseService::response(0); // 0 correspond à "OK"
}

// Méthode overwrite : remplace le contenu du dataset par newData.
RoktResponseObject *RoktDataset::overwrite(const json &newData) {
    if (datasetFiles.empty())
        return RoktResponseService::response(567);
    std::string filename = datasetFiles[0];
    writeDataset(filename, newData);
    return RoktResponseService::response(0);
}
