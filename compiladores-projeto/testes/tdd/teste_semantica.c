/* ═══════════════════════════════════════════════════════════════════
 * teste_semantica.c — Testes unitários para o módulo de Semântica
 *
 * Seam: API pública de semantica.h (analisar_semantica)
 * ═══════════════════════════════════════════════════════════════════ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/ast.h"
#include "../../src/tabela.h"
#include "../../src/semantica.h"

static int passed = 0;
static int failed = 0;

#define ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            printf("  FAILED: %s\n", msg); \
            return 0; \
        } \
    } while(0)

static int test_infraestrutura(void) {
    printf("Running test_infraestrutura...\n");
    
    TabelaSimbolos *tab = tabela_criar();
    NoAST *no = criar_no_int(42, 1);
    
    int erros = analisar_semantica(no, tab);
    
    ASSERT(erros == 0, "analisar_semantica should return 0 errors for a basic literal");
    
    liberar_ast(no);
    tabela_liberar(tab);
    
    printf("  PASSED\n");
    return 1;
}

static int test_declaracao_vs_uso(void) {
    printf("Running test_declaracao_vs_uso...\n");
    
    // Caso 1: Uso sem declaração
    TabelaSimbolos *tab1 = tabela_criar();
    NoAST *uso_y = criar_no_id("y", 1);
    int erros1 = analisar_semantica(uso_y, tab1);
    ASSERT(erros1 == 1, "Should report 1 error for undeclared variable 'y'");
    liberar_ast(uso_y);
    tabela_liberar(tab1);
    
    // Caso 2: Declaração seguida de uso
    TabelaSimbolos *tab2 = tabela_criar();
    NoAST *decl_x = criar_no_assign("x", criar_no_int(10, 1), 1);
    NoAST *uso_x = criar_no_id("x", 2);
    NoAST **cmds = malloc(2 * sizeof(NoAST*));
    cmds[0] = decl_x;
    cmds[1] = uso_x;
    NoAST *prog = criar_no_programa(cmds, 2);
    
    int erros2 = analisar_semantica(prog, tab2);
    ASSERT(erros2 == 0, "Should report 0 errors when 'x' is declared before use");
    liberar_ast(prog);
    tabela_liberar(tab2);
    
    printf("  PASSED\n");
    return 1;
}

static int test_escopos_aninhados(void) {
    printf("Running test_escopos_aninhados...\n");
    
    // Cenário 1: Acesso a variável do escopo pai
    TabelaSimbolos *tab1 = tabela_criar();
    NoAST *decl_x = criar_no_assign("x", criar_no_int(10, 1), 1);
    
    NoAST **cmds_bloco = malloc(1 * sizeof(NoAST*));
    cmds_bloco[0] = criar_no_id("x", 2);
    NoAST *bloco = criar_no_bloco(cmds_bloco, 1);
    
    NoAST **cmds_prog = malloc(2 * sizeof(NoAST*));
    cmds_prog[0] = decl_x;
    cmds_prog[1] = bloco;
    NoAST *prog1 = criar_no_programa(cmds_prog, 2);
    
    int erros1 = analisar_semantica(prog1, tab1);
    ASSERT(erros1 == 0, "Should access variable 'x' from outer scope");
    liberar_ast(prog1);
    tabela_liberar(tab1);
    
    // Cenário 2: Variável de escopo interno não vaza para fora
    TabelaSimbolos *tab2 = tabela_criar();
    
    NoAST **cmds_bloco2 = malloc(1 * sizeof(NoAST*));
    cmds_bloco2[0] = criar_no_assign("y", criar_no_int(20, 1), 1);
    NoAST *bloco2 = criar_no_bloco(cmds_bloco2, 1);
    
    NoAST *uso_y = criar_no_id("y", 2);
    
    NoAST **cmds_prog2 = malloc(2 * sizeof(NoAST*));
    cmds_prog2[0] = bloco2;
    cmds_prog2[1] = uso_y;
    NoAST *prog2 = criar_no_programa(cmds_prog2, 2);
    
    int erros2 = analisar_semantica(prog2, tab2);
    ASSERT(erros2 == 1, "Should report 1 error for accessing inner variable 'y' from outer scope");
    liberar_ast(prog2);
    tabela_liberar(tab2);

    printf("  PASSED\n");
    return 1;
}

static int test_inferencia_e_operacoes(void) {
    printf("Running test_inferencia_e_operacoes...\n");

    TabelaSimbolos *tab = tabela_criar();
    NoAST *soma_float = criar_no_binop(OP_SOMA, criar_no_int(1, 1), criar_no_float(2.5, 1), 1);
    NoAST *soma_string = criar_no_binop(OP_SOMA, criar_no_string("a", 2), criar_no_string("b", 2), 2);
    NoAST *soma_invalida = criar_no_binop(OP_SOMA, criar_no_string("a", 3), criar_no_int(1, 3), 3);
    NoAST **comandos = malloc(3 * sizeof(NoAST *));
    comandos[0] = criar_no_assign("float_resultado", soma_float, 1);
    comandos[1] = criar_no_assign("string_resultado", soma_string, 2);
    comandos[2] = criar_no_assign("resultado_invalido", soma_invalida, 3);
    NoAST *programa = criar_no_programa(comandos, 3);

    int erros = analisar_semantica(programa, tab);

    ASSERT(erros == 1, "Should reject only string plus integer");
    ASSERT(soma_float->tipo_resolvido == TIPO_FLOAT, "Should promote int plus float to float");
    ASSERT(soma_string->tipo_resolvido == TIPO_STRING, "Should accept string concatenation");

    liberar_ast(programa);
    tabela_liberar(tab);

    printf("  PASSED\n");
    return 1;
}

static int test_funcoes_e_contexto(void) {
    printf("Running test_funcoes_e_contexto...\n");

    NoAST **params = malloc(sizeof(NoAST *));
    params[0] = criar_no_id("valor", 1);
    NoAST **corpo_cmds = malloc(sizeof(NoAST *));
    corpo_cmds[0] = criar_no_return(criar_no_id("valor", 2), 2);
    NoAST *funcao = criar_no_funcdef("dobrar", params, 1, criar_no_bloco(corpo_cmds, 1), 1);

    NoAST **argumentos = malloc(sizeof(NoAST *));
    argumentos[0] = criar_no_int(10, 4);
    NoAST *chamada_correta = criar_no_funccall("dobrar", argumentos, 1, 4);
    NoAST *resultado = criar_no_assign("resultado", chamada_correta, 4);
    NoAST **programa_cmds = malloc(2 * sizeof(NoAST *));
    programa_cmds[0] = funcao;
    programa_cmds[1] = resultado;

    TabelaSimbolos *tab1 = tabela_criar();
    NoAST *programa = criar_no_programa(programa_cmds, 2);
    int erros1 = analisar_semantica(programa, tab1);
    ASSERT(erros1 == 0, "Should accept a function call with the correct number of arguments");
    liberar_ast(programa);
    tabela_liberar(tab1);

    NoAST **parametros2 = malloc(sizeof(NoAST *));
    parametros2[0] = criar_no_id("valor", 1);
    NoAST *funcao2 = criar_no_funcdef("dobrar", parametros2, 1, criar_no_bloco(NULL, 0), 1);
    NoAST *chamada_errada = criar_no_funccall("dobrar", NULL, 0, 3);
    NoAST *resultado_errado = criar_no_assign("resultado", chamada_errada, 3);
    NoAST **programa_cmds2 = malloc(2 * sizeof(NoAST *));
    programa_cmds2[0] = funcao2;
    programa_cmds2[1] = resultado_errado;

    TabelaSimbolos *tab2 = tabela_criar();
    NoAST *programa2 = criar_no_programa(programa_cmds2, 2);
    int erros2 = analisar_semantica(programa2, tab2);
    ASSERT(erros2 == 1, "Should reject a function call with the wrong number of arguments");
    liberar_ast(programa2);
    tabela_liberar(tab2);

    TabelaSimbolos *tab3 = tabela_criar();
    NoAST *retorno_global = criar_no_return(criar_no_int(1, 1), 1);
    int erros3 = analisar_semantica(retorno_global, tab3);
    ASSERT(erros3 == 1, "Should reject return outside a function");
    liberar_ast(retorno_global);
    tabela_liberar(tab3);

    printf("  PASSED\n");
    return 1;
}

int main(void) {
    printf("=======================================\n");
    printf(" TESTES DA ANÁLISE SEMÂNTICA\n");
    printf("=======================================\n");

    int test_results[] = {
        test_infraestrutura(),
        test_declaracao_vs_uso(),
        test_escopos_aninhados(),
        test_inferencia_e_operacoes(),
        test_funcoes_e_contexto()
    };

    int n_tests = sizeof(test_results) / sizeof(test_results[0]);
    for (int i = 0; i < n_tests; i++) {
        if (test_results[i]) passed++;
        else failed++;
    }

    printf("=======================================\n");
    printf(" RESULTS: %d passed, %d failed\n", passed, failed);
    printf("=======================================\n");

    return failed == 0 ? 0 : 1;
}
