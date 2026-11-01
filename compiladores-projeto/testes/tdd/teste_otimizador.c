#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/codegen_intermediario.h"
#include "../../src/otimizador.h"

#define ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            printf("  FAILED: %s\n", msg); \
            return 0; \
        } \
    } while(0)

static int test_propagacao_constantes(void) {
    printf("Running test_propagacao_constantes...\n");
    liberar_tac();

    // x = 5
    emitir(cria_instrucao(TAC_ASSIGN, "x", "5", NULL, NULL));
    // y = x + 1
    emitir(cria_instrucao(TAC_BINOP, "y", "x", "1", "+"));
    // z = x * 2
    emitir(cria_instrucao(TAC_BINOP, "z", "x", "2", "*"));

    TAC *tac = obter_tac();
    bool mudou = otimizar_propagacao_constantes(tac);
    ASSERT(mudou == true, "otimizar_propagacao_constantes should report modification");

    ASSERT(strcmp(tac->Instrucoes[1].operando1, "5") == 0, "y = 5 + 1: x should be replaced by 5");
    ASSERT(strcmp(tac->Instrucoes[2].operando1, "5") == 0, "z = 5 * 2: x should be replaced by 5");

    liberar_tac();
    printf("  PASSED\n");
    return 1;
}

static int test_propagacao_com_constant_folding(void) {
    printf("Running test_propagacao_com_constant_folding...\n");
    liberar_tac();

    // x = 5
    emitir(cria_instrucao(TAC_ASSIGN, "x", "5", NULL, NULL));
    // y = x + 1
    emitir(cria_instrucao(TAC_BINOP, "y", "x", "1", "+"));
    // z = x * 2
    emitir(cria_instrucao(TAC_BINOP, "z", "x", "2", "*"));

    TAC *tac = obter_tac();
    int iter = otimizar_tac(tac);
    ASSERT(iter > 0, "otimizar_tac should run at least 1 iteration");

    // After full optimization:
    // tac[0]: x = 5
    // tac[1]: y = 6
    // tac[2]: z = 10
    ASSERT(tac->tamanho == 3, "tac should have 3 instructions");
    ASSERT(tac->Instrucoes[0].tipo == TAC_ASSIGN && strcmp(tac->Instrucoes[0].operando1, "5") == 0, "x should be 5");
    ASSERT(tac->Instrucoes[1].tipo == TAC_ASSIGN && strcmp(tac->Instrucoes[1].operando1, "6") == 0, "y should be 6");
    ASSERT(tac->Instrucoes[2].tipo == TAC_ASSIGN && strcmp(tac->Instrucoes[2].operando1, "10") == 0, "z should be 10");

    liberar_tac();
    printf("  PASSED\n");
    return 1;
}

static int test_remocao_codigo_morto(void) {
    printf("Running test_remocao_codigo_morto...\n");
    liberar_tac();

    // t0 = a + b (t0 never used -> dead)
    emitir(cria_instrucao(TAC_BINOP, "t0", "a", "b", "+"));
    // t1 = c * d
    emitir(cria_instrucao(TAC_BINOP, "t1", "c", "d", "*"));
    // x = t1
    emitir(cria_instrucao(TAC_ASSIGN, "x", "t1", NULL, NULL));

    TAC *tac = obter_tac();
    otimizar_tac(tac);

    // Expected: t0 removed, and t1 collapsed into x = c * d
    ASSERT(tac->tamanho == 1, "tac should have exactly 1 instruction remaining");
    ASSERT(strcmp(tac->Instrucoes[0].resultado, "x") == 0, "resulting instruction must target x");
    ASSERT(tac->Instrucoes[0].tipo == TAC_BINOP, "resulting instruction must be c * d");
    ASSERT(strcmp(tac->Instrucoes[0].operando1, "c") == 0, "op1 must be c");
    ASSERT(strcmp(tac->Instrucoes[0].operando2, "d") == 0, "op2 must be d");
    ASSERT(strcmp(tac->Instrucoes[0].operador, "*") == 0, "operator must be *");

    liberar_tac();
    printf("  PASSED\n");
    return 1;
}

static int test_simplificacoes_algebricas(void) {
    printf("Running test_simplificacoes_algebricas...\n");
    liberar_tac();

    // t0 = x * 1 -> x
    emitir(cria_instrucao(TAC_BINOP, "t0", "x", "1", "*"));
    // t1 = x + 0 -> x
    emitir(cria_instrucao(TAC_BINOP, "t1", "x", "0", "+"));
    // t2 = x * 0 -> 0
    emitir(cria_instrucao(TAC_BINOP, "t2", "x", "0", "*"));
    // t3 = x - 0 -> x
    emitir(cria_instrucao(TAC_BINOP, "t3", "x", "0", "-"));
    // t4 = x / 1 -> x
    emitir(cria_instrucao(TAC_BINOP, "t4", "x", "1", "/"));

    TAC *tac = obter_tac();
    bool mudou = otimizar_constant_folding(tac);
    ASSERT(mudou == true, "simplificacoes algebricas should report changes");

    ASSERT(tac->Instrucoes[0].tipo == TAC_ASSIGN && strcmp(tac->Instrucoes[0].operando1, "x") == 0, "x*1 -> x");
    ASSERT(tac->Instrucoes[1].tipo == TAC_ASSIGN && strcmp(tac->Instrucoes[1].operando1, "x") == 0, "x+0 -> x");
    ASSERT(tac->Instrucoes[2].tipo == TAC_ASSIGN && strcmp(tac->Instrucoes[2].operando1, "0") == 0, "x*0 -> 0");
    ASSERT(tac->Instrucoes[3].tipo == TAC_ASSIGN && strcmp(tac->Instrucoes[3].operando1, "x") == 0, "x-0 -> x");
    ASSERT(tac->Instrucoes[4].tipo == TAC_ASSIGN && strcmp(tac->Instrucoes[4].operando1, "x") == 0, "x/1 -> x");

    liberar_tac();
    printf("  PASSED\n");
    return 1;
}

