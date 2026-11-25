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

static int test_constant_folding_issue_27(void) {
    printf("Running test_constant_folding_issue_27 (Exemplo exato da Issue #27)...\n");
    liberar_tac();

    // t1 = 3 + 4 -> t1 = 7
    emitir(cria_instrucao(TAC_BINOP, "t1", "3", "4", "+"));
    // t2 = 10 * 2 -> t2 = 20
    emitir(cria_instrucao(TAC_BINOP, "t2", "10", "2", "*"));
    // t3 = t1 + t2 -> t3 = 27 (propaga t1=7 e t2=20, depois dobra)
    emitir(cria_instrucao(TAC_BINOP, "t3", "t1", "t2", "+"));

    TAC *tac = obter_tac();
    bool mudou = otimizar_constant_folding(tac);
    ASSERT(mudou == true, "otimizar_constant_folding should report changes");

    ASSERT(tac->tamanho == 3, "tac should have 3 instructions");
    ASSERT(tac->Instrucoes[0].tipo == TAC_ASSIGN && strcmp(tac->Instrucoes[0].resultado, "t1") == 0 && strcmp(tac->Instrucoes[0].operando1, "7") == 0, "t1 = 7");
    ASSERT(tac->Instrucoes[1].tipo == TAC_ASSIGN && strcmp(tac->Instrucoes[1].resultado, "t2") == 0 && strcmp(tac->Instrucoes[1].operando1, "20") == 0, "t2 = 20");
    ASSERT(tac->Instrucoes[2].tipo == TAC_ASSIGN && strcmp(tac->Instrucoes[2].resultado, "t3") == 0 && strcmp(tac->Instrucoes[2].operando1, "27") == 0, "t3 = 27");

    liberar_tac();
    printf("  PASSED\n");
    return 1;
}

static int test_simplificacoes_algebricas_adicionais(void) {
    printf("Running test_simplificacoes_algebricas_adicionais (x-x, booleanos, comutativas)...\n");
    liberar_tac();

    // t0 = x - x -> 0
    emitir(cria_instrucao(TAC_BINOP, "t0", "x", "x", "-"));
    // t1 = 1 * y -> y
    emitir(cria_instrucao(TAC_BINOP, "t1", "1", "y", "*"));
    // t2 = 0 + z -> z
    emitir(cria_instrucao(TAC_BINOP, "t2", "0", "z", "+"));
    // t3 = 0 * w -> 0
    emitir(cria_instrucao(TAC_BINOP, "t3", "0", "w", "*"));
    // t4 = a and True -> a
    emitir(cria_instrucao(TAC_BINOP, "t4", "a", "True", "and"));
    // t5 = True and b -> b
    emitir(cria_instrucao(TAC_BINOP, "t5", "True", "b", "and"));
    // t6 = c and False -> False
    emitir(cria_instrucao(TAC_BINOP, "t6", "c", "False", "and"));
    // t7 = d or True -> True
    emitir(cria_instrucao(TAC_BINOP, "t7", "d", "True", "or"));
    // t8 = e or False -> e
    emitir(cria_instrucao(TAC_BINOP, "t8", "e", "False", "or"));

    TAC *tac = obter_tac();
    otimizar_constant_folding(tac);

    ASSERT(tac->Instrucoes[0].tipo == TAC_ASSIGN && strcmp(tac->Instrucoes[0].operando1, "0") == 0, "x - x -> 0");
    ASSERT(tac->Instrucoes[1].tipo == TAC_ASSIGN && strcmp(tac->Instrucoes[1].operando1, "y") == 0, "1 * y -> y");
    ASSERT(tac->Instrucoes[2].tipo == TAC_ASSIGN && strcmp(tac->Instrucoes[2].operando1, "z") == 0, "0 + z -> z");
    ASSERT(tac->Instrucoes[3].tipo == TAC_ASSIGN && strcmp(tac->Instrucoes[3].operando1, "0") == 0, "0 * w -> 0");
    ASSERT(tac->Instrucoes[4].tipo == TAC_ASSIGN && strcmp(tac->Instrucoes[4].operando1, "a") == 0, "a and True -> a");
    ASSERT(tac->Instrucoes[5].tipo == TAC_ASSIGN && strcmp(tac->Instrucoes[5].operando1, "b") == 0, "True and b -> b");
    ASSERT(tac->Instrucoes[6].tipo == TAC_ASSIGN && strcmp(tac->Instrucoes[6].operando1, "False") == 0, "c and False -> False");
    ASSERT(tac->Instrucoes[7].tipo == TAC_ASSIGN && strcmp(tac->Instrucoes[7].operando1, "True") == 0, "d or True -> True");
    ASSERT(tac->Instrucoes[8].tipo == TAC_ASSIGN && strcmp(tac->Instrucoes[8].operando1, "e") == 0, "e or False -> e");

    liberar_tac();
    printf("  PASSED\n");
    return 1;
}

