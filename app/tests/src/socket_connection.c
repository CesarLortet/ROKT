#include "../includes/socket_connection.h"

// Fonction pour se connecter à un socket
int connect_to_socket(const char *ip, int port) {
    int sock;
    struct sockaddr_in server;

    // Création du socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("Could not create socket");
        return -1;
    }

    // Configuration de l'adresse du serveur
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &server.sin_addr) <= 0) {
        perror("Invalid IP address");
        close(sock);
        return -1;
    }

    // Connexion au serveur
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Connection failed");
        close(sock);
        return -1;
    }

    // Fermeture du socket après utilisation
    close(sock);
    return 0;
}