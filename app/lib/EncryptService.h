// EncryptService.h
#ifndef ENCRYPTSERVICE_H
#define ENCRYPTSERVICE_H

#include <string>

class EncryptService {
private:
    std::string key; // clé de 16 octets
    std::string iv;  // vecteur d'initialisation de 16 octets
public:
    EncryptService(const std::string &key_, const std::string &iv_);
    std::string encrypt(const std::string &plaintext);
    std::string decrypt(const std::string &ciphertext);

    // Nouveaux utilitaires pour chiffrer/déchiffrer les noms de fichiers/dossiers
    std::string encryptFilename(const std::string &filename);
    std::string decryptFilename(const std::string &encryptedFilename);
};

#endif // ENCRYPTSERVICE_H