static int test_constant_folding_strings(void) {
    printf("Running test_constant_folding_strings...\n");
    liberar_tac();

    // t0 = "hello " + "world" -> "hello world"
    emitir(cria_instrucao(TAC_BINOP, "t0", "\"hello \"", "\"world\"", "+"));
    // t1 = "abc" == "abc" -> True
    emitir(cria_instrucao(TAC_BINOP, "t1", "\"abc\"", "\"abc\"", "=="));
    // t2 = "abc" != "abc" -> False
    emitir(cria_instrucao(TAC_BINOP, "t2", "\"abc\"", "\"abc\"", "!="));

    TAC *tac = obter_tac();
    otimizar_constant_folding(tac);

    ASSERT(tac->Instrucoes[0].tipo == TAC_ASSIGN && strcmp(tac->Instrucoes[0].operando1, "\"hello world\"") == 0, "concatena strings");
    ASSERT(tac->Instrucoes[1].tipo == TAC_ASSIGN && strcmp(tac->Instrucoes[1].operando1, "True") == 0, "strings iguais == True");
    ASSERT(tac->Instrucoes[2].tipo == TAC_ASSIGN && strcmp(tac->Instrucoes[2].operando1, "False") == 0, "strings iguais != False");

    liberar_tac();
    printf("  PASSED\n");
    return 1;
}

static NoAST *programa_ast(int n, NoAST **cmds_estaticos) {
    NoAST **cmds = malloc(sizeof(NoAST *) * n);
    for (int i = 0; i < n; i++) cmds[i] = cmds_estaticos[i];
    return criar_no_programa(cmds, n);
}

