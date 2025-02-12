#ifndef ROKT_RESPONSE_SERVICE_H
#define ROKT_RESPONSE_SERVICE_H

#include "RoktResponseObject.h"
#include <string>

/**
 * @brief RoktResponseService fournit des méthodes statiques pour générer
 * des objets RoktResponseObject standardisés.
 */
class RoktResponseService {
public:
    /**
     * @brief Crée un objet de réponse.
     * @param code Le code de statut.
     * @param message Le message associé.
     * @return RoktResponseObject* pointeur vers l'objet réponse.
     */
    static RoktResponseObject *response(int code) {
        return new RoktResponseObject(code);
    }

    static RoktResponseObject *response(int code, std::string message) {
        return new RoktResponseObject(code, message);
    }

    static RoktResponseObject *response(int code, std::string message, std::string datas) {
        return new RoktResponseObject(code, message, datas);
    }
};

#endif // ROKT_RESPONSE_SERVICE_H
