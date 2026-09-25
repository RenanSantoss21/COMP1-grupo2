/* ═══════════════════════════════════════════════════════════════════
 * teste_ast.c — Testes unitários para o módulo AST (ast.h / ast.c)
 *
 * Seam: API pública de ast.h (criar_no_*, imprimir_ast, liberar_ast)
 * Prior art: testes/tdd/teste_tabela.c
 * ═══════════════════════════════════════════════════════════════════ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/ast.h"

/* ── Contadores globais ──────────────────────────────────────────── */
static int passed = 0;
static int failed = 0;

#define ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            printf("  FAILED: %s\n", msg); \
            return 0; \
        } \
    } while(0)

/* ═════════════════════════════════════════════════════════════════════
 * Slice 1 – Literal constructors produce correct type and value
 * ═════════════════════════════════════════════════════════════════════ */

static int test_criar_no_int(void) {
    printf("Running test_criar_no_int...\n");
    NoAST *no = criar_no_int(42, 1);
    ASSERT(no != NULL, "node is NULL");
    ASSERT(no->tipo == NO_NUM_INT, "tipo should be NO_NUM_INT");
    ASSERT(no->valor_int == 42, "valor_int should be 42");
    ASSERT(no->linha == 1, "linha should be 1");
    liberar_ast(no);
    printf("  PASSED\n");
    return 1;
}

static int test_criar_no_float(void) {
    printf("Running test_criar_no_float...\n");
    NoAST *no = criar_no_float(3.14, 2);
    ASSERT(no != NULL, "node is NULL");
    ASSERT(no->tipo == NO_NUM_FLOAT, "tipo should be NO_NUM_FLOAT");
    ASSERT(no->valor_float > 3.13 && no->valor_float < 3.15,
           "valor_float should be ~3.14");
    ASSERT(no->linha == 2, "linha should be 2");
    liberar_ast(no);
    printf("  PASSED\n");
    return 1;
}

static int test_criar_no_string(void) {
    printf("Running test_criar_no_string...\n");
    NoAST *no = criar_no_string("hello", 3);
    ASSERT(no != NULL, "node is NULL");
    ASSERT(no->tipo == NO_STRING, "tipo should be NO_STRING");
    ASSERT(strcmp(no->valor_string, "hello") == 0,
           "valor_string should be 'hello'");
    ASSERT(no->linha == 3, "linha should be 3");
    liberar_ast(no);
    printf("  PASSED\n");
    return 1;
}

static int test_criar_no_bool_true(void) {
    printf("Running test_criar_no_bool_true...\n");
    NoAST *no = criar_no_bool(1, 4);
    ASSERT(no != NULL, "node is NULL");
    ASSERT(no->tipo == NO_BOOL, "tipo should be NO_BOOL");
    ASSERT(no->valor_int == 1, "valor_int should be 1 (True)");
    ASSERT(no->linha == 4, "linha should be 4");
    liberar_ast(no);
    printf("  PASSED\n");
    return 1;
}

static int test_criar_no_bool_false(void) {
    printf("Running test_criar_no_bool_false...\n");
    NoAST *no = criar_no_bool(0, 5);
    ASSERT(no != NULL, "node is NULL");
    ASSERT(no->tipo == NO_BOOL, "tipo should be NO_BOOL");
    ASSERT(no->valor_int == 0, "valor_int should be 0 (False)");
    liberar_ast(no);
    printf("  PASSED\n");
    return 1;
}

static int test_criar_no_id(void) {
    printf("Running test_criar_no_id...\n");
    NoAST *no = criar_no_id("variavel", 6);
    ASSERT(no != NULL, "node is NULL");
    ASSERT(no->tipo == NO_ID, "tipo should be NO_ID");
    ASSERT(strcmp(no->nome, "variavel") == 0, "nome should be 'variavel'");
    ASSERT(no->linha == 6, "linha should be 6");
    liberar_ast(no);
    printf("  PASSED\n");
    return 1;
}

