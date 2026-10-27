#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
        tabela_liberar(tabela);
        return 0;
    }
    tabela_liberar(tabela);
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
        tabela_liberar(tabela);
        return 0;
    }
    if (s->tipo != TIPO_INT) {
        printf("FAILED: 'x' has wrong type\n");
        tabela_liberar(tabela);
        return 0;
    }
    
    // Search for a non-existent symbol
    Simbolo* n = tabela_buscar(tabela, "y");
    if (n != NULL) {
        printf("FAILED: found 'y' which was never inserted\n");
        tabela_liberar(tabela);
        return 0;
    }
    
    tabela_liberar(tabela);
    printf("PASSED\n");
    return 1;
}

// Shadowing and scope exit behavior
int test_shadowing_and_scope_exit() {
    printf("Running test_shadowing_and_scope_exit...\n");
    TabelaSimbolos *tabela = tabela_criar();

    // Insert 'x' in scope 0
    tabela_inserir(tabela, "x", TIPO_INT, 1);

    tabela_entrar_escopo(tabela);

    // Insert 'x' in scope 1 (shadowing) and a local-only symbol
    tabela_inserir(tabela, "x", TIPO_STRING, 2);
    tabela_inserir(tabela, "temp", TIPO_FLOAT, 2);

    // tabela_buscar must return the one in scope 1
    Simbolo *s1 = tabela_buscar(tabela, "x");
    if (s1 == NULL || s1->tipo != TIPO_STRING || s1->escopo != 1) {
        printf("FAILED: expected 'x' from scope 1 with type STRING\n");
        tabela_liberar(tabela);
        return 0;
    }

    // Exit scope 1
    tabela_sair_escopo(tabela);

    // tabela_buscar must return the outer 'x' from scope 0
    Simbolo *s0 = tabela_buscar(tabela, "x");
    if (s0 == NULL || s0->tipo != TIPO_INT || s0->escopo != 0) {
        printf("FAILED: expected 'x' from scope 0 with type INT after scope exit\n");
        tabela_liberar(tabela);
        return 0;
    }

    // Symbols from scope 1 must be gone
    if (tabela_buscar(tabela, "temp") != NULL) {
        printf("FAILED: 'temp' from scope 1 should have been deleted\n");
        tabela_liberar(tabela);
        return 0;
    }

    tabela_liberar(tabela);
    printf("PASSED\n");
    return 1;
}

// Growth beyond initial capacity (realloc verification)
int test_table_growth_realloc() {
    printf("Running test_table_growth_realloc...\n");
    TabelaSimbolos *tabela = tabela_criar();

    // Insert 25 symbols (initial capacity is 10)
    char name[32];
    for (int i = 0; i < 25; i++) {
        snprintf(name, sizeof(name), "var_%d", i);
        tabela_inserir(tabela, name, TIPO_INT, i + 1);
    }

    if (tabela->tamanho != 25 || tabela->capacidade < 25) {
        printf("FAILED: table size or capacity did not grow properly\n");
        tabela_liberar(tabela);
        return 0;
    }

    // Verify that every symbol is still found after reallocations
    for (int i = 0; i < 25; i++) {
        snprintf(name, sizeof(name), "var_%d", i);
        Simbolo *s = tabela_buscar(tabela, name);
        if (s == NULL) {
            printf("FAILED: symbol '%s' lost after realloc\n", name);
            tabela_liberar(tabela);
            return 0;
        }
    }

    tabela_liberar(tabela);
    printf("PASSED\n");
    return 1;
}

// Function symbols with allocated tipos_parametros cleaned up properly
int test_function_symbol_cleanup() {
    printf("Running test_function_symbol_cleanup...\n");
    TabelaSimbolos *tabela = tabela_criar();

    tabela_entrar_escopo(tabela);
    tabela_inserir(tabela, "sample_func", TIPO_INT, 1);

    Simbolo *s = tabela_buscar(tabela, "sample_func");
    if (s == NULL) {
        printf("FAILED: function symbol was not inserted\n");
        tabela_liberar(tabela);
        return 0;
    }

    s->e_funcao = true;
    s->num_parametros = 3;
    s->tipos_parametros = malloc(sizeof(TipoDado) * 3);
    s->tipos_parametros[0] = TIPO_INT;
    s->tipos_parametros[1] = TIPO_FLOAT;
    s->tipos_parametros[2] = TIPO_STRING;

    tabela_sair_escopo(tabela);

    tabela_liberar(tabela);
    printf("PASSED\n");
    return 1;


}

int main() {
    int passed = 0;
    int failed = 0;

    if (test_creation()) passed++; else failed++;
    if (test_insertion()) passed++; else failed++;
    if (test_shadowing_and_scope_exit()) passed++; else failed++;
    if (test_table_growth_realloc()) passed++; else failed++;
    if (test_function_symbol_cleanup()) passed++; else failed++;

    printf("Tests run: %d, Passed: %d, Failed: %d\n", passed + failed, passed, failed);
    return failed == 0 ? 0 : 1;
}
