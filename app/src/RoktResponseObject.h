#ifndef ROKT_RESPONSE_OBJECT_H
#define ROKT_RESPONSE_OBJECT_H

#include <string>

/**
 * @brief RoktResponseObject encapsule le résultat d'une commande.
 *
 * Ce type d'objet contient un code de statut (0 pour OK, autre valeur pour erreur)
 * et un message (reason) permettant d'indiquer le résultat d'une commande (par exemple, "OK", "Already Exists", etc.).
 */
class RoktResponseObject {
private:
    int code;              ///< Code de statut (0 pour OK, autre valeur pour erreur)
    std::string reason;    ///< Message ou raison associée à la réponse
    std::string datas;

public:
    /**
     * @brief Constructeur de RoktResponseObject.
     * @param code Le code de statut.
     * @param reason Le message associé. Si la chaîne est vide, la méthode match() pourra assigner un message par défaut.
     */
    RoktResponseObject(int code)
        : code(code) {
            this->match();
        }
    RoktResponseObject(int code, std::string reason)
        : code(code), reason(reason) {}

    RoktResponseObject(int code, std::string reason, std::string datas)
        : code(code), reason(reason), datas(datas) {}
    /**
     * @brief Retourne le code de statut.
     * @return Le code de statut.
     */
    int getStatusCode() const { return code; }

    /**
     * @brief Retourne le message associé à la réponse.
     * @return Une référence constante vers le message.
     */
    const std::string& getReasonPhrase() const { return reason; }

    /**
     * @brief Retourne le message associé à la réponse.
     * @return Une référence constante vers le message.
     */
    const std::string& getDatas() const { return datas; }

    const std::string& getResponse() const {
        if(datas.empty())
            return reason;
        return datas;
    }

    /**
     * @brief Assigne un message par défaut si aucun message n'a été spécifié.
     *
     * Si le message est vide, cette méthode définit un message par défaut en fonction du code de statut.
     */
    void match() {
        if (reason.empty()) {
            switch (code) {
                case 0:
                    reason = "OK";
                    break;
                case 1:
                    reason = "ERROR";
                    break;
                case 2:
                    reason = "Inserted";
                    break;
                case 10:
                    reason = "Already Exists";
                    break;
                case 11:
                    reason = "Bad file size format";
                    break;
                case 12:
                    reason = "Bad file number format";
                    break;
                case 168:
                    reason = "Configfile not found";
                    break;
                case 244:
                    reason = "Operator not found";
                    break;
                case 567:
                    reason = "NULL";
                    break;
                default:
                    reason = "Unknown Error";
                    break;
            }
        }
    }
};

#endif // ROKT_RESPONSE_OBJECT_H
