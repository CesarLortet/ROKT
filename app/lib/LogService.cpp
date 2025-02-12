#include "LogService.h"
#include <iostream>
#include <cstdlib>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <unistd.h>      // pour getpid() et gethostname()
#include <sys/utsname.h> // pour obtenir des infos système

bool LogService::debugEnabled() {
    const char* debugEnv = std::getenv("DEBUG");
    return (debugEnv && std::string(debugEnv) == "1");
}

void LogService::log(const std::string &message) {
    if (!debugEnabled()) return;

    // On utilise une version simplifiée du format RFC 5424 :
    // <PRI>VERSION TIMESTAMP HOSTNAME APP-NAME PROCID MSGID - MESSAGE
    // Ici, nous choisissons une valeur fixe pour PRI (par exemple 14) et VERSION = 1.
    int pri = 14;
    int version = 1;

    // Création du timestamp en ISO 8601
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#if defined(_WIN32) || defined(_WIN64)
    localtime_s(&tm, &now_c);
#else
    localtime_r(&now_c, &tm);
#endif
    std::ostringstream timestampStream;
    timestampStream << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S");
    std::string timestamp = timestampStream.str();

    // Récupérer le hostname
    char hostname[256] = {0};
    gethostname(hostname, sizeof(hostname));

    // Obtenir le Process ID
    pid_t pid = getpid();

    // Formatage final (APP-NAME fixé à "rokt_socket", MSGID et Structured Data non utilisés)
    std::ostringstream logStream;
    logStream << "<" << pri << ">" << version << " " << timestamp << " " << hostname
              << " rokt_socket " << pid << " - - " << message;
    
    std::cout << logStream.str() << std::endl;
}
