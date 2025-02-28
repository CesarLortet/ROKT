// RoktService.cpp
#include "RoktService.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <filesystem>
#include <cstdlib>
#include <nlohmann/json.hpp>



RoktService::RoktService(const std::string& dir, std::shared_ptr<EncryptService> enc)
    : baseDir(dir), encryptService(enc)
{
    // On conserve le dossier "shared" en clair,
    // puis on crypte le nom du dossier "datas" pour obtenir le dossier contenant les datasets.
    // Par exemple, si "datas" encrypté donne "a3f4d2...", le dossier sera : <baseDir>/shared/a3f4d2...
    encryptedDatabaseRoot = baseDir;
    encryptedDatabaseRoot.std::string::append("/shared/");
    encryptedDatabaseRoot.std::string::append(encryptService->encryptFilename("datas"));
    // Le fichier de configuration est placé dans ce dossier et son nom est également encrypté.
    encryptedDataConfigFile = encryptedDatabaseRoot;
    encryptedDataConfigFile.std::string::append("/");
    encryptedDataConfigFile.std::string::append(encryptService->encryptFilename("datasets.config.json"));

    // Assurer que le dossier encrypté existe
    std::filesystem::create_directories(encryptedDatabaseRoot);
    // Si le fichier de configuration n'existe pas, on le crée avec une configuration par défaut.
    std::ifstream ifs(encryptedDataConfigFile, std::ios::binary);
    if (!ifs) {
        nlohmann::json defaultConfig;
        defaultConfig["datasets"] = nlohmann::json::object();
        writeConfig(defaultConfig);
    }
}

nlohmann::json RoktService::loadConfig() {
    std::ifstream configFile(encryptedDataConfigFile, std::ios::binary);
    if (!configFile) {
        nlohmann::json defaultConfig;
        defaultConfig["datasets"] = nlohmann::json::object();
        writeConfig(defaultConfig);
        return defaultConfig;
    }
    std::stringstream buffer;
    buffer << configFile.rdbuf();
    std::string encryptedData = buffer.str();
    std::string decryptedData;
    try {
        decryptedData = encryptService->decrypt(encryptedData);
    } catch (std::exception& e) {
        nlohmann::json defaultConfig;
        defaultConfig["datasets"] = nlohmann::json::object();
        writeConfig(defaultConfig);
        return defaultConfig;
    }
    try {
        return nlohmann::json::parse(decryptedData);
    } catch (nlohmann::json::parse_error& e) {
        nlohmann::json defaultConfig;
        defaultConfig["datasets"] = nlohmann::json::object();
        writeConfig(defaultConfig);
        return defaultConfig;
    }
}

void RoktService::writeConfig(const nlohmann::json &configJson) {
    std::string plaintext = configJson.dump(4);
    std::string encryptedData = encryptService->encrypt(plaintext);
    std::ofstream outConfig(encryptedDataConfigFile, std::ios::binary);
    if (!outConfig)
        throw std::runtime_error("Impossible d'écrire le fichier de configuration.");
    outConfig.write(encryptedData.data(), encryptedData.size());
}

