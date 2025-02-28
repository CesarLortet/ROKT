#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include "LogService.h"

#define DEFAULT_BACKLOG 10

class Config {
public:
    struct Encryption {
        std::string passphrase;
        std::string iv;
    };
    struct Network {
        int port;
        int backlog = DEFAULT_BACKLOG;
    };
    struct Thread {
        int maxWorkers;
        int maxTaskQueueSize;
    };

    Encryption encryption;
    Network network;
    Thread thread;

    Config(const std::string& filename);
    bool isValid() const;
private:
    void loadFromEnv();
};

#endif // CONFIG_H