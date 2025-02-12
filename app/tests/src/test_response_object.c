#include "../Unity/src/unity.c"
#include "../../includes/response.h"

// Configuration avant chaque test (si nécessaire)
void setUp(void) {
    // Pas de configuration requise pour ce test
}

// Nettoyage après chaque test (si nécessaire)
void tearDown(void) {
    // Pas de nettoyage requis pour ce test
}

// Test : Vérifie le code 0 (OK)
void test_response_code_0(void) {
    Response res = response_get(0);
    TEST_ASSERT_EQUAL_INT(0, res.code);
    TEST_ASSERT_EQUAL_STRING("OK", res.phrase);
}

// Test : Vérifie le code 1 (Created)
void test_response_code_1(void) {
    Response res = response_get(1);
    TEST_ASSERT_EQUAL_INT(1, res.code);
    TEST_ASSERT_EQUAL_STRING("Created", res.phrase);
}
// Test : Vérifie le code 2 (Created)
void test_response_code_2(void) {
    Response res = response_get(2);
    TEST_ASSERT_EQUAL_INT(2, res.code);
    TEST_ASSERT_EQUAL_STRING("Inserted", res.phrase);
}
// Test : Vérifie le code 4 (Created)
void test_response_code_4(void) {
    Response res = response_get(4);
    TEST_ASSERT_EQUAL_INT(4, res.code);
    TEST_ASSERT_EQUAL_STRING("Not implemented", res.phrase);
}

// Test : Vérifie le code 10 (Already Exists)
void test_response_code_10(void) {
    Response res = response_get(10);
    TEST_ASSERT_EQUAL_INT(10, res.code);
    TEST_ASSERT_EQUAL_STRING("Already Exists", res.phrase);
}

// Test : Vérifie le code 11 (Already Exists)
void test_response_code_11(void) {
    Response res = response_get(11);
    TEST_ASSERT_EQUAL_INT(11, res.code);
    TEST_ASSERT_EQUAL_STRING("Bad file size format (Needs to be %dMB, %dKB, %dGB)", res.phrase);
}

// Test : Vérifie le code 12 (Already Exists)
void test_response_code_12(void) {
    Response res = response_get(12);
    TEST_ASSERT_EQUAL_INT(12, res.code);
    TEST_ASSERT_EQUAL_STRING("Bad file number format (Needs to be INT)", res.phrase);
}

// Test : Vérifie le code 168 (Already Exists)
void test_response_code_168(void) {
    Response res = response_get(168);
    TEST_ASSERT_EQUAL_INT(168, res.code);
    TEST_ASSERT_EQUAL_STRING("Config file not found", res.phrase);
}

// Test : Vérifie le code 244 (Operator not found)
void test_response_code_244(void) {
    Response res = response_get(244);
    TEST_ASSERT_EQUAL_INT(244, res.code);
    TEST_ASSERT_EQUAL_STRING("Operator not found", res.phrase);
}

// Test : Vérifie le code 567 (Operator not found)
void test_response_code_567(void) {
    Response res = response_get(567);
    TEST_ASSERT_EQUAL_INT(567, res.code);
    TEST_ASSERT_EQUAL_STRING("NULL", res.phrase);
}

// Test : Vérifie un code inconnu (par exemple, 999)
void test_response_code_unknown(void) {
    Response res = response_get(999);
    TEST_ASSERT_EQUAL_INT(999, res.code);
    TEST_ASSERT_EQUAL_STRING("Unknown Error", res.phrase);
}

// Point d'entrée des tests
int main(void) {
    UNITY_BEGIN();

    // Ajouter tous les tests ici
    RUN_TEST(test_response_code_0);
    RUN_TEST(test_response_code_1);
    RUN_TEST(test_response_code_2);
    RUN_TEST(test_response_code_4);
    RUN_TEST(test_response_code_10);
    RUN_TEST(test_response_code_11);
    RUN_TEST(test_response_code_12);
    RUN_TEST(test_response_code_168);
    RUN_TEST(test_response_code_244);
    RUN_TEST(test_response_code_567);
    RUN_TEST(test_response_code_unknown);

    return UNITY_END();
}
