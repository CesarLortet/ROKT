#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <string>
#include "RoktResponseObject.h"
#include "RoktResponseService.h"
#include "RoktService.h"

/**
 * @brief Classe de base pour les handlers.
 *
 * Chaque handler doit redéfinir la méthode handle() qui prend en paramètre
 * une commande et une référence à RoktService, et renvoie un RoktResponseObject*.
 */
class CommandHandler {
protected:
    CommandHandler* next;
    RoktService *service;
public:
    CommandHandler(RoktService *service) : next(nullptr), service(service) {}
    /**
     * @brief Définit le handler suivant dans la chaîne.
     * @param nextHandler Le pointeur vers le prochain handler.
     */
    void setNext(CommandHandler* nextHandler) {
        next = nextHandler;
    }
    /**
     * @brief Traite la commande.
     * Si le handler courant ne peut pas la traiter, il la passe au suivant.
     *
     * @param command La commande à traiter.
     * @param service Référence à RoktService.
     * @return RoktResponseObject* Le résultat de la commande.
     */
    virtual RoktResponseObject* handle(const std::string &command) {
        if (next != nullptr) {
            return next->handle(command);
        } else {
            return RoktResponseService::response(423, "Commande non reconnue");
        }
    }
    virtual ~CommandHandler() {}
};

#endif // COMMAND_HANDLER_H