/* ═════════════════════════════════════════════════════════════════════
 * Slice 2 – BinOp and UnaryOp link children correctly
 * ═════════════════════════════════════════════════════════════════════ */

static int test_criar_no_binop(void) {
    printf("Running test_criar_no_binop...\n");
    NoAST *esq = criar_no_int(10, 7);
    NoAST *dir = criar_no_int(20, 7);
    NoAST *binop = criar_no_binop(OP_SOMA, esq, dir, 7);

    ASSERT(binop != NULL, "binop is NULL");
    ASSERT(binop->tipo == NO_BINOP, "tipo should be NO_BINOP");
    ASSERT(binop->operador == OP_SOMA, "operador should be OP_SOMA");
    ASSERT(binop->esquerda == esq, "esquerda should point to left child");
    ASSERT(binop->direita == dir, "direita should point to right child");
    ASSERT(binop->esquerda->valor_int == 10, "left child value should be 10");
    ASSERT(binop->direita->valor_int == 20, "right child value should be 20");

    liberar_ast(binop);
    printf("  PASSED\n");
    return 1;
}

static int test_criar_no_unaryop(void) {
    printf("Running test_criar_no_unaryop...\n");
    NoAST *operando = criar_no_bool(1, 8);
    NoAST *unary = criar_no_unaryop(OP_NOT, operando, 8);

    ASSERT(unary != NULL, "unary is NULL");
    ASSERT(unary->tipo == NO_UNARYOP, "tipo should be NO_UNARYOP");
    ASSERT(unary->operador == OP_NOT, "operador should be OP_NOT");
    ASSERT(unary->esquerda == operando, "esquerda should point to operand");

    liberar_ast(unary);
    printf("  PASSED\n");
    return 1;
}

static int test_binop_nested(void) {
    printf("Running test_binop_nested...\n");
    /* (10 + 5) * 2 */
    NoAST *soma = criar_no_binop(OP_SOMA,
                                 criar_no_int(10, 1),
                                 criar_no_int(5, 1), 1);
    NoAST *mult = criar_no_binop(OP_MULT, soma, criar_no_int(2, 1), 1);

    ASSERT(mult->tipo == NO_BINOP, "root should be NO_BINOP");
    ASSERT(mult->operador == OP_MULT, "root operator should be OP_MULT");
    ASSERT(mult->esquerda->tipo == NO_BINOP,
           "left child should be NO_BINOP (soma)");
    ASSERT(mult->esquerda->operador == OP_SOMA,
           "left child operator should be OP_SOMA");
    ASSERT(mult->direita->tipo == NO_NUM_INT,
           "right child should be NO_NUM_INT");
    ASSERT(mult->direita->valor_int == 2, "right child value should be 2");

    liberar_ast(mult);
    printf("  PASSED\n");
    return 1;
}

/* ═════════════════════════════════════════════════════════════════════
 * Slice 3 – Assign
 * ═════════════════════════════════════════════════════════════════════ */

static int test_criar_no_assign(void) {
    printf("Running test_criar_no_assign...\n");
    NoAST *expr = criar_no_int(99, 10);
    NoAST *assign = criar_no_assign("x", expr, 10);

    ASSERT(assign->tipo == NO_ASSIGN, "tipo should be NO_ASSIGN");
    ASSERT(strcmp(assign->nome, "x") == 0, "nome should be 'x'");
    ASSERT(assign->esquerda == expr, "esquerda should point to expr");
    ASSERT(assign->esquerda->valor_int == 99, "expr value should be 99");

    liberar_ast(assign);
    printf("  PASSED\n");
    return 1;
}

/* ═════════════════════════════════════════════════════════════════════
 * Slice 4 – Print and Input
 * ═════════════════════════════════════════════════════════════════════ */

