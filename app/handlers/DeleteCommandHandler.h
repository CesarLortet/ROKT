#ifndef DELETE_COMMAND_HANDLER_H
#define DELETE_COMMAND_HANDLER_H

#include "CommandHandler.h"
#include "RoktResponseService.h"
#include "RoktService.h"
#include <sstream>
#include "Utils.h" // pour trim()

class DeleteCommandHandler : public CommandHandler {
public:
    DeleteCommandHandler(RoktService *service) : CommandHandler(service) {}
    virtual RoktResponseObject* handle(const std::string &command) override {
        std::istringstream iss(command);
        std::string keyword, dataset;
        iss >> keyword >> dataset;
        keyword = trim(keyword);
        dataset = trim(dataset);
        if (keyword != "DELETE")
            return CommandHandler::handle(command);
        return this->service->drop(dataset);
    }
};

#endif // DELETE_COMMAND_HANDLER_H
