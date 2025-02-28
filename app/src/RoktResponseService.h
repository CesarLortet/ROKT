#ifndef ROKT_RESPONSE_SERVICE_H
#define ROKT_RESPONSE_SERVICE_H

#include <memory>
#include "RoktResponseObject.h"

namespace ROKT {
    /**
     * @brief Namespace utilitaire pour créer des objets RoktResponseObject de manière standardisée.
     */
    namespace ResponseService {
        /**
         * @brief Crée un objet RoktResponseObject avec un code de statut, un message optionnel et des données optionnelles.
         * @param code Le code de statut (0 pour OK, autre valeur pour erreur). Les codes négatifs sont corrigés automatiquement.
         * @param message Le message associé à la réponse (par défaut vide).
         * @param datas Les données associées à la réponse (par défaut vide).
         * @return Un pointeur unique vers un RoktResponseObject dans un état valide.
         */
        static std::unique_ptr<ResponseObject> response(
            int code,
            const std::string& message = "",
            const std::string& datas = ""
        ) {
            // Pas de throw : on laisse ResponseObject gérer les codes invalides
            return std::make_unique<ResponseObject>(code, message, datas);
        }
    } // namespace ResponseService
} // namespace ROKT

#endif // ROKT_RESPONSE_SERVICE_H