static int test_criar_no_print(void) {
    printf("Running test_criar_no_print...\n");
    NoAST **args = malloc(2 * sizeof(NoAST *));
    args[0] = criar_no_string("hello", 11);
    args[1] = criar_no_id("x", 11);
    NoAST *print_no = criar_no_print(args, 2, 11);

    ASSERT(print_no->tipo == NO_PRINT, "tipo should be NO_PRINT");
    ASSERT(print_no->num_argumentos == 2, "should have 2 arguments");
    ASSERT(print_no->argumentos[0]->tipo == NO_STRING,
           "first arg should be string");
    ASSERT(print_no->argumentos[1]->tipo == NO_ID,
           "second arg should be id");

    liberar_ast(print_no);
    printf("  PASSED\n");
    return 1;
}

static int test_criar_no_input(void) {
    printf("Running test_criar_no_input...\n");
    NoAST *prompt = criar_no_string("Digite: ", 12);
    NoAST *input_no = criar_no_input(prompt, 12);

    ASSERT(input_no->tipo == NO_INPUT, "tipo should be NO_INPUT");
    ASSERT(input_no->esquerda == prompt, "esquerda should be prompt");

    liberar_ast(input_no);
    printf("  PASSED\n");
    return 1;
}

static int test_criar_no_input_sem_prompt(void) {
    printf("Running test_criar_no_input_sem_prompt...\n");
    NoAST *input_no = criar_no_input(NULL, 13);

    ASSERT(input_no->tipo == NO_INPUT, "tipo should be NO_INPUT");
    ASSERT(input_no->esquerda == NULL, "esquerda should be NULL");

    liberar_ast(input_no);
    printf("  PASSED\n");
    return 1;
}

/* ═════════════════════════════════════════════════════════════════════
 * Slice 5 – If / Elif / While / For
 * ═════════════════════════════════════════════════════════════════════ */

static int test_criar_no_if_simples(void) {
    printf("Running test_criar_no_if_simples...\n");
    NoAST *cond = criar_no_bool(1, 14);
    NoAST **cmds = malloc(sizeof(NoAST *));
    cmds[0] = criar_no_print(NULL, 0, 15);
    NoAST *bloco = criar_no_bloco(cmds, 1);
    NoAST *if_no = criar_no_if(cond, bloco, NULL, 0, NULL, 14);

    ASSERT(if_no->tipo == NO_IF, "tipo should be NO_IF");
    ASSERT(if_no->condicao == cond, "condicao should point to cond");
    ASSERT(if_no->bloco_if == bloco, "bloco_if should point to block");
    ASSERT(if_no->num_argumentos == 0, "no elifs expected");
    ASSERT(if_no->bloco_else == NULL, "no else expected");

    liberar_ast(if_no);
    printf("  PASSED\n");
    return 1;
}

static int test_criar_no_if_com_elif_e_else(void) {
    printf("Running test_criar_no_if_com_elif_e_else...\n");
    NoAST *cond = criar_no_bool(1, 20);
    NoAST **cmds_if = malloc(sizeof(NoAST *));
    cmds_if[0] = criar_no_int(1, 21);
    NoAST *bloco_if = criar_no_bloco(cmds_if, 1);

    /* elif */
    NoAST *cond_elif = criar_no_bool(0, 22);
    NoAST **cmds_elif = malloc(sizeof(NoAST *));
    cmds_elif[0] = criar_no_int(2, 23);
    NoAST *bloco_elif = criar_no_bloco(cmds_elif, 1);
    NoAST *elif_no = criar_no_elif(cond_elif, bloco_elif, 22);

    NoAST **elifs = malloc(sizeof(NoAST *));
    elifs[0] = elif_no;

    /* else */
    NoAST **cmds_else = malloc(sizeof(NoAST *));
    cmds_else[0] = criar_no_int(3, 25);
    NoAST *bloco_else = criar_no_bloco(cmds_else, 1);

    NoAST *if_no = criar_no_if(cond, bloco_if, elifs, 1, bloco_else, 20);

    ASSERT(if_no->tipo == NO_IF, "tipo should be NO_IF");
    ASSERT(if_no->num_argumentos == 1, "should have 1 elif");
    ASSERT(if_no->argumentos[0]->tipo == NO_ELIF,
           "elif node should be NO_ELIF");
    ASSERT(if_no->bloco_else != NULL, "should have else block");
    ASSERT(if_no->bloco_else->tipo == NO_BLOCO,
           "else should be a block");

    liberar_ast(if_no);
    printf("  PASSED\n");
    return 1;
}

