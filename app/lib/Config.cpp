#include "Config.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <cstdlib>

Config::Config(const std::string& filename) {
    // Valeurs par défaut
    encryption.passphrase = "default_passphrase";
    encryption.iv = "default_iv";
    network.port = 8080;
    thread.maxWorkers = 8;
    thread.maxTaskQueueSize = 100; // Valeur par défaut

    // Charger depuis le fichier JSON
    std::ifstream file(filename);
    if (file) {
        nlohmann::json json;
        file >> json;
        if (json.contains("encryption")) {
            auto& enc = json["encryption"];
            if (enc.contains("passphrase")) encryption.passphrase = enc["passphrase"];
            if (enc.contains("iv")) encryption.iv = enc["iv"];
        }
        if (json.contains("network")) {
            auto& net = json["network"];
            if (net.contains("port")) {
                network.port = net["port"].get<int>(); // Accès à "port" dans "network" + conversion en int
            }
        }
        if (json.contains("thread")) {
            auto& thr = json["thread"];
            if (thr.contains("maxWorkers")) {
                thread.maxWorkers = thr["maxWorkers"].get<int>(); // Accès à "maxWorkers" dans "thread" + conversion
            }
            if (thr.contains("maxTaskQueueSize")) {
                thread.maxTaskQueueSize = thr["maxTaskQueueSize"].get<int>(); // Idem pour "maxTaskQueueSize"
            }
        }
    }

    // Surcharge par les variables d'environnement
    loadFromEnv();
}

void Config::loadFromEnv() {
    const char* portEnv = std::getenv("ROKT_PORT");
    if (portEnv != nullptr) {
        int envPort = std::atoi(portEnv);
        if (envPort > 0 && envPort <= 65535) {
            network.port = envPort;
        } else {
            LogService::log("Valeur de ROKT_PORT invalide. Conservation de la valeur actuelle.");
        }
    }

    const char* workersEnv = std::getenv("ROKT_MAX_WORKERS");
    if (workersEnv != nullptr) {
        int envWorkers = std::atoi(workersEnv);
        if (envWorkers > 0) {
            thread.maxWorkers = envWorkers;
        } else {
            LogService::log("Valeur de ROKT_MAX_WORKERS invalide. Conservation de la valeur actuelle.");
        }
    }

    const char* queueSizeEnv = std::getenv("ROKT_MAX_TASK_QUEUE_SIZE");
    if (queueSizeEnv != nullptr) {
        int envQueueSize = std::atoi(queueSizeEnv);
        if (envQueueSize > 0) {
            thread.maxTaskQueueSize = envQueueSize;
        } else {
            LogService::log("Valeur de ROKT_MAX_TASK_QUEUE_SIZE invalide. Conservation de la valeur actuelle.");
        }
    }
}

bool Config::isValid() const {
    // Vérification des champs de chiffrement
    if (encryption.passphrase.empty() || encryption.iv.empty()) {
        return false;
    }
    
    // Vérification du port réseau
    if (network.port < 1 || network.port > 65535) {
        return false;
    }
    
    // Vérification des paramètres de threads
    if (thread.maxWorkers <= 0 || thread.maxTaskQueueSize <= 0) {
        return false;
    }
    
    return true;
}