static int test_ast_constant_folding_numerico_e_unario(void) {
    printf("Running test_ast_constant_folding_numerico_e_unario...\n");

    NoAST *expr_2_3_4 = criar_no_binop(
        OP_SOMA,
        criar_no_int(2, 1),
        criar_no_binop(OP_MULT, criar_no_int(3, 1), criar_no_int(4, 1), 1),
        1);
    NoAST *expr_5_div_2 = criar_no_binop(OP_DIV, criar_no_int(5, 2), criar_no_int(2, 2), 2);
    NoAST *expr_4_div_2 = criar_no_binop(OP_DIV, criar_no_int(4, 3), criar_no_int(2, 3), 3);
    NoAST *expr_float = criar_no_binop(OP_SOMA, criar_no_float(1.5, 4), criar_no_float(2.5, 4), 4);
    NoAST *expr_rel = criar_no_binop(OP_GT, criar_no_int(10, 5), criar_no_int(5, 5), 5);
    NoAST *expr_neg_int = criar_no_unaryop(OP_NEG, criar_no_int(7, 6), 6);
    NoAST *expr_neg_float = criar_no_unaryop(OP_NEG, criar_no_float(1.5, 7), 7);
    NoAST *expr_not = criar_no_unaryop(OP_NOT, criar_no_bool(1, 8), 8);
    NoAST *expr_div_zero_i = criar_no_binop(OP_DIV, criar_no_int(5, 9), criar_no_int(0, 9), 9);
    NoAST *expr_div_zero_f = criar_no_binop(OP_DIV, criar_no_float(5.0, 10), criar_no_float(0.0, 10), 10);

    NoAST *cmds[] = {
        criar_no_assign("x", expr_2_3_4, 1),
        criar_no_assign("y", expr_5_div_2, 2),
        criar_no_assign("z", expr_4_div_2, 3),
        criar_no_assign("w", expr_float, 4),
        criar_no_assign("b", expr_rel, 5),
        criar_no_assign("ni", expr_neg_int, 6),
        criar_no_assign("nf", expr_neg_float, 7),
        criar_no_assign("nb", expr_not, 8),
        criar_no_assign("dz1", expr_div_zero_i, 9),
        criar_no_assign("dz2", expr_div_zero_f, 10),
    };
    NoAST *raiz = programa_ast(10, cmds);

    int iter = otimizar_ast(raiz);
    ASSERT(iter > 0, "otimizar_ast deve executar ao menos 1 iteracao");

    NoAST *x = raiz->argumentos[0]->esquerda;
    ASSERT(x->tipo == NO_NUM_INT && x->valor_int == 14 && x->tipo_resolvido == TIPO_INT && x->linha == 1,
           "2 + 3 * 4 -> NO_NUM_INT(14) TIPO_INT linha 1");

    NoAST *y = raiz->argumentos[1]->esquerda;
    ASSERT(y->tipo == NO_NUM_FLOAT && y->valor_float == 2.5 && y->tipo_resolvido == TIPO_FLOAT,
           "5 / 2 -> NO_NUM_FLOAT(2.5) TIPO_FLOAT");

    NoAST *z = raiz->argumentos[2]->esquerda;
    ASSERT(z->tipo == NO_NUM_FLOAT && z->valor_float == 2.0 && z->tipo_resolvido == TIPO_FLOAT,
           "4 / 2 -> NO_NUM_FLOAT(2.0) TIPO_FLOAT");

    NoAST *w = raiz->argumentos[3]->esquerda;
    ASSERT(w->tipo == NO_NUM_FLOAT && w->valor_float == 4.0 && w->tipo_resolvido == TIPO_FLOAT,
           "1.5 + 2.5 -> NO_NUM_FLOAT(4.0) TIPO_FLOAT");

    NoAST *b = raiz->argumentos[4]->esquerda;
    ASSERT(b->tipo == NO_BOOL && b->valor_int == 1 && b->tipo_resolvido == TIPO_BOOL,
           "10 > 5 -> NO_BOOL(1) TIPO_BOOL");

    NoAST *ni = raiz->argumentos[5]->esquerda;
    ASSERT(ni->tipo == NO_NUM_INT && ni->valor_int == -7 && ni->tipo_resolvido == TIPO_INT,
           "-7 -> NO_NUM_INT(-7) TIPO_INT");

    NoAST *nf = raiz->argumentos[6]->esquerda;
    ASSERT(nf->tipo == NO_NUM_FLOAT && nf->valor_float == -1.5 && nf->tipo_resolvido == TIPO_FLOAT,
           "-1.5 -> NO_NUM_FLOAT(-1.5) TIPO_FLOAT");

    NoAST *nb = raiz->argumentos[7]->esquerda;
    ASSERT(nb->tipo == NO_BOOL && nb->valor_int == 0 && nb->tipo_resolvido == TIPO_BOOL,
           "not True -> NO_BOOL(0) TIPO_BOOL");

    ASSERT(raiz->argumentos[8]->esquerda->tipo == NO_BINOP, "5 / 0 nao deve ser dobrado");
    ASSERT(raiz->argumentos[9]->esquerda->tipo == NO_BINOP, "5.0 / 0.0 nao deve ser dobrado");

    liberar_ast(raiz);
    printf("  PASSED\n");
    return 1;
}

