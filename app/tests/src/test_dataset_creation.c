#include "../Unity/src/unity.c"
#include "../../lib/configuration.h"
#include "../../includes/dataset.h"

#include <unistd.h>
#include <sys/stat.h>

// Chemin vers les fichiers pour les tests
#define TEST_CONFIG_FILE "config.yml"
#define TEST_DATASET_DIR "datasets"

// Fonctions d'initialisation et de nettoyage
void setUp(void) {
    // Supprime les fichiers précédents pour les tests
    unlink(TEST_CONFIG_FILE);
    unlink(TEST_DATASET_DIR "/simple_dataset.rokt");
    unlink(TEST_DATASET_DIR "/rotate_dataset.rokt");

    // Crée le dossier datasets s'il n'existe pas
    mkdir(TEST_DATASET_DIR, 0755);

    // Initialise un fichier de configuration vide
    saveConfiguration(TEST_CONFIG_FILE, "datasets:\n");
}

void tearDown(void) {
    // Supprime les fichiers créés pendant les tests
    unlink(TEST_CONFIG_FILE);
    unlink(TEST_DATASET_DIR "/simple_dataset.rokt");
    unlink(TEST_DATASET_DIR "/rotate_dataset.rokt");

    // Supprime le dossier datasets s'il est vide
    rmdir(TEST_DATASET_DIR);
}

// Test : Création d'une table SIMPLE
void test_create_simple_dataset(void) {
    TEST_ASSERT_TRUE(create_dataset("simple_dataset", SIMPLE, NULL));

    // Vérifie que la table a été ajoutée à config.yml
    FILE *file = fopen(TEST_CONFIG_FILE, "r");
    TEST_ASSERT_NOT_NULL(file);

    char buffer[256];
    (void)!fgets(buffer, sizeof(buffer), file); // datasets:
    TEST_ASSERT_EQUAL_STRING("datasets:\n", buffer);

    (void)!fgets(buffer, sizeof(buffer), file); //   - simple_dataset
    TEST_ASSERT_EQUAL_STRING("  - simple_dataset\n", buffer);

    fclose(file);

    // Vérifie que le fichier .rokt existe
    char dataset_path[256];
    (void)!sprintf(dataset_path, TEST_DATASET_DIR "/simple_dataset.rokt");
    TEST_ASSERT_TRUE(access(dataset_path, F_OK) == 0);
}

// Test : Création d'une table ROTATE
void test_create_rotate_dataset(void) {
    TEST_ASSERT_TRUE(create_dataset("rotate_dataset", ROTATE, "10Mo"));

    // Vérifie que la table a été ajoutée à config.yml
    FILE *file = fopen(TEST_CONFIG_FILE, "r");
    TEST_ASSERT_NOT_NULL(file);

    char buffer[256];
    (void)!fgets(buffer, sizeof(buffer), file); // datasets:
    TEST_ASSERT_EQUAL_STRING("datasets:\n", buffer);

    (void)!fgets(buffer, sizeof(buffer), file); //   - rotate_dataset
    TEST_ASSERT_EQUAL_STRING("  - rotate_dataset:\n", buffer);

    (void)!fgets(buffer, sizeof(buffer), file); //   - rotate_dataset
    TEST_ASSERT_EQUAL_STRING("    type: ROTATE\n", buffer);
    
    (void)!fgets(buffer, sizeof(buffer), file); //   - rotate_dataset
    TEST_ASSERT_EQUAL_STRING("    size: 10Mo", buffer);

    fclose(file);

    // Vérifie que le fichier .rokt existe
    char dataset_path[256];
    (void)!sprintf(dataset_path, TEST_DATASET_DIR "/rotate_dataset.rokt");
    TEST_ASSERT_TRUE(access(dataset_path, F_OK) == 0);
}

void t() {
    create_dataset("simple_dataset", SIMPLE, NULL);
    create_dataset("rotate_dataset", ROTATE, "10Mo");
}

// Test : Échec pour un nom de table vide
void test_create_dataset_invalid_name(void) {
    TEST_ASSERT_FALSE(create_dataset("", SIMPLE, NULL));
}

// Test : Échec pour une taille ROTATE invalide
void test_create_dataset_invalid_rotate_size(void) {
    TEST_ASSERT_FALSE(create_dataset("invalid_rotate_dataset", ROTATE, "100XYZ"));
}

// Test : Création d'une table qui existe déjà
void test_create_dataset_already_exists(void) {
    // Création initiale
    TEST_ASSERT_TRUE(create_dataset("simple_dataset", SIMPLE, NULL));

    // Tente de recréer la même table
    TEST_ASSERT_FALSE(create_dataset("simple_dataset", SIMPLE, NULL));
}

// Fonction principale pour exécuter les tests
int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_create_simple_dataset);
    RUN_TEST(test_create_rotate_dataset);
    RUN_TEST(test_create_dataset_invalid_name);
    RUN_TEST(test_create_dataset_invalid_rotate_size);
    RUN_TEST(test_create_dataset_already_exists);

    return UNITY_END();
}