static int test_criar_no_while(void) {
    printf("Running test_criar_no_while...\n");
    NoAST *cond = criar_no_binop(OP_LT, criar_no_id("i", 30),
                                 criar_no_int(10, 30), 30);
    NoAST **cmds = malloc(sizeof(NoAST *));
    cmds[0] = criar_no_id("i", 31);
    NoAST *bloco = criar_no_bloco(cmds, 1);
    NoAST *while_no = criar_no_while(cond, bloco, 30);

    ASSERT(while_no->tipo == NO_WHILE, "tipo should be NO_WHILE");
    ASSERT(while_no->condicao == cond, "condicao should point to cond");
    ASSERT(while_no->bloco_if == bloco, "bloco_if should point to block");

    liberar_ast(while_no);
    printf("  PASSED\n");
    return 1;
}

static int test_criar_no_for(void) {
    printf("Running test_criar_no_for...\n");
    NoAST *range = criar_no_int(10, 35);
    NoAST **cmds = malloc(sizeof(NoAST *));
    cmds[0] = criar_no_id("i", 36);
    NoAST *bloco = criar_no_bloco(cmds, 1);
    NoAST *for_no = criar_no_for("i", range, bloco, 35);

    ASSERT(for_no->tipo == NO_FOR, "tipo should be NO_FOR");
    ASSERT(strcmp(for_no->nome, "i") == 0, "nome should be 'i'");
    ASSERT(for_no->esquerda == range, "esquerda should be range");
    ASSERT(for_no->bloco_if == bloco, "bloco_if should be block");

    liberar_ast(for_no);
    printf("  PASSED\n");
    return 1;
}

/* ═════════════════════════════════════════════════════════════════════
 * Slice 6 – FuncDef, Return, FuncCall
 * ═════════════════════════════════════════════════════════════════════ */

static int test_criar_no_funcdef(void) {
    printf("Running test_criar_no_funcdef...\n");
    NoAST **params = malloc(2 * sizeof(NoAST *));
    params[0] = criar_no_id("a", 40);
    params[1] = criar_no_id("b", 40);

    NoAST **cmds = malloc(sizeof(NoAST *));
    cmds[0] = criar_no_return(
        criar_no_binop(OP_SOMA, criar_no_id("a", 41),
                       criar_no_id("b", 41), 41),
        41);
    NoAST *bloco = criar_no_bloco(cmds, 1);

    NoAST *funcdef = criar_no_funcdef("soma", params, 2, bloco, 40);

    ASSERT(funcdef->tipo == NO_FUNCDEF, "tipo should be NO_FUNCDEF");
    ASSERT(strcmp(funcdef->nome, "soma") == 0, "nome should be 'soma'");
    ASSERT(funcdef->num_argumentos == 2, "should have 2 params");
    ASSERT(strcmp(funcdef->argumentos[0]->nome, "a") == 0,
           "first param should be 'a'");
    ASSERT(strcmp(funcdef->argumentos[1]->nome, "b") == 0,
           "second param should be 'b'");
    ASSERT(funcdef->bloco_if == bloco, "bloco_if should point to block");

    liberar_ast(funcdef);
    printf("  PASSED\n");
    return 1;
}

static int test_criar_no_return(void) {
    printf("Running test_criar_no_return...\n");
    NoAST *expr = criar_no_int(0, 45);
    NoAST *ret = criar_no_return(expr, 45);

    ASSERT(ret->tipo == NO_RETURN, "tipo should be NO_RETURN");
    ASSERT(ret->esquerda == expr, "esquerda should point to expr");

    liberar_ast(ret);
    printf("  PASSED\n");
    return 1;
}

