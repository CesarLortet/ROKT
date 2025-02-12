#include "../Unity/src/unity.c"
#include "../includes/socket_connection.h"

// Configuration initiale avant chaque test
void setUp(void) {
    // Initialisation si nécessaire
}

// Nettoyage après chaque test
void tearDown(void) {
    // Nettoyage si nécessaire
}

// Test : Connexion à un serveur valide (simulateur ou local)
void test_valid_connection(void) {
    // IP locale et port simulé pour un test
    const char *ip = "127.0.0.1";
    int port = 8047;

    // Démarrer un serveur factice ici si nécessaire (ou utiliser un simulateur)
    TEST_ASSERT_EQUAL(0, connect_to_socket(ip, port));
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_valid_connection);

    return UNITY_END();
}
