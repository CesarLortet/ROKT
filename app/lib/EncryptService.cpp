#include "EncryptService.h"
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <cstdlib>

static std::string toHex(const std::string &input) {
    std::ostringstream oss;
    for (unsigned char c : input) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);
    }
    return oss.str();
}

static std::string fromHex(const std::string &hexStr) {
    std::string output;
    if (hexStr.length() % 2 != 0) return "";
    for (size_t i = 0; i < hexStr.length(); i += 2) {
        std::string byteStr = hexStr.substr(i, 2);
        char byte = static_cast<char>(std::stoi(byteStr, nullptr, 16));
        output.push_back(byte);
    }
    return output;
}

EncryptService::EncryptService(const std::string &key_, const std::string &iv_)
    : key(key_), iv(iv_) {}

std::string EncryptService::encrypt(const std::string &plaintext) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx)
        throw std::runtime_error("Échec de création du contexte OpenSSL.");
    if (EVP_EncryptInit_ex(ctx, EVP_aes_128_ctr(), NULL,
                           reinterpret_cast<const unsigned char*>(key.data()),
                           reinterpret_cast<const unsigned char*>(iv.data())) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_EncryptInit_ex a échoué.");
    }
    std::string ciphertext;
    ciphertext.resize(plaintext.size() + AES_BLOCK_SIZE);
    int out_len1 = 0;
    if (EVP_EncryptUpdate(ctx,
                          reinterpret_cast<unsigned char*>(&ciphertext[0]),
                          &out_len1,
                          reinterpret_cast<const unsigned char*>(plaintext.data()),
                          plaintext.size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_EncryptUpdate a échoué.");
    }
    int out_len2 = 0;
    if (EVP_EncryptFinal_ex(ctx,
                            reinterpret_cast<unsigned char*>(&ciphertext[0]) + out_len1,
                            &out_len2) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_EncryptFinal_ex a échoué.");
    }
    EVP_CIPHER_CTX_free(ctx);
    ciphertext.resize(out_len1 + out_len2);
    return ciphertext;
}

std::string EncryptService::decrypt(const std::string &ciphertext) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx)
        throw std::runtime_error("Échec de création du contexte OpenSSL.");
    if (EVP_DecryptInit_ex(ctx, EVP_aes_128_ctr(), NULL,
                           reinterpret_cast<const unsigned char*>(key.data()),
                           reinterpret_cast<const unsigned char*>(iv.data())) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_DecryptInit_ex a échoué.");
    }
    std::string plaintext;
    plaintext.resize(ciphertext.size());
    int out_len1 = 0;
    if (EVP_DecryptUpdate(ctx,
                          reinterpret_cast<unsigned char*>(&plaintext[0]),
                          &out_len1,
                          reinterpret_cast<const unsigned char*>(ciphertext.data()),
                          ciphertext.size()) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_DecryptUpdate a échoué.");
    }
    int out_len2 = 0;
    if (EVP_DecryptFinal_ex(ctx,
                            reinterpret_cast<unsigned char*>(&plaintext[0]) + out_len1,
                            &out_len2) != 1) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_DecryptFinal_ex a échoué.");
    }
    EVP_CIPHER_CTX_free(ctx);
    plaintext.resize(out_len1 + out_len2);
    return plaintext;
}

std::string EncryptService::encryptFilename(const std::string &filename) {
    // On chiffre le nom à l'aide de encrypt(), puis on encode en hexadécimal
    std::string encrypted = encrypt(filename);
    return toHex(encrypted);
}

std::string EncryptService::decryptFilename(const std::string &encryptedFilename) {
    std::string binary = fromHex(encryptedFilename);
    return decrypt(binary);
}