static int test_ast_constant_folding_booleano_e_strings(void) {
    printf("Running test_ast_constant_folding_booleano_e_strings...\n");

    NoAST *cmds[] = {
        criar_no_assign("b1", criar_no_binop(OP_AND, criar_no_bool(1, 1), criar_no_bool(0, 1), 1), 1),
        criar_no_assign("b2", criar_no_binop(OP_OR, criar_no_bool(0, 2), criar_no_bool(1, 2), 2), 2),
        criar_no_assign("b3", criar_no_binop(OP_EQ, criar_no_bool(1, 3), criar_no_bool(1, 3), 3), 3),
        criar_no_assign("b4", criar_no_binop(OP_GT, criar_no_bool(1, 4), criar_no_bool(0, 4), 4), 4),
        criar_no_assign("s1", criar_no_binop(OP_SOMA, criar_no_string("\"hello \"", 5), criar_no_string("\"world\"", 5), 5), 5),
        criar_no_assign("s2", criar_no_binop(OP_SOMA, criar_no_string("'diz \"oi\"'", 6), criar_no_string("\" !\"", 6), 6), 6),
        criar_no_assign("eq1", criar_no_binop(OP_EQ, criar_no_string("\"abc\"", 7), criar_no_string("\"abc\"", 7), 7), 7),
        criar_no_assign("eq2", criar_no_binop(OP_EQ, criar_no_string("'abc'", 8), criar_no_string("\"abc\"", 8), 8), 8),
        criar_no_assign("neq", criar_no_binop(OP_NEQ, criar_no_string("\"abc\"", 9), criar_no_string("\"def\"", 9), 9), 9),
    };
    NoAST *raiz = programa_ast(9, cmds);

    int iter = otimizar_ast(raiz);
    ASSERT(iter > 0, "otimizar_ast deve dobrar booleanos e strings");

    ASSERT(raiz->argumentos[0]->esquerda->tipo == NO_BOOL && raiz->argumentos[0]->esquerda->valor_int == 0, "True and False -> False");
    ASSERT(raiz->argumentos[1]->esquerda->tipo == NO_BOOL && raiz->argumentos[1]->esquerda->valor_int == 1, "False or True -> True");
    ASSERT(raiz->argumentos[2]->esquerda->tipo == NO_BOOL && raiz->argumentos[2]->esquerda->valor_int == 1, "True == True -> True");
    ASSERT(raiz->argumentos[3]->esquerda->tipo == NO_BOOL && raiz->argumentos[3]->esquerda->valor_int == 1, "True > False -> True");

    NoAST *s1 = raiz->argumentos[4]->esquerda;
    ASSERT(s1->tipo == NO_STRING && strcmp(s1->valor_string, "\"hello world\"") == 0 && s1->tipo_resolvido == TIPO_STRING,
           "\"hello \" + \"world\" -> \"hello world\"");

    NoAST *s2 = raiz->argumentos[5]->esquerda;
    ASSERT(s2->tipo == NO_STRING && strcmp(s2->valor_string, "\"diz \\\"oi\\\" !\"") == 0 && s2->tipo_resolvido == TIPO_STRING,
           "'diz \"oi\"' + \" !\" escapa aspas duplas");

    ASSERT(raiz->argumentos[6]->esquerda->tipo == NO_BOOL && raiz->argumentos[6]->esquerda->valor_int == 1, "\"abc\" == \"abc\" -> True");
    ASSERT(raiz->argumentos[7]->esquerda->tipo == NO_BOOL && raiz->argumentos[7]->esquerda->valor_int == 1, "'abc' == \"abc\" -> True");
    ASSERT(raiz->argumentos[8]->esquerda->tipo == NO_BOOL && raiz->argumentos[8]->esquerda->valor_int == 1, "\"abc\" != \"def\" -> True");

    liberar_ast(raiz);
    printf("  PASSED\n");
    return 1;
}

static NoAST *id_tipado(const char *nome, TipoDado tipo, int linha) {
    NoAST *n = criar_no_id((char *)nome, linha);
    n->tipo_resolvido = tipo;
    return n;
}

