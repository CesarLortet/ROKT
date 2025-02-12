#ifndef EMPTY_COMMAND_HANDLER_H
#define EMPTY_COMMAND_HANDLER_H

#include "CommandHandler.h"
#include "RoktResponseService.h"
#include "RoktService.h"
#include "RoktDataset.h"
#include "Utils.h" // pour trim()
#include <sstream>

class EmptyCommandHandler : public CommandHandler {
public:
    EmptyCommandHandler(RoktService *service) : CommandHandler(service) {}
    virtual RoktResponseObject* handle(const std::string &command) override {
        std::istringstream iss(command);
        std::string keyword, dataset;
        iss >> keyword >> dataset;
        keyword = trim(keyword);
        dataset = trim(dataset);
        if (keyword != "EMPTY")
            return CommandHandler::handle(command);
        RoktDataset datasetObj = this->service->from(dataset);
        nlohmann::json emptyData = nlohmann::json::array();
        datasetObj.overwrite(emptyData);
        return RoktResponseService::response(0, "OK, table vide");
    }
};

#endif // EMPTY_COMMAND_HANDLER_H
