#include "../Unity/src/unity.c"
#include "../../lib/encrypt.h"

// Fonction appelée avant chaque test
void setUp(void) {
}

// Fonction appelée après chaque test
void tearDown(void) {
}

int rettrue(){
    return 1;
}

// Test de création de la configuration
void test_encrypt(void) {

    char my_chain[] = "Ceci est ma chaine super secrete";
    char key[] = "Ceci est ma clef super secrete";

    xor_encrypt_decrypt(my_chain, sizeof(my_chain), key);
    printf("Encrypted: ");
    for (size_t i = 0; i < sizeof(my_chain); i++)
    {
         printf("%02X:", my_chain[i]);
    }
    printf("\n");
    
    xor_encrypt_decrypt(my_chain, sizeof(my_chain), key);
    printf("Decrypted: %s\n", my_chain);
    TEST_ASSERT_TRUE(rettrue());
}

// Fonction principale des tests
int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_encrypt);

    return UNITY_END();
}