static int test_ast_simplificacao_algebrica(void) {
    printf("Running test_ast_simplificacao_algebrica...\n");

    NoAST *cmds[] = {
        criar_no_assign("r1", criar_no_binop(OP_SOMA, id_tipado("x", TIPO_INT, 1), criar_no_int(0, 1), 1), 1),
        criar_no_assign("r2", criar_no_binop(OP_SOMA, criar_no_int(0, 2), id_tipado("x", TIPO_INT, 2), 2), 2),
        criar_no_assign("r3", criar_no_binop(OP_SUB, id_tipado("x", TIPO_INT, 3), criar_no_int(0, 3), 3), 3),
        criar_no_assign("r4", criar_no_binop(OP_MULT, id_tipado("x", TIPO_INT, 4), criar_no_int(1, 4), 4), 4),
        criar_no_assign("r5", criar_no_binop(OP_MULT, criar_no_int(1, 5), id_tipado("x", TIPO_INT, 5), 5), 5),
        criar_no_assign("r6", criar_no_binop(OP_MULT, id_tipado("x", TIPO_INT, 6), criar_no_int(0, 6), 6), 6),
        criar_no_assign("r7", criar_no_binop(OP_MULT, criar_no_int(0, 7), id_tipado("x", TIPO_INT, 7), 7), 7),
        criar_no_assign("r8", criar_no_binop(OP_SUB, id_tipado("x", TIPO_INT, 8), id_tipado("x", TIPO_INT, 8), 8), 8),
        criar_no_assign("b1", criar_no_binop(OP_AND, id_tipado("p", TIPO_BOOL, 9), criar_no_bool(1, 9), 9), 9),
        criar_no_assign("b2", criar_no_binop(OP_AND, criar_no_bool(1, 10), id_tipado("p", TIPO_BOOL, 10), 10), 10),
        criar_no_assign("b3", criar_no_binop(OP_OR, id_tipado("p", TIPO_BOOL, 11), criar_no_bool(0, 11), 11), 11),
        criar_no_assign("b4", criar_no_binop(OP_OR, criar_no_bool(0, 12), id_tipado("p", TIPO_BOOL, 12), 12), 12),
        criar_no_assign("b5", criar_no_binop(OP_AND, id_tipado("p", TIPO_BOOL, 13), criar_no_bool(0, 13), 13), 13),
        criar_no_assign("b6", criar_no_binop(OP_AND, criar_no_bool(0, 14), id_tipado("p", TIPO_BOOL, 14), 14), 14),
        criar_no_assign("b7", criar_no_binop(OP_OR, id_tipado("p", TIPO_BOOL, 15), criar_no_bool(1, 15), 15), 15),
        criar_no_assign("b8", criar_no_binop(OP_OR, criar_no_bool(1, 16), id_tipado("p", TIPO_BOOL, 16), 16), 16),
        criar_no_assign("div_i", criar_no_binop(OP_DIV, id_tipado("x", TIPO_INT, 17), criar_no_int(1, 17), 17), 17),
        criar_no_assign("div_f", criar_no_binop(OP_DIV, id_tipado("f", TIPO_FLOAT, 18), criar_no_int(1, 18), 18), 18),
        criar_no_assign("prom", criar_no_binop(OP_SOMA, id_tipado("x", TIPO_INT, 19), criar_no_float(0.0, 19), 19), 19),
        criar_no_assign("imp1", criar_no_binop(OP_MULT, criar_no_funccall("foo", NULL, 0, 20), criar_no_int(0, 20), 20), 20),
        criar_no_assign("imp2", criar_no_binop(OP_MULT, criar_no_int(0, 21), criar_no_funccall("foo", NULL, 0, 21), 21), 21),
        criar_no_assign("imp3", criar_no_binop(OP_AND, criar_no_funccall("bar", NULL, 0, 22), criar_no_bool(0, 22), 22), 22),
        criar_no_assign("imp4", criar_no_binop(OP_OR, criar_no_funccall("bar", NULL, 0, 23), criar_no_bool(1, 23), 23), 23),
        criar_no_assign("cc1", criar_no_binop(OP_AND, criar_no_bool(0, 24), criar_no_funccall("bar", NULL, 0, 24), 24), 24),
        criar_no_assign("cc2", criar_no_binop(OP_OR, criar_no_bool(1, 25), criar_no_funccall("bar", NULL, 0, 25), 25), 25),
    };
    NoAST *raiz = programa_ast(25, cmds);

    int iter = otimizar_ast(raiz);
    ASSERT(iter > 0, "simplificacao algebrica deve otimizar");

    for (int i = 0; i < 5; i++) {
        NoAST *e = raiz->argumentos[i]->esquerda;
        ASSERT(e->tipo == NO_ID && strcmp(e->nome, "x") == 0, "identidades aditivas/multiplicativas -> x");
    }
    for (int i = 5; i < 8; i++) {
        NoAST *e = raiz->argumentos[i]->esquerda;
        ASSERT(e->tipo == NO_NUM_INT && e->valor_int == 0 && e->tipo_resolvido == TIPO_INT, "x*0, 0*x, x-x -> 0");
    }
    for (int i = 8; i < 12; i++) {
        NoAST *e = raiz->argumentos[i]->esquerda;
        ASSERT(e->tipo == NO_ID && strcmp(e->nome, "p") == 0, "p and True / p or False -> p");
    }
    ASSERT(raiz->argumentos[12]->esquerda->tipo == NO_BOOL && raiz->argumentos[12]->esquerda->valor_int == 0, "p and False -> False");
    ASSERT(raiz->argumentos[13]->esquerda->tipo == NO_BOOL && raiz->argumentos[13]->esquerda->valor_int == 0, "False and p -> False");
    ASSERT(raiz->argumentos[14]->esquerda->tipo == NO_BOOL && raiz->argumentos[14]->esquerda->valor_int == 1, "p or True -> True");
    ASSERT(raiz->argumentos[15]->esquerda->tipo == NO_BOOL && raiz->argumentos[15]->esquerda->valor_int == 1, "True or p -> True");

    ASSERT(raiz->argumentos[16]->esquerda->tipo == NO_BINOP, "x / 1 com x int NAO deve ser simplificado");
    ASSERT(raiz->argumentos[17]->esquerda->tipo == NO_ID && strcmp(raiz->argumentos[17]->esquerda->nome, "f") == 0,
           "f / 1 com f float simplifica para f");
    ASSERT(raiz->argumentos[18]->esquerda->tipo == NO_BINOP, "x + 0.0 com x int NAO simplifica para x");
    ASSERT(raiz->argumentos[19]->esquerda->tipo == NO_BINOP, "foo() * 0 preserva chamada");
    ASSERT(raiz->argumentos[20]->esquerda->tipo == NO_BINOP, "0 * foo() preserva chamada");
    ASSERT(raiz->argumentos[21]->esquerda->tipo == NO_BINOP, "bar() and False preserva chamada");
    ASSERT(raiz->argumentos[22]->esquerda->tipo == NO_BINOP, "bar() or True preserva chamada");
    ASSERT(raiz->argumentos[23]->esquerda->tipo == NO_BOOL && raiz->argumentos[23]->esquerda->valor_int == 0, "False and bar() -> False");
    ASSERT(raiz->argumentos[24]->esquerda->tipo == NO_BOOL && raiz->argumentos[24]->esquerda->valor_int == 1, "True or bar() -> True");

    liberar_ast(raiz);
    printf("  PASSED\n");
    return 1;
}

