#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/codegen_intermediario.h"
#include "../../src/ast.h"

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

int test_gerar_tac_expressao_e_atribuicao(void) {
    printf("Running test_gerar_tac_expressao_e_atribuicao...\n");
    liberar_tac();

    /* Cria a AST para: x = a + b * 2 */
    NoAST *num2 = criar_no_int(2, 1);
    NoAST *id_b = criar_no_id("b", 1);
    NoAST *mult = criar_no_binop(OP_MULT, id_b, num2, 1);
    NoAST *id_a = criar_no_id("a", 1);
    NoAST *soma = criar_no_binop(OP_SOMA, id_a, mult, 1);
    NoAST *assign = criar_no_assign("x", soma, 1);

    codegen_gerar_atribuicao(assign);

    TAC *tac = obter_tac();

    if (tac->tamanho != 3) {
        printf("FAILED: esperava 3 instrucoes, obteve %d\n", tac->tamanho);
        liberar_ast(assign);
        liberar_tac();
        return 0;
    }
    
    if (tac->Instrucoes[0].tipo != TAC_BINOP || strcmp(tac->Instrucoes[0].operador, "*") != 0 ||
        strcmp(tac->Instrucoes[1].tipo == TAC_BINOP ? tac->Instrucoes[1].operador : "", "+") != 0 ||
        tac->Instrucoes[2].tipo != TAC_ASSIGN || strcmp(tac->Instrucoes[2].resultado, "x") != 0) {
        printf("FAILED: sequencia de instrucoes incorreta\n");
        liberar_ast(assign);
        liberar_tac();
        return 0;
    }

    liberar_ast(assign);
    liberar_tac();
    printf("PASSED\n");
    return 1;
}

/* Formata uma instrução no mesmo formato de imprimir_codigo_intermediario() */
static void formatar_instrucao(Instrucao inst, char *saida, size_t tamanho) {
    switch (inst.tipo) {
        case TAC_ASSIGN:
            snprintf(saida, tamanho, "%s = %s", inst.resultado, inst.operando1);
            break;
        case TAC_BINOP:
            snprintf(saida, tamanho, "%s = %s %s %s", inst.resultado, inst.operando1, inst.operador, inst.operando2);
            break;
        case TAC_UNOP:
            snprintf(saida, tamanho, "%s = %s%s", inst.resultado, inst.operador, inst.operando1);
            break;
        case TAC_LABEL:
            snprintf(saida, tamanho, "%s:", inst.resultado);
            break;
        case TAC_GOTO:
            snprintf(saida, tamanho, "GOTO %s", inst.resultado);
            break;
        case TAC_IF_FALSE:
            snprintf(saida, tamanho, "IF_FALSE %s GOTO %s", inst.operando1, inst.resultado);
            break;
        default:
            snprintf(saida, tamanho, "?");
            break;
    }
}

/* Gera o TAC de `no`, compara com `esperado` e libera a AST e o TAC */
static int confere_tac(NoAST *no, const char **esperado, int n_esperado) {
    liberar_tac();
    codegen_gerar_tac(no);

    TAC *tac = obter_tac();
    int ok = tac->tamanho == n_esperado;
    if (!ok) {
        printf("FAILED: esperava %d instrucoes, obteve %d\n", n_esperado, tac->tamanho);
    }

    char linha[128];
    for (int i = 0; ok && i < n_esperado; i++) {
        formatar_instrucao(tac->Instrucoes[i], linha, sizeof(linha));
        if (strcmp(linha, esperado[i]) != 0) {
            printf("FAILED: instrucao %d: esperava '%s', obteve '%s'\n", i, esperado[i], linha);
            ok = 0;
        }
    }

    liberar_ast(no);
    liberar_tac();
    return ok;
}

static NoAST *bloco_com_atribuicao(char *nome, NoAST *expr) {
    NoAST **cmds = malloc(sizeof(NoAST *));
    cmds[0] = criar_no_assign(nome, expr, 1);
    return criar_no_bloco(cmds, 1);
}

static NoAST *lista_range(NoAST *a, NoAST *b, NoAST *c) {
    int n = c ? 3 : 2;
    NoAST **args = malloc(n * sizeof(NoAST *));
    args[0] = a;
    args[1] = b;
    if (c) args[2] = c;
    return criar_no_bloco(args, n);
}

int test_if_elif_else(void) {
    printf("Running test_if_elif_else...\n");

    NoAST **elifs = malloc(sizeof(NoAST *));
    elifs[0] = criar_no_elif(criar_no_binop(OP_EQ, criar_no_id("x", 3), criar_no_int(0, 3), 3),
                             bloco_com_atribuicao("y", criar_no_int(0, 4)), 3);
    NoAST *no = criar_no_if(criar_no_binop(OP_GT, criar_no_id("x", 1), criar_no_int(0, 1), 1),
                            bloco_com_atribuicao("y", criar_no_int(1, 2)), elifs, 1,
                            bloco_com_atribuicao("y", criar_no_int(-1, 6)), 1);

    const char *esperado[] = {
        "t0 = x > 0", "IF_FALSE t0 GOTO L0", "y = 1", "GOTO L2", "L0:",
        "t1 = x == 0", "IF_FALSE t1 GOTO L1", "y = 0", "GOTO L2", "L1:",
        "y = -1", "L2:"
    };
    if (!confere_tac(no, esperado, 12)) return 0;

    printf("PASSED\n");
    return 1;
}

int test_if_sem_else(void) {
    printf("Running test_if_sem_else...\n");

    NoAST *simples = criar_no_if(criar_no_id("a", 1), bloco_com_atribuicao("y", criar_no_int(1, 2)),
                                 NULL, 0, NULL, 1);
    const char *esperado1[] = {"IF_FALSE a GOTO L0", "y = 1", "L0:"};
    if (!confere_tac(simples, esperado1, 3)) return 0;

    NoAST **elifs = malloc(sizeof(NoAST *));
    elifs[0] = criar_no_elif(criar_no_id("b", 3), bloco_com_atribuicao("y", criar_no_int(2, 4)), 3);
    NoAST *com_elif = criar_no_if(criar_no_id("a", 1), bloco_com_atribuicao("y", criar_no_int(1, 2)),
                                  elifs, 1, NULL, 1);
    const char *esperado2[] = {
        "IF_FALSE a GOTO L0", "y = 1", "GOTO L1", "L0:",
        "IF_FALSE b GOTO L1", "y = 2", "L1:"
    };
    if (!confere_tac(com_elif, esperado2, 7)) return 0;

    printf("PASSED\n");
    return 1;
}

int test_while(void) {
    printf("Running test_while...\n");

    NoAST *no = criar_no_while(criar_no_binop(OP_GT, criar_no_id("x", 1), criar_no_int(0, 1), 1),
                               bloco_com_atribuicao("x", criar_no_binop(OP_SUB, criar_no_id("x", 2), criar_no_int(1, 2), 2)), 1);

    const char *esperado[] = {
        "L0:", "t0 = x > 0", "IF_FALSE t0 GOTO L1",
        "t1 = x - 1", "x = t1", "GOTO L0", "L1:"
    };
    if (!confere_tac(no, esperado, 7)) return 0;

    printf("PASSED\n");
    return 1;
}

int test_for_range(void) {
    printf("Running test_for_range...\n");

    /* for i in range(n) */
    NoAST *um_arg = criar_no_for("i", criar_no_id("n", 1), bloco_com_atribuicao("s", criar_no_id("i", 2)), 1);
    const char *esperado1[] = {
        "i = 0", "L0:", "t0 = i < n", "IF_FALSE t0 GOTO L1",
        "s = i", "t1 = i + 1", "i = t1", "GOTO L0", "L1:"
    };
    if (!confere_tac(um_arg, esperado1, 9)) return 0;

    /* for i in range(1, 10) */
    NoAST *dois_args = criar_no_for("i", lista_range(criar_no_int(1, 1), criar_no_int(10, 1), NULL),
                                    bloco_com_atribuicao("s", criar_no_id("i", 2)), 1);
    const char *esperado2[] = {
        "i = 1", "L0:", "t0 = i < 10", "IF_FALSE t0 GOTO L1",
        "s = i", "t1 = i + 1", "i = t1", "GOTO L0", "L1:"
    };
    if (!confere_tac(dois_args, esperado2, 9)) return 0;

    /* for i in range(0, 20, 2) */
    NoAST *tres_args = criar_no_for("i", lista_range(criar_no_int(0, 1), criar_no_int(20, 1), criar_no_int(2, 1)),
                                    bloco_com_atribuicao("s", criar_no_id("i", 2)), 1);
    const char *esperado3[] = {
        "i = 0", "L0:", "t0 = i < 20", "IF_FALSE t0 GOTO L1",
        "s = i", "t1 = i + 2", "i = t1", "GOTO L0", "L1:"
    };
    if (!confere_tac(tres_args, esperado3, 9)) return 0;

    /* for i in range(10, 0, -1) */
    NoAST *decrescente = criar_no_for("i", lista_range(criar_no_int(10, 1), criar_no_int(0, 1),
                                      criar_no_unaryop(OP_NEG, criar_no_int(1, 1), 1)),
                                      bloco_com_atribuicao("s", criar_no_id("i", 2)), 1);
    const char *esperado4[] = {
        "t0 = -1", "i = 10", "L0:", "t1 = i > 0", "IF_FALSE t1 GOTO L1",
        "s = i", "t2 = i + t0", "i = t2", "GOTO L0", "L1:"
    };
    if (!confere_tac(decrescente, esperado4, 10)) return 0;

    printf("PASSED\n");
    return 1;
}