static int test_criar_no_return_vazio(void) {
    printf("Running test_criar_no_return_vazio...\n");
    NoAST *ret = criar_no_return(NULL, 46);

    ASSERT(ret->tipo == NO_RETURN, "tipo should be NO_RETURN");
    ASSERT(ret->esquerda == NULL, "esquerda should be NULL");

    liberar_ast(ret);
    printf("  PASSED\n");
    return 1;
}

static int test_criar_no_funccall(void) {
    printf("Running test_criar_no_funccall...\n");
    NoAST **args = malloc(2 * sizeof(NoAST *));
    args[0] = criar_no_int(1, 50);
    args[1] = criar_no_int(2, 50);
    NoAST *call = criar_no_funccall("soma", args, 2, 50);

    ASSERT(call->tipo == NO_FUNCCALL, "tipo should be NO_FUNCCALL");
    ASSERT(strcmp(call->nome, "soma") == 0, "nome should be 'soma'");
    ASSERT(call->num_argumentos == 2, "should have 2 arguments");
    ASSERT(call->argumentos[0]->valor_int == 1,
           "first arg should be 1");
    ASSERT(call->argumentos[1]->valor_int == 2,
           "second arg should be 2");

    liberar_ast(call);
    printf("  PASSED\n");
    return 1;
}

/* ═════════════════════════════════════════════════════════════════════
 * Slice 7 – Block and Program
 * ═════════════════════════════════════════════════════════════════════ */

static int test_criar_no_bloco(void) {
    printf("Running test_criar_no_bloco...\n");
    NoAST **cmds = malloc(3 * sizeof(NoAST *));
    cmds[0] = criar_no_assign("x", criar_no_int(1, 1), 1);
    cmds[1] = criar_no_assign("y", criar_no_int(2, 2), 2);
    cmds[2] = criar_no_assign("z", criar_no_int(3, 3), 3);
    NoAST *bloco = criar_no_bloco(cmds, 3);

    ASSERT(bloco->tipo == NO_BLOCO, "tipo should be NO_BLOCO");
    ASSERT(bloco->num_argumentos == 3, "should have 3 commands");
    ASSERT(bloco->argumentos[0]->tipo == NO_ASSIGN,
           "first cmd should be assign");
    ASSERT(strcmp(bloco->argumentos[2]->nome, "z") == 0,
           "third cmd should assign 'z'");

    liberar_ast(bloco);
    printf("  PASSED\n");
    return 1;
}

static int test_criar_no_programa(void) {
    printf("Running test_criar_no_programa...\n");
    NoAST **cmds = malloc(2 * sizeof(NoAST *));
    cmds[0] = criar_no_assign("x", criar_no_binop(OP_SOMA,
                  criar_no_int(10, 1), criar_no_int(5, 1), 1), 1);
    NoAST **print_args = malloc(sizeof(NoAST *));
    print_args[0] = criar_no_id("x", 2);
    cmds[1] = criar_no_print(print_args, 1, 2);

    NoAST *prog = criar_no_programa(cmds, 2);

    ASSERT(prog->tipo == NO_PROGRAMA, "tipo should be NO_PROGRAMA");
    ASSERT(prog->num_argumentos == 2, "should have 2 top-level cmds");
    ASSERT(prog->argumentos[0]->tipo == NO_ASSIGN,
           "first cmd should be assign");
    ASSERT(prog->argumentos[1]->tipo == NO_PRINT,
           "second cmd should be print");

    liberar_ast(prog);
    printf("  PASSED\n");
    return 1;
}

/* ═════════════════════════════════════════════════════════════════════
 * Slice 8 – imprimir_ast produces expected output
 * ═════════════════════════════════════════════════════════════════════ */

static int test_imprimir_ast_programa(void) {
    printf("Running test_imprimir_ast_programa...\n");

    /*
     * Programa equivalente a:
     *   x = 10 + 5
     *   print(x)
     *
     * Saída esperada:
     *   Programa
     *     Assign: x
     *       BinOp: +
     *         NumInt: 10
     *         NumInt: 5
     *     Print
     *       Id: x
     */

    NoAST **cmds = malloc(2 * sizeof(NoAST *));
    cmds[0] = criar_no_assign("x",
        criar_no_binop(OP_SOMA,
            criar_no_int(10, 1),
            criar_no_int(5, 1), 1), 1);
    NoAST **print_args = malloc(sizeof(NoAST *));
    print_args[0] = criar_no_id("x", 2);
    cmds[1] = criar_no_print(print_args, 1, 2);
    NoAST *prog = criar_no_programa(cmds, 2);

    /* Redirect stdout to a temp file */
    const char *tmppath = "testes/tdd/_ast_test_output.tmp";
    fflush(stdout);
    if (freopen(tmppath, "w", stdout) == NULL) {
        /* Can't redirect, skip test gracefully */
        fprintf(stderr, "  SKIPPED: cannot redirect stdout\n");
        liberar_ast(prog);
        return 1;
    }

    imprimir_ast(prog, 0);
    fflush(stdout);

    /* Restore stdout */
#ifdef _WIN32
    freopen("CON", "w", stdout);
#else
    freopen("/dev/tty", "w", stdout);
#endif

    /* Read captured output */
    char buffer[2048];
    FILE *f = fopen(tmppath, "r");
    if (f == NULL) {
        printf("  FAILED: cannot read temp file\n");
        liberar_ast(prog);
        return 0;
    }
    size_t n = fread(buffer, 1, sizeof(buffer) - 1, f);
    buffer[n] = '\0';
    fclose(f);
    remove(tmppath);

    /* Verificar linhas-chave da saída */
    ASSERT(strstr(buffer, "Programa") != NULL,
           "output should contain 'Programa'");
    ASSERT(strstr(buffer, "Assign: x") != NULL,
           "output should contain 'Assign: x'");
    ASSERT(strstr(buffer, "BinOp: +") != NULL,
           "output should contain 'BinOp: +'");
    ASSERT(strstr(buffer, "NumInt: 10") != NULL,
           "output should contain 'NumInt: 10'");
    ASSERT(strstr(buffer, "NumInt: 5") != NULL,
           "output should contain 'NumInt: 5'");
    ASSERT(strstr(buffer, "Print") != NULL,
           "output should contain 'Print'");
    ASSERT(strstr(buffer, "Id: x") != NULL,
           "output should contain 'Id: x'");

    liberar_ast(prog);
    printf("  PASSED\n");
    return 1;
}

/* ═════════════════════════════════════════════════════════════════════
 * Slice 9 – liberar_ast on a complex tree doesn't crash
 * ═════════════════════════════════════════════════════════════════════ */

static int test_liberar_ast_complexa(void) {
    printf("Running test_liberar_ast_complexa...\n");

    /* Build: def soma(a, b): return a + b */
    NoAST **params = malloc(2 * sizeof(NoAST *));
    params[0] = criar_no_id("a", 1);
    params[1] = criar_no_id("b", 1);

    NoAST **body = malloc(sizeof(NoAST *));
    body[0] = criar_no_return(
        criar_no_binop(OP_SOMA,
            criar_no_id("a", 2), criar_no_id("b", 2), 2), 2);
    NoAST *bloco = criar_no_bloco(body, 1);
    NoAST *funcdef = criar_no_funcdef("soma", params, 2, bloco, 1);

    /* Build: print(soma(1, 2)) */
    NoAST **call_args = malloc(2 * sizeof(NoAST *));
    call_args[0] = criar_no_int(1, 4);
    call_args[1] = criar_no_int(2, 4);
    NoAST *call = criar_no_funccall("soma", call_args, 2, 4);
    NoAST **print_args = malloc(sizeof(NoAST *));
    print_args[0] = call;
    NoAST *print_no = criar_no_print(print_args, 1, 4);

    /* programa */
    NoAST **prog_cmds = malloc(2 * sizeof(NoAST *));
    prog_cmds[0] = funcdef;
    prog_cmds[1] = print_no;
    NoAST *prog = criar_no_programa(prog_cmds, 2);

    /* This should free everything without crashing */
    liberar_ast(prog);

    printf("  PASSED\n");
    return 1;
}