static int test_ast_propagacao_constantes_e_fronteiras(void) {
    printf("Running test_ast_propagacao_constantes_e_fronteiras...\n");

    /* a = 5; b = a + 3; a = foo(); c = a + 1; a = 10; d = a + 1 */
    NoAST **arg_print = malloc(sizeof(NoAST *));
    arg_print[0] = id_tipado("b", TIPO_INT, 3);

    NoAST **b_while = malloc(sizeof(NoAST *) * 2);
    NoAST **pw = malloc(sizeof(NoAST *));
    pw[0] = id_tipado("i", TIPO_INT, 8);
    b_while[0] = criar_no_print(pw, 1, 8);
    b_while[1] = criar_no_assign("i", criar_no_binop(OP_SOMA, id_tipado("i", TIPO_INT, 9), criar_no_int(1, 9), 9), 9);

    NoAST **b_if = malloc(sizeof(NoAST *));
    b_if[0] = criar_no_assign("x", criar_no_int(2, 12), 12);

    NoAST **b_for = malloc(sizeof(NoAST *));
    b_for[0] = criar_no_assign("k", criar_no_int(9, 15), 15);

    NoAST **b_fn = malloc(sizeof(NoAST *));
    b_fn[0] = criar_no_return(id_tipado("a", TIPO_INT, 17), 17);

    NoAST **p_after_while = malloc(sizeof(NoAST *));
    p_after_while[0] = id_tipado("i", TIPO_INT, 10);

    NoAST **p_after_if = malloc(sizeof(NoAST *));
    p_after_if[0] = id_tipado("x", TIPO_INT, 13);

    NoAST **p_after_for = malloc(sizeof(NoAST *));
    p_after_for[0] = id_tipado("k", TIPO_INT, 16);

    NoAST **p_after_fn = malloc(sizeof(NoAST *));
    p_after_fn[0] = id_tipado("a", TIPO_INT, 18);

    NoAST *cmds[] = {
        criar_no_assign("a", criar_no_int(5, 1), 1),
        criar_no_assign("b", criar_no_binop(OP_SOMA, id_tipado("a", TIPO_INT, 2), criar_no_int(3, 2), 2), 2),
        criar_no_print(arg_print, 1, 3),
        criar_no_assign("a", criar_no_funccall("foo", NULL, 0, 4), 4),
        criar_no_assign("c", criar_no_binop(OP_SOMA, id_tipado("a", TIPO_INT, 5), criar_no_int(1, 5), 5), 5),
        criar_no_assign("a", criar_no_int(10, 6), 6),
        criar_no_assign("d", criar_no_binop(OP_SOMA, id_tipado("a", TIPO_INT, 6), criar_no_int(1, 6), 6), 6),
        criar_no_assign("i", criar_no_int(0, 7), 7),
        criar_no_while(criar_no_binop(OP_LT, id_tipado("i", TIPO_INT, 7), criar_no_int(3, 7), 7),
                       criar_no_bloco(b_while, 2), 7),
        criar_no_print(p_after_while, 1, 10),
        criar_no_assign("x", criar_no_int(1, 11), 11),
        criar_no_if(id_tipado("cond", TIPO_BOOL, 11), criar_no_bloco(b_if, 1), NULL, 0, NULL, 11),
        criar_no_print(p_after_if, 1, 13),
        criar_no_assign("n", criar_no_int(4, 14), 14),
        criar_no_assign("k", criar_no_int(1, 14), 14),
        criar_no_for("idx", id_tipado("n", TIPO_INT, 14), criar_no_bloco(b_for, 1), 14),
        criar_no_print(p_after_for, 1, 16),
        criar_no_funcdef("fn", NULL, 0, criar_no_bloco(b_fn, 1), 17),
        criar_no_print(p_after_fn, 1, 18),
    };
    NoAST *raiz = programa_ast(19, cmds);

    otimizar_ast(raiz);

    ASSERT(raiz->argumentos[1]->esquerda->tipo == NO_NUM_INT &&
           raiz->argumentos[1]->esquerda->valor_int == 8, "b = a + 3 -> 8");
    ASSERT(raiz->argumentos[2]->argumentos[0]->tipo == NO_NUM_INT &&
           raiz->argumentos[2]->argumentos[0]->valor_int == 8, "print(b) -> print(8)");
    ASSERT(raiz->argumentos[4]->esquerda->tipo == NO_BINOP &&
           raiz->argumentos[4]->esquerda->esquerda->tipo == NO_ID, "a = foo() invalida a");
    ASSERT(raiz->argumentos[6]->esquerda->tipo == NO_NUM_INT &&
           raiz->argumentos[6]->esquerda->valor_int == 11, "a = 10; d = a + 1 -> 11");

    NoAST *w = raiz->argumentos[8];
    ASSERT(w->condicao->esquerda->tipo == NO_ID, "while condicao NAO propaga i=0");
    ASSERT(w->bloco_if->argumentos[0]->argumentos[0]->tipo == NO_ID, "while corpo NAO propaga i=0");
    ASSERT(raiz->argumentos[9]->argumentos[0]->tipo == NO_ID, "apos while NAO propaga i=0");

    ASSERT(raiz->argumentos[12]->argumentos[0]->tipo == NO_ID, "apos if NAO propaga x=1");

    NoAST *f = raiz->argumentos[15];
    ASSERT(f->esquerda->tipo == NO_NUM_INT && f->esquerda->valor_int == 4, "for range(n) propaga n=4");
    ASSERT(raiz->argumentos[16]->argumentos[0]->tipo == NO_ID, "apos for NAO propaga k");

    NoAST *fn = raiz->argumentos[17];
    ASSERT(fn->bloco_if->argumentos[0]->esquerda->tipo == NO_ID, "funcdef nao herda a=10 global");
    ASSERT(raiz->argumentos[18]->argumentos[0]->tipo == NO_ID, "apos funcdef a nao foi sobrescrito");

    liberar_ast(raiz);
    printf("  PASSED\n");
    return 1;
}

