#include "../Unity/src/unity.c"
#include "../../lib/configuration.h"

// Fichier de configuration pour les tests
#define TEST_CONFIG_FILE "test_config.yml"

// Configuration YAML de test
const char *valid_yaml = "datasets:\n  - dataset1.csv\n  - dataset2.csv\n";
const char *invalid_yaml = "datasets:\n  - [dataset1.csv\n";

// Fonction appelée avant chaque test
void setUp(void) {
    // Supprime le fichier de test s'il existe
    unlink(TEST_CONFIG_FILE);
}

// Fonction appelée après chaque test
void tearDown(void) {
    // Supprime le fichier de test s'il existe
    unlink(TEST_CONFIG_FILE);
}

// Test de création de la configuration
void test_saveConfiguration(void) {
    TEST_ASSERT_TRUE(saveConfiguration(TEST_CONFIG_FILE, valid_yaml));

    // Vérifie que le fichier a été créé
    FILE *file = fopen(TEST_CONFIG_FILE, "r");
    TEST_ASSERT_NOT_NULL(file);

    // Vérifie le contenu du fichier
    char buffer[256];

    (void)!fgets(buffer, sizeof(buffer), file);
    TEST_ASSERT_EQUAL_STRING("datasets:\n", buffer);

    fclose(file);
}

// Test d'affichage de la configuration
void test_showConfiguration(void) {
    // Crée un fichier de configuration
    saveConfiguration(TEST_CONFIG_FILE, valid_yaml);

    // Affiche la configuration
    showConfiguration(TEST_CONFIG_FILE);
}

// Test d'ajout de configuration valide
void test_appendConfiguration_valid(void) {
    saveConfiguration(TEST_CONFIG_FILE, "datasets:\n");

    // Ajoute une nouvelle configuration valide
    TEST_ASSERT_TRUE(appendConfiguration(TEST_CONFIG_FILE, "  - new_dataset.csv\n"));

    // Vérifie le contenu mis à jour
    FILE *file = fopen(TEST_CONFIG_FILE, "r");
    char buffer[256];
    TEST_ASSERT_NOT_NULL(file);
    
    (void)!fgets(buffer, sizeof(buffer), file); // datasets:
    TEST_ASSERT_EQUAL_STRING("datasets:\n", buffer);

    (void)!fgets(buffer, sizeof(buffer), file); //   - new_dataset.csv
    TEST_ASSERT_EQUAL_STRING("  - new_dataset.csv\n", buffer);


    fclose(file);
}

// Test de validation de configuration YAML
void test_validate_yaml(void) {
    TEST_ASSERT_TRUE(validate_yaml(valid_yaml));
    TEST_ASSERT_FALSE(validate_yaml(invalid_yaml));
}

// Test de chiffrement du fichier
void test_encrypt_config_file(void) {
    saveConfiguration(TEST_CONFIG_FILE, valid_yaml);

    // Chiffre le fichier
    TEST_ASSERT_TRUE(encrypt_config_file(TEST_CONFIG_FILE));

    // Vérifie que le fichier est chiffré
    FILE *file = fopen(TEST_CONFIG_FILE, "r");
    TEST_ASSERT_NOT_NULL(file);

    char buffer[256];

    (void)!fread(buffer, 1, sizeof(buffer), file);

    // Le contenu du fichier ne doit pas correspondre au contenu YAML original
    TEST_ASSERT_NOT_EQUAL(0, strcmp(buffer, valid_yaml));

    fclose(file);
}

// Fonction principale des tests
int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_saveConfiguration);
    RUN_TEST(test_showConfiguration);
    RUN_TEST(test_appendConfiguration_valid);
    RUN_TEST(test_showConfiguration);
    RUN_TEST(test_validate_yaml);
    RUN_TEST(test_encrypt_config_file);

    return UNITY_END();
}