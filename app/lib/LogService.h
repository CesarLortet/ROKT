#ifndef LOGSERVICE_H
#define LOGSERVICE_H

#include <string>

class LogService {
public:
    // Renvoie true si la variable d'environnement DEBUG est définie à "1"
    static bool debugEnabled();
    // Affiche le message formaté selon une version simplifiée de RFC 5424
    static void log(const std::string &message);
};

#endif // LOGSERVICE_H