static int test_ast_remocao_codigo_morto(void) {
    printf("Running test_ast_remocao_codigo_morto...\n");

    /* def f(cond): if True: return 42; x = 1 */
    NoAST **p_f = malloc(sizeof(NoAST *));
    p_f[0] = id_tipado("cond", TIPO_BOOL, 1);

    NoAST **b_if_true_ret = malloc(sizeof(NoAST *));
    b_if_true_ret[0] = criar_no_return(criar_no_int(42, 2), 2);
    NoAST **b_else_dead = malloc(sizeof(NoAST *));
    b_else_dead[0] = criar_no_return(criar_no_int(0, 3), 3);

    NoAST **b_fn = malloc(sizeof(NoAST *) * 2);
    b_fn[0] = criar_no_if(criar_no_bool(1, 2), criar_no_bloco(b_if_true_ret, 1), NULL, 0, criar_no_bloco(b_else_dead, 1), 2);
    b_fn[1] = criar_no_assign("morto", criar_no_int(99, 4), 4);
    NoAST *fn = criar_no_funcdef("f", p_f, 1, criar_no_bloco(b_fn, 2), 1);

    /* while 1 > 2: print(1) */
    NoAST **pw = malloc(sizeof(NoAST *));
    pw[0] = criar_no_int(1, 6);
    NoAST **bw = malloc(sizeof(NoAST *));
    bw[0] = criar_no_print(pw, 1, 6);
    NoAST *w_dead = criar_no_while(criar_no_binop(OP_GT, criar_no_int(1, 5), criar_no_int(2, 5), 5),
                                   criar_no_bloco(bw, 1), 5);

    /* if False: dead1 = 1 */
    NoAST **b_if_f1 = malloc(sizeof(NoAST *));
    b_if_f1[0] = criar_no_assign("dead1", criar_no_int(1, 7), 7);
    NoAST *if_f_solo = criar_no_if(criar_no_bool(0, 7), criar_no_bloco(b_if_f1, 1), NULL, 0, NULL, 7);

    /* if False: dead2 = 1 else: live_else = 7 */
    NoAST **b_if_f2 = malloc(sizeof(NoAST *));
    b_if_f2[0] = criar_no_assign("dead2", criar_no_int(1, 8), 8);
    NoAST **b_else_live = malloc(sizeof(NoAST *));
    b_else_live[0] = criar_no_assign("live_else", criar_no_int(7, 9), 9);
    NoAST *if_f_else = criar_no_if(criar_no_bool(0, 8), criar_no_bloco(b_if_f2, 1), NULL, 0, criar_no_bloco(b_else_live, 1), 8);

    /* if False: dead3 = 1 elif cond_ext: live_elif = 8 */
    NoAST **b_if_f3 = malloc(sizeof(NoAST *));
    b_if_f3[0] = criar_no_assign("dead3", criar_no_int(1, 10), 10);
    NoAST **b_elif_live = malloc(sizeof(NoAST *));
    b_elif_live[0] = criar_no_assign("live_elif", criar_no_int(8, 11), 11);
    NoAST **elifs = malloc(sizeof(NoAST *));
    elifs[0] = criar_no_elif(id_tipado("cond_ext", TIPO_BOOL, 11), criar_no_bloco(b_elif_live, 1), 11);
    NoAST *if_f_elif = criar_no_if(criar_no_bool(0, 10), criar_no_bloco(b_if_f3, 1), elifs, 1, NULL, 10);

    /* flag = True; if flag: live_flag = 12 */
    NoAST **b_if_flag = malloc(sizeof(NoAST *));
    b_if_flag[0] = criar_no_assign("live_flag", criar_no_int(12, 13), 13);
    NoAST *if_flag = criar_no_if(id_tipado("flag", TIPO_BOOL, 13), criar_no_bloco(b_if_flag, 1), NULL, 0, NULL, 13);

    NoAST *cmds[] = {
        fn,
        w_dead,
        if_f_solo,
        if_f_else,
        if_f_elif,
        criar_no_assign("flag", criar_no_bool(1, 12), 12),
        if_flag,
    };
    NoAST *raiz = programa_ast(7, cmds);

    otimizar_ast(raiz);

    /* Dentro de f(): apenas return 42 deve permanecer */
    ASSERT(fn->bloco_if->num_argumentos == 1 && fn->bloco_if->argumentos[0]->tipo == NO_RETURN,
           "if True splice + eliminacao pos-return em f()");

    /* No programa global: fn, live_else=7, if(cond_ext), flag=True, live_flag=12 */
    ASSERT(raiz->num_argumentos == 5, "while False e if False solo removidos, else e if True com splice");
    ASSERT(raiz->argumentos[0]->tipo == NO_FUNCDEF, "fn preservada no indice 0");
    ASSERT(raiz->argumentos[1]->tipo == NO_ASSIGN && strcmp(raiz->argumentos[1]->nome, "live_else") == 0,
           "if False + else -> splice de live_else");
    ASSERT(raiz->argumentos[2]->tipo == NO_IF && raiz->argumentos[2]->num_argumentos == 0 &&
           raiz->argumentos[2]->condicao->tipo == NO_ID && strcmp(raiz->argumentos[2]->condicao->nome, "cond_ext") == 0,
           "if False + elif -> elif promovido a if");
    ASSERT(raiz->argumentos[3]->tipo == NO_ASSIGN && strcmp(raiz->argumentos[3]->nome, "flag") == 0,
           "flag = True mantido");
    ASSERT(raiz->argumentos[4]->tipo == NO_ASSIGN && strcmp(raiz->argumentos[4]->nome, "live_flag") == 0,
           "if flag (True) -> splice de live_flag");

    liberar_ast(raiz);
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
        test_condicional_float_e_apos_return(),
        test_constant_folding_issue_27(),
        test_simplificacoes_algebricas_adicionais(),
        test_constant_folding_strings(),
        test_ast_constant_folding_numerico_e_unario(),
        test_ast_constant_folding_booleano_e_strings(),
        test_ast_simplificacao_algebrica(),
        test_ast_propagacao_constantes_e_fronteiras(),
        test_ast_remocao_codigo_morto()
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