int test_rotulos_aninhados(void) {
    printf("Running test_rotulos_aninhados...\n");
    liberar_tac();

    /* for i in range(n):
     *     while a:
     *         if b:
     *             y = 1
     *         else:
     *             y = 2
     */
    NoAST *se = criar_no_if(criar_no_id("b", 3), bloco_com_atribuicao("y", criar_no_int(1, 4)), NULL, 0,
                            bloco_com_atribuicao("y", criar_no_int(2, 6)), 3);
    NoAST **corpo_while = malloc(sizeof(NoAST *));
    corpo_while[0] = se;
    NoAST *enquanto = criar_no_while(criar_no_id("a", 2), criar_no_bloco(corpo_while, 1), 2);
    NoAST **corpo_for = malloc(sizeof(NoAST *));
    corpo_for[0] = enquanto;
    NoAST *laco = criar_no_for("i", criar_no_id("n", 1), criar_no_bloco(corpo_for, 1), 1);

    codegen_gerar_tac(laco);

    TAC *tac = obter_tac();
    int ok = 1;
    int n_rotulos = 0;
    for (int i = 0; i < tac->tamanho; i++) {
        if (tac->Instrucoes[i].tipo != TAC_LABEL && tac->Instrucoes[i].tipo != TAC_GOTO &&
            tac->Instrucoes[i].tipo != TAC_IF_FALSE) continue;

        /* Todo rótulo é definido uma única vez e todo salto tem destino definido */
        int definicoes = 0;
        for (int j = 0; j < tac->tamanho; j++) {
            if (tac->Instrucoes[j].tipo == TAC_LABEL &&
                strcmp(tac->Instrucoes[j].resultado, tac->Instrucoes[i].resultado) == 0) {
                definicoes++;
            }
        }
        if (definicoes != 1) {
            printf("FAILED: rotulo %s definido %d vezes\n", tac->Instrucoes[i].resultado, definicoes);
            ok = 0;
        }
        if (tac->Instrucoes[i].tipo == TAC_LABEL) n_rotulos++;
    }
    if (n_rotulos != 6) {
        printf("FAILED: esperava 6 rotulos, obteve %d\n", n_rotulos);
        ok = 0;
    }

    liberar_ast(laco);
    liberar_tac();
    if (!ok) return 0;

    printf("PASSED\n");
    return 1;
}

int main(void) {
    int passed = 0;
    int failed = 0;

    if (test_temporarios_e_rotulos()) passed++; else failed++;
    if (test_emitir_e_obter_tac()) passed++; else failed++;
    if (test_crescimento_dinamico_realloc()) passed++; else failed++;
    if (test_gerar_tac_expressao_e_atribuicao()) passed++; else failed++;
    if (test_if_elif_else()) passed++; else failed++;
    if (test_if_sem_else()) passed++; else failed++;
    if (test_while()) passed++; else failed++;
    if (test_for_range()) passed++; else failed++;
    if (test_rotulos_aninhados()) passed++; else failed++;
    printf("Tests run: %d, Passed: %d, Failed: %d\n", passed + failed, passed, failed);
    return failed == 0 ? 0 : 1;
}