std::unique_ptr<ROKT::ResponseObject>RoktService::create(const std::string& dataset, const std::string& type, const std::vector<std::string>& args) {
    // Charger la configuration chiffrée
    nlohmann::json configJson = loadConfig();

    // Vérifier si le dataset existe déjà
    if (configJson["datasets"].contains(dataset))
        return ROKT::ResponseService::response(10, "Already Exists");

    // Mettre à jour la configuration pour ce dataset
    configJson["datasets"][dataset]["type"] = type;
    
    if (type == "SIMPLE") {
        // Pour un dataset SIMPLE, on définit un nom de fichier par défaut
        std::string defaultFileName = encryptService->encryptFilename("dataset.rokt");
        configJson["datasets"][dataset]["file"] = defaultFileName;
    }
    if (type == "ROTATE") {
        if (!args.empty())
            try {
                configJson["datasets"][dataset]["size"] = std::stoi(args[0]);
            } catch (...) {
                return ROKT::ResponseService::response(12, "Bad file size format");
            }
        else
            configJson["datasets"][dataset]["size"] = "3Mo";
        if (args.size() > 1) {
            try {
                int nb = (std::stoi(args[1]) ? std::stoi(args[1]) : 3);
                configJson["datasets"][dataset]["nb_rotation"] = nb;
            } catch (...) {
                return ROKT::ResponseService::response(12, "Bad file number format");
            }
        } else {
            configJson["datasets"][dataset]["nb_rotation"] = 2;
        }
    }
    
    // Le nom du dataset est conservé en clair dans la configuration,
    // mais le dossier créé sera obfusqué.
    std::string encryptedDatasetName = encryptService->encryptFilename(dataset);
    std::string datasetDir = encryptedDatabaseRoot + "/" + encryptedDatasetName;
    std::filesystem::create_directories(datasetDir);
    
    // Pour les datasets SIMPLE, créer le fichier dataset s'il n'existe pas
    if (type == "SIMPLE") {
        // Récupérer le nom encrypté du fichier (défini dans la configuration)
        std::string fileName = configJson["datasets"][dataset]["file"].get<std::string>();
        std::string filePath = datasetDir + "/" + fileName;
        if (!std::filesystem::exists(filePath)) {
            std::ofstream outFile(filePath, std::ios::binary);
            if (!outFile)
                return ROKT::ResponseService::response(423, "Impossible de créer le fichier du dataset");
            // Initialiser le fichier avec un tableau nlohmann::json vide
            nlohmann::json emptyArray = nlohmann::json::array();
            std::string plaintext = emptyArray.dump();
            std::string encryptedContent = encryptService->encrypt(plaintext);
            outFile.write(encryptedContent.data(), encryptedContent.size());
            outFile.close();
        }
    }
    
    // Enregistrer la configuration mise à jour (le fichier de config est lui-même encrypté)
    writeConfig(configJson);
    return ROKT::ResponseService::response(0, "OK");
}


std::unique_ptr<ROKT::ResponseObject> RoktService::drop(const std::string& dataset) {
    nlohmann::json configJson = loadConfig();
    if (!configJson["datasets"].contains(dataset))
        return ROKT::ResponseService::response(567); // Dataset non existant

    std::string encryptedDatasetName = encryptService->encryptFilename(dataset);
    std::string datasetDir = encryptedDatabaseRoot;
    datasetDir.append("/"); 
    datasetDir.append(encryptedDatasetName);

    std::error_code ec;
    std::filesystem::remove_all(datasetDir, ec);
    
    if (ec) {
        return ROKT::ResponseService::response(457);
    }

    configJson["datasets"].erase(dataset);
    writeConfig(configJson);
    return ROKT::ResponseService::response(0);
}

std::unique_ptr<ROKT::ResponseObject> RoktService::from(const std::string& dataset, std::shared_ptr<RoktDataset>& result) {
    nlohmann::json configJson = loadConfig();
    if (!configJson["datasets"].contains(dataset)) {
        return ROKT::ResponseService::response(1, "Dataset does not exist");
    }

    std::string type = configJson["datasets"][dataset]["type"].get<std::string>();
    std::string encryptedDatasetName = encryptService->encryptFilename(dataset);
    std::string datasetDir = encryptedDatabaseRoot;
    datasetDir.std::string::append("/");
    datasetDir.std::string::append(encryptedDatasetName);
    
    if (type == "ROTATE") {
        std::vector<std::string> files = { encryptService->encryptFilename("1.rokt") };
        result = std::make_shared<RoktDataset>(DatasetConfigType::ROTATE, datasetDir, files, encryptService);
        return ROKT::ResponseService::response(0);
    } 

    // Tous les autres cas (SIMPLE et NON EXISTANTS)
    result = std::make_shared<RoktDataset>(DatasetConfigType::DATASET, datasetDir, encryptService->encryptFilename("dataset.rokt"), encryptService);
    return ROKT::ResponseService::response(0);
}
