#ifndef CONFIG_H
#define CONFIG_H

#include <string>

class Config {
public:
    std::string passphrase;
    std::string iv;
    Config(const std::string& configFilePath);
};

#endif // CONFIG_H
