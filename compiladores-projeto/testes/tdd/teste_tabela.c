#include <stdio.h>
#include <stdlib.h>
#include "../../src/tabela.h"

int test_creation() {
    printf("Running test_creation...\n");
    TabelaSimbolos* tabela = tabela_criar();
    if (tabela == NULL) {
        printf("FAILED: tabela is NULL\n");
        return 0;
    }
    if (tabela->escopo_atual != 0) {
        printf("FAILED: initial escopo is %d, expected 0\n", tabela->escopo_atual);
        return 0;
    }
    printf("PASSED\n");
    return 1;
}

int test_insertion() {
    printf("Running test_insertion...\n");
    TabelaSimbolos* tabela = tabela_criar();
    
    // Insert a symbol
    tabela_inserir(tabela, "x", TIPO_INT, 0);
    
    // Search for it
    Simbolo* s = tabela_buscar(tabela, "x");
    if (s == NULL) {
        printf("FAILED: 'x' not found after insertion\n");
        return 0;
    }
    if (s->tipo != TIPO_INT) {
        printf("FAILED: 'x' has wrong type\n");
        return 0;
    }
    
    // Search for a non-existent symbol
    Simbolo* n = tabela_buscar(tabela, "y");
    if (n != NULL) {
        printf("FAILED: found 'y' which was never inserted\n");
        return 0;
    }
    
    printf("PASSED\n");
    return 1;
}

int test_scope() {
    printf("Running test_scope...\n");
    TabelaSimbolos* tabela = tabela_criar();
    
    // Insert 'a' in scope 0
    tabela_inserir(tabela, "a", TIPO_INT, 0);
    
    tabela_entrar_escopo(tabela);
    
    // Insert 'a' again in scope 1
    tabela_inserir(tabela, "a", TIPO_FLOAT, 0);
    
    Simbolo* s1 = tabela_buscar(tabela, "a");
    if (s1 == NULL || s1->tipo != TIPO_FLOAT) {
        printf("FAILED: expected 'a' to be FLOAT in inner scope\n");
        return 0;
    }
    
    tabela_sair_escopo(tabela);
    
    Simbolo* s2 = tabela_buscar(tabela, "a");
    if (s2 == NULL || s2->tipo != TIPO_INT) {
        printf("FAILED: expected 'a' to be INT in outer scope\n");
        return 0;
    }
    
    printf("PASSED\n");
    return 1;
}

int test_cleanup() {
    printf("Running test_cleanup...\n");
    TabelaSimbolos* tabela = tabela_criar();
    tabela_inserir(tabela, "cleanup1", TIPO_INT, 0);
    tabela_entrar_escopo(tabela);
    tabela_inserir(tabela, "cleanup2", TIPO_FLOAT, 0);
    
    // We can't easily assert memory was freed without Valgrind,
    // but we can ensure calling these functions doesn't crash.
    tabela_imprimir(tabela); // Should print the table
    
    tabela_liberar(tabela);
    
    printf("PASSED\n");
    return 1;
}

int main() {
    int passed = 0;
    int failed = 0;

    if (test_creation()) passed++; else failed++;
    if (test_insertion()) passed++; else failed++;
    if (test_scope()) passed++; else failed++;
    if (test_cleanup()) passed++; else failed++;

    printf("Tests run: %d, Passed: %d, Failed: %d\n", passed + failed, passed, failed);
    return failed == 0 ? 0 : 1;
}