static int test_fronteira_controle_fluxo(void) {
    printf("Running test_fronteira_controle_fluxo...\n");
    liberar_tac();

    // x = 5
    emitir(cria_instrucao(TAC_ASSIGN, "x", "5", NULL, NULL));
    // L0:
    emitir(cria_instrucao(TAC_LABEL, "L0", NULL, NULL, NULL));
    // y = x + 1 (x should NOT be propagated across label)
    emitir(cria_instrucao(TAC_BINOP, "y", "x", "1", "+"));

    TAC *tac = obter_tac();
    otimizar_propagacao_constantes(tac);

    ASSERT(strcmp(tac->Instrucoes[2].operando1, "x") == 0, "x should NOT be propagated across TAC_LABEL");

    liberar_tac();
    printf("  PASSED\n");
    return 1;
}

static int test_invalidacao_por_reatribuicao(void) {
    printf("Running test_invalidacao_por_reatribuicao...\n");
    liberar_tac();

    // x = 5
    emitir(cria_instrucao(TAC_ASSIGN, "x", "5", NULL, NULL));
    // x = 10
    emitir(cria_instrucao(TAC_ASSIGN, "x", "10", NULL, NULL));
    // y = x
    emitir(cria_instrucao(TAC_ASSIGN, "y", "x", NULL, NULL));

    TAC *tac = obter_tac();
    otimizar_propagacao_constantes(tac);

    ASSERT(strcmp(tac->Instrucoes[2].operando1, "10") == 0, "y = x must use the updated value 10, not 5");

    liberar_tac();
    printf("  PASSED\n");
    return 1;
}

static int test_condicional_float_e_apos_return(void) {
    printf("Running test_condicional_float_e_apos_return...\n");
    liberar_tac();

    // t0 = 10 > 5; IF_FALSE t0 GOTO L0; x = 1.5 + 2.5; L0:
    emitir(cria_instrucao(TAC_BINOP, "t0", "10", "5", ">"));
    emitir(cria_instrucao(TAC_IF_FALSE, "L0", "t0", NULL, NULL));
    emitir(cria_instrucao(TAC_BINOP, "t1", "1.5", "2.5", "+"));
    emitir(cria_instrucao(TAC_ASSIGN, "x", "t1", NULL, NULL));
    emitir(cria_instrucao(TAC_LABEL, "L0", NULL, NULL, NULL));

    // Função com código morto após RETURN
    emitir(cria_instrucao(TAC_FUNC_BEGIN, "f", NULL, NULL, NULL));
    emitir(cria_instrucao(TAC_RETURN, NULL, "42", NULL, NULL));
    emitir(cria_instrucao(TAC_ASSIGN, "inutil", "99", NULL, NULL));
    emitir(cria_instrucao(TAC_FUNC_END, "f", NULL, NULL, NULL));

    TAC *tac = obter_tac();
    otimizar_tac(tac);

    // Esperado:
    // 0: IF_FALSE True GOTO L0 (t0 dobrado, propagado e removido!)
    // 1: x = 4.000000          (1.5 + 2.5 dobrado e t1 coalescido)
    // 2: L0:
    // 3: BEGIN_FUNC f
    // 4: RETURN 42             (inutil = 99 após RETURN removido!)
    // 5: END_FUNC f
    ASSERT(tac->tamanho == 6, "Should fold float, propagate into IF_FALSE, and remove code after RETURN");
    ASSERT(tac->Instrucoes[0].tipo == TAC_IF_FALSE, "First instruction should be IF_FALSE");
    ASSERT(strcmp(tac->Instrucoes[0].operando1, "True") == 0, "Condition in IF_FALSE should be 'True'");
    ASSERT(strcmp(tac->Instrucoes[1].resultado, "x") == 0, "Second instruction should assign x");
    ASSERT(tac->Instrucoes[4].tipo == TAC_RETURN, "Fifth instruction should be RETURN 42");
    ASSERT(tac->Instrucoes[5].tipo == TAC_FUNC_END, "Sixth instruction should be END_FUNC f");

    liberar_tac();
    printf("  PASSED\n");
    return 1;
}

int main(void) {
    printf("=========================================\n");
    printf("   Testes do módulo Otimizador (TAC)     \n");
    printf("=========================================\n");

    int results[] = {
        test_propagacao_constantes(),
        test_propagacao_com_constant_folding(),
        test_remocao_codigo_morto(),
        test_simplificacoes_algebricas(),
        test_fronteira_controle_fluxo(),
        test_invalidacao_por_reatribuicao(),
        test_condicional_float_e_apos_return()
    };

    int passed = 0;
    int failed = 0;
    int n = sizeof(results) / sizeof(results[0]);

    for (int i = 0; i < n; i++) {
        if (results[i]) passed++;
        else failed++;
    }

    printf("=========================================\n");
    printf(" RESULTS: %d passed, %d failed\n", passed, failed);
    printf("=========================================\n");

    return failed == 0 ? 0 : 1;
}