static int test_liberar_ast_null(void) {
    printf("Running test_liberar_ast_null...\n");
    /* Calling liberar_ast(NULL) should not crash */
    liberar_ast(NULL);
    printf("  PASSED\n");
    return 1;
}

/* ═════════════════════════════════════════════════════════════════════
 * Slice 10 – String ownership (strdup independence)
 * ═════════════════════════════════════════════════════════════════════ */

static int test_string_ownership(void) {
    printf("Running test_string_ownership...\n");

    /* The node should own a copy of the string,
     * not the original pointer */
    char nome[] = "original";
    NoAST *no = criar_no_id(nome, 1);

    /* Mutate the source string */
    nome[0] = 'X';

    ASSERT(strcmp(no->nome, "original") == 0,
           "node should own independent copy (strdup)");

    liberar_ast(no);
    printf("  PASSED\n");
    return 1;
}

/* ═════════════════════════════════════════════════════════════════════
 * main
 * ═════════════════════════════════════════════════════════════════════ */

int main(void) {
    printf("=========================================\n");
    printf("   Testes do módulo AST\n");
    printf("=========================================\n\n");

    /* Slice 1 – Literais */
    if (test_criar_no_int())           passed++; else failed++;
    if (test_criar_no_float())         passed++; else failed++;
    if (test_criar_no_string())        passed++; else failed++;
    if (test_criar_no_bool_true())     passed++; else failed++;
    if (test_criar_no_bool_false())    passed++; else failed++;
    if (test_criar_no_id())            passed++; else failed++;

    /* Slice 2 – Operações */
    if (test_criar_no_binop())         passed++; else failed++;
    if (test_criar_no_unaryop())       passed++; else failed++;
    if (test_binop_nested())           passed++; else failed++;

    /* Slice 3 – Atribuição */
    if (test_criar_no_assign())        passed++; else failed++;

    /* Slice 4 – Print / Input */
    if (test_criar_no_print())         passed++; else failed++;
    if (test_criar_no_input())         passed++; else failed++;
    if (test_criar_no_input_sem_prompt()) passed++; else failed++;

    /* Slice 5 – Controle de fluxo */
    if (test_criar_no_if_simples())    passed++; else failed++;
    if (test_criar_no_if_com_elif_e_else()) passed++; else failed++;
    if (test_criar_no_while())         passed++; else failed++;
    if (test_criar_no_for())           passed++; else failed++;

    /* Slice 6 – Funções */
    if (test_criar_no_funcdef())       passed++; else failed++;
    if (test_criar_no_return())        passed++; else failed++;
    if (test_criar_no_return_vazio())  passed++; else failed++;
    if (test_criar_no_funccall())      passed++; else failed++;

    /* Slice 7 – Bloco / Programa */
    if (test_criar_no_bloco())         passed++; else failed++;
    if (test_criar_no_programa())      passed++; else failed++;

    /* Slice 8 – imprimir_ast */
    if (test_imprimir_ast_programa())  passed++; else failed++;

    /* Slice 9 – liberar_ast */
    if (test_liberar_ast_complexa())   passed++; else failed++;
    if (test_liberar_ast_null())       passed++; else failed++;

    /* Slice 10 – Ownership */
    if (test_string_ownership())       passed++; else failed++;

    printf("\n=========================================\n");
    printf("   Resultado: %d passaram, %d falharam\n",
           passed, failed);
    printf("=========================================\n");

    return failed == 0 ? 0 : 1;
}
