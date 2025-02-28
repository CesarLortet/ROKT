#ifndef ROKT_RESPONSE_OBJECT_H
#define ROKT_RESPONSE_OBJECT_H

#include <string>
#include <sstream>
#include "LogService.h"

namespace ROKT {
    /**
     * @brief ResponseObject encapsule le résultat d'une commande.
     *
     * Ce type d'objet contient un code de statut (0 pour OK, autre valeur pour erreur),
     * un message (reason) indiquant le résultat, et des données optionnelles (datas).
     */
    class ResponseObject {
    private:
        int code;              ///< Code de statut (0 pour OK, autre valeur pour erreur)
        std::string reason;    ///< Message ou raison associée à la réponse
        std::string datas;     ///< Données associées à la réponse

        /**
         * @brief Assigne un message par défaut si aucun message n'est spécifié.
         * Gère les codes invalides (négatifs) en assignant un état d'erreur par défaut.
         */
        void setDefaultReason() {
            if (reason.empty()) {
                if (code < 0) {
                    code = 1; // Code générique pour erreur
                    reason = "Invalid negative code corrected to ERROR";
                    return;
                }
                switch (code) {
                    // ---- GENERIC ----
                    case 0: reason = "OK"; break;
                    case 1: reason = "ERROR"; break;
                    case 2: reason = "Inserted"; break;
                    case 3: reason = "Can't get params"; break;

                    // ---- FILES ----
                    case 10: reason = "Already Exists"; break;
                    case 11: reason = "Bad file size format"; break;
                    case 12: reason = "Bad file number format"; break;

                    // ---- CONFIG ----
                    case 168: reason = "Configfile not found"; break;
                    case 244: reason = "Operator not found"; break;

                    // ---- MISC ----
                    case 423: reason = "Impossible de créer le fichier"; break;
                    case 457: reason = "Impossible de supprimer les fichiers"; break;
                    case 567: reason = "NULL"; break;

                    default: reason = "Unknown Error"; break;
                }
            }
        }

    public:
        /**
         * @brief Constructeur avec un code de statut uniquement.
         * @param code Le code de statut. Un message par défaut est assigné via setDefaultReason().
         */
        explicit ResponseObject(int code)
            : code(code), reason(""), datas("") {
            setDefaultReason();
        }

        /**
         * @brief Constructeur avec code et message.
         * @param code Le code de statut.
         * @param reason Le message associé (non modifié par setDefaultReason() si non vide).
         */
        ResponseObject(int code, const std::string& reason)
            : code(code), reason(reason), datas("") {
            setDefaultReason();
        }

        /**
         * @brief Constructeur avec code, message et données.
         * @param code Le code de statut.
         * @param reason Le message associé (non modifié par setDefaultReason() si non vide).
         * @param datas Les données associées.
         */
        ResponseObject(int code, const std::string& reason, const std::string& datas)
            : code(code), reason(reason), datas(datas) {
            setDefaultReason();
        }

        /**
         * @brief Retourne le code de statut.
         * @return Le code de statut.
         */
        int getStatusCode() const noexcept { return code; }

        /**
         * @brief Retourne le message associé à la réponse.
         * @return Une référence constante vers le message.
         */
        const std::string& getReasonPhrase() const noexcept { return reason; }

        /**
         * @brief Retourne les données associées à la réponse.
         * @return Une référence constante vers les données.
         */
        const std::string& getDatas() const noexcept { return datas; }

        /**
         * @brief Retourne soit le message, soit les données si présentes.
         * @return Une référence constante vers la chaîne appropriée.
         */
        std::string getResponse() const noexcept {
            std::ostringstream response;
            response << "{\"status\": " << code << ", \"reason\": \"" << reason << "\"";
            bool is_success = (code == 0);
            if (is_success && !datas.empty()) {
                response << ", \"datas\": " << datas;
            }
            response << "}";
            return response.str();
        }

        /**
         * @brief Retourne false si il n'y a pas d'erreur
         * @return un booleen constant du resultat
         */
        const bool hasError() {
            return code > 0;
        }
    };
} // namespace ROKT

#endif // ROKT_RESPONSE_OBJECT_H