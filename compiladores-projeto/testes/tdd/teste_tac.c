#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/codegen_intermediario.h"

int test_temporarios_e_rotulos(void) {
    printf("Running test_temporarios_e_rotulos...\n");
    liberar_tac();

    char *t0 = novo_temporario();
    char *t1 = novo_temporario();
    char *t2 = novo_temporario();

    char *l0 = novo_rotulo();
    char *l1 = novo_rotulo();

    if (strcmp(t0, "t0") != 0 || strcmp(t1, "t1") != 0 || strcmp(t2, "t2") != 0) {
        printf("FAILED: temporarios gerados incorretamente (%s, %s, %s)\n", t0, t1, t2);
        free(t0); free(t1); free(t2); free(l0); free(l1);
        liberar_tac();
        return 0;
    }

    if (strcmp(l0, "L0") != 0 || strcmp(l1, "L1") != 0) {
        printf("FAILED: rotulos gerados incorretamente (%s, %s)\n", l0, l1);
        free(t0); free(t1); free(t2); free(l0); free(l1);
        liberar_tac();
        return 0;
    }

    free(t0);
    free(t1);
    free(t2);
    free(l0);
    free(l1);
    liberar_tac();

    printf("PASSED\n");
    return 1;
}

int test_emitir_e_obter_tac(void) {
    printf("Running test_emitir_e_obter_tac...\n");
    liberar_tac();

    char *t0 = novo_temporario();
    char *l0 = novo_rotulo();

    emitir(cria_instrucao(TAC_ASSIGN, "x", "10", NULL, NULL));
    emitir(cria_instrucao(TAC_BINOP, t0, "x", "5", "+"));
    emitir(cria_instrucao(TAC_IF_FALSE, l0, t0, NULL, NULL));
    emitir(cria_instrucao(TAC_PRINT, NULL, "x", NULL, NULL));
    emitir(cria_instrucao(TAC_LABEL, l0, NULL, NULL, NULL));

    TAC *tac = obter_tac();
    if (tac == NULL) {
        printf("FAILED: obter_tac() retornou NULL\n");
        free(t0); free(l0);
        return 0;
    }

    if (tac->tamanho != 5) {
        printf("FAILED: tamanho esperado 5, obteve %d\n", tac->tamanho);
        free(t0); free(l0);
        liberar_tac();
        return 0;
    }

    if (tac->Instrucoes[0].tipo != TAC_ASSIGN ||
        strcmp(tac->Instrucoes[0].resultado, "x") != 0 ||
        strcmp(tac->Instrucoes[0].operando1, "10") != 0) {
        printf("FAILED: instrucao 0 incorreta\n");
        free(t0); free(l0);
        liberar_tac();
        return 0;
    }

    if (tac->Instrucoes[1].tipo != TAC_BINOP ||
        strcmp(tac->Instrucoes[1].resultado, "t0") != 0 ||
        strcmp(tac->Instrucoes[1].operando1, "x") != 0 ||
        strcmp(tac->Instrucoes[1].operando2, "5") != 0 ||
        strcmp(tac->Instrucoes[1].operador, "+") != 0) {
        printf("FAILED: instrucao 1 incorreta\n");
        free(t0); free(l0);
        liberar_tac();
        return 0;
    }

    printf("--- Saida do imprimir_codigo_intermediario() ---\n");
    imprimir_codigo_intermediario();
    printf("------------------------------------------------\n");

    free(t0);
    free(l0);
    liberar_tac();

    if (tac->tamanho != 0 || tac->Instrucoes != NULL) {
        printf("FAILED: liberar_tac() nao limpou a lista corretamente\n");
        return 0;
    }

    printf("PASSED\n");
    return 1;
}

int test_crescimento_dinamico_realloc(void) {
    printf("Running test_crescimento_dinamico_realloc...\n");
    liberar_tac();

    for (int i = 0; i < 25; i++) {
        char *t = novo_temporario();
        emitir(cria_instrucao(TAC_ASSIGN, t, "1", NULL, NULL));
        free(t);
    }

    TAC *tac = obter_tac();
    if (tac->tamanho != 25 || tac->capacidade < 25) {
        printf("FAILED: esperado tamanho 25, obteve %d (capacidade %d)\n", tac->tamanho, tac->capacidade);
        liberar_tac();
        return 0;
    }

    if (strcmp(tac->Instrucoes[24].resultado, "t24") != 0) {
        printf("FAILED: ultima instrucao deveria ser t24, obteve %s\n", tac->Instrucoes[24].resultado);
        liberar_tac();
        return 0;
    }

    liberar_tac();
    printf("PASSED\n");
    return 1;
}

int main(void) {
    int passed = 0;
    int failed = 0;

    if (test_temporarios_e_rotulos()) passed++; else failed++;
    if (test_emitir_e_obter_tac()) passed++; else failed++;
    if (test_crescimento_dinamico_realloc()) passed++; else failed++;

    printf("Tests run: %d, Passed: %d, Failed: %d\n", passed + failed, passed, failed);
    return failed == 0 ? 0 : 1;
}
