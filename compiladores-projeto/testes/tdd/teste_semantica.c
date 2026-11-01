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
    
    // Cenário 1: Variável declarada dentro de bloco vaza para fora
    TabelaSimbolos *tab1 = tabela_criar();
    NoAST **cmds_bloco1 = malloc(1 * sizeof(NoAST*));
    cmds_bloco1[0] = criar_no_assign("y", criar_no_int(20, 1), 1);
    NoAST *bloco1 = criar_no_bloco(cmds_bloco1, 1);

    NoAST *uso_y1 = criar_no_id("y", 2);

    NoAST **cmds_prog1 = malloc(2 * sizeof(NoAST*));
    cmds_prog1[0] = bloco1;
    cmds_prog1[1] = uso_y1;
    NoAST *prog1 = criar_no_programa(cmds_prog1, 2);

    int erros1 = analisar_semantica(prog1, tab1);
    ASSERT(erros1 == 0, "Variables inside blocks should remain accessible outside (Python scope rule)");
    liberar_ast(prog1);
    tabela_liberar(tab1);
    
    // Cenário 2: Variável declarada em função não vaza para fora
    TabelaSimbolos *tab2 = tabela_criar();
    
    NoAST **cmds_bloco2 = malloc(1 * sizeof(NoAST*));
    cmds_bloco2[0] = criar_no_assign("y", criar_no_int(99, 2), 2);
    NoAST *bloco2 = criar_no_funcdef("funcao", NULL, 0, criar_no_bloco(cmds_bloco2, 1), 1);
    
    NoAST *uso_y2 = criar_no_id("y", 3);
    
    NoAST **cmds_prog2 = malloc(2 * sizeof(NoAST*));
    cmds_prog2[0] = bloco2;
    cmds_prog2[1] = uso_y2;
    NoAST *prog2 = criar_no_programa(cmds_prog2, 2);
    
    int erros2 = analisar_semantica(prog2, tab2);
    ASSERT(erros2 == 1, "Should report 1 error for accessing function local variable from outer scope");
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

static int test_divisao_resulta_float(void) {
    printf("Running test_divisao_resulta_float...\n");

    TabelaSimbolos *tab = tabela_criar();
    NoAST *divisao = criar_no_binop(OP_DIV, criar_no_int(5, 1), criar_no_int(2, 1), 1);
    NoAST *multiplicacao = criar_no_binop(OP_MULT, criar_no_int(5, 2), criar_no_int(2, 2), 2);
    NoAST **comandos = malloc(2 * sizeof(NoAST *));
    comandos[0] = criar_no_assign("a", divisao, 1);
    comandos[1] = criar_no_assign("b", multiplicacao, 2);
    NoAST *programa = criar_no_programa(comandos, 2);

    int erros = analisar_semantica(programa, tab);
    Simbolo *a = tabela_buscar(tab, "a");

    ASSERT(erros == 0, "Should accept integer division");
    ASSERT(divisao->tipo_resolvido == TIPO_FLOAT, "Should infer int / int as float");
    ASSERT(a != NULL && a->tipo == TIPO_FLOAT, "Should register 'a' as float");
    ASSERT(multiplicacao->tipo_resolvido == TIPO_INT, "Should keep int * int as int");

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

static int contar_erros(NoAST *no) {
    TabelaSimbolos *tab = tabela_criar();
    int erros = analisar_semantica(no, tab);
    liberar_ast(no);
    tabela_liberar(tab);
    return erros;
}

static NoAST *bloco_vazio(void) {
    return criar_no_bloco(NULL, 0);
}

static NoAST *comparacao_nao_declarada(char *nome, int linha) {
    return criar_no_binop(OP_GT, criar_no_id(nome, linha), criar_no_int(0, linha), linha);
}

static int test_contagem_nao_declarada(void) {
    printf("Running test_contagem_nao_declarada...\n");

    // y = z + 1
    NoAST *soma = criar_no_binop(OP_SOMA, criar_no_id("z", 1), criar_no_int(1, 1), 1);
    ASSERT(contar_erros(criar_no_assign("y", soma, 1)) == 1, "Should report undeclared variable once in assignment");

    // if w > 0:
    NoAST *se = criar_no_if(comparacao_nao_declarada("w", 1), bloco_vazio(), NULL, 0, NULL, 1);
    ASSERT(contar_erros(se) == 1, "Should report undeclared variable once in if condition");

    // while w > 0:
    NoAST *enquanto = criar_no_while(comparacao_nao_declarada("w", 1), bloco_vazio(), 1);
    ASSERT(contar_erros(enquanto) == 1, "Should report undeclared variable once in while condition");

    // if True: ... elif w > 0:
    NoAST **elifs = malloc(sizeof(NoAST *));
    elifs[0] = criar_no_elif(comparacao_nao_declarada("w", 2), bloco_vazio(), 2);
    NoAST *se_elif = criar_no_if(criar_no_bool(1, 1), bloco_vazio(), elifs, 1, NULL, 1);
    ASSERT(contar_erros(se_elif) == 1, "Should report undeclared variable once in elif condition");

    // def f(): return k
    NoAST **corpo = malloc(sizeof(NoAST *));
    corpo[0] = criar_no_return(criar_no_id("k", 2), 2);
    NoAST *funcao = criar_no_funcdef("f", NULL, 0, criar_no_bloco(corpo, 1), 1);
    ASSERT(contar_erros(funcao) == 1, "Should report undeclared variable once in return");

    printf("  PASSED\n");
    return 1;
}

static int test_contagem_argumentos_de_chamada(void) {
    printf("Running test_contagem_argumentos_de_chamada...\n");

    // y = g(z), com g não declarada: erro na chamada e no argumento
    NoAST **args = malloc(sizeof(NoAST *));
    args[0] = criar_no_id("z", 1);
    NoAST *chamada = criar_no_funccall("g", args, 1, 1);
    ASSERT(contar_erros(criar_no_assign("y", chamada, 1)) == 2, "Should report undeclared function and its undeclared argument");

    // x = 1; y = x(z): erro na chamada e no argumento
    NoAST **args2 = malloc(sizeof(NoAST *));
    args2[0] = criar_no_id("z", 2);
    NoAST **cmds2 = malloc(2 * sizeof(NoAST *));
    cmds2[0] = criar_no_assign("x", criar_no_int(1, 1), 1);
    cmds2[1] = criar_no_assign("y", criar_no_funccall("x", args2, 1, 2), 2);
    ASSERT(contar_erros(criar_no_programa(cmds2, 2)) == 2, "Should report non-function call and its undeclared argument");

    // def f(): ...; y = f(z): erro de aridade e no argumento
    NoAST **args3 = malloc(sizeof(NoAST *));
    args3[0] = criar_no_id("z", 2);
    NoAST **cmds3 = malloc(2 * sizeof(NoAST *));
    cmds3[0] = criar_no_funcdef("f", NULL, 0, bloco_vazio(), 1);
    cmds3[1] = criar_no_assign("y", criar_no_funccall("f", args3, 1, 2), 2);
    ASSERT(contar_erros(criar_no_programa(cmds3, 2)) == 2, "Should report wrong arity and its undeclared argument");

    // def f(a): ...; y = f(z): somente o argumento
    NoAST **params4 = malloc(sizeof(NoAST *));
    params4[0] = criar_no_id("a", 1);
    NoAST **args4 = malloc(sizeof(NoAST *));
    args4[0] = criar_no_id("z", 2);
    NoAST **cmds4 = malloc(2 * sizeof(NoAST *));
    cmds4[0] = criar_no_funcdef("f", params4, 1, bloco_vazio(), 1);
    cmds4[1] = criar_no_assign("y", criar_no_funccall("f", args4, 1, 2), 2);
    ASSERT(contar_erros(criar_no_programa(cmds4, 2)) == 1, "Should report undeclared argument once in a valid call");

    printf("  PASSED\n");
    return 1;
}

static int test_redefinicao_funcao_e_conflito(void) {
    printf("Running test_redefinicao_funcao_e_conflito...\n");

    // Caso 1: Redefinição de função no mesmo escopo (def f(): return 1; def f(a): return 2)
    NoAST **corpo1 = malloc(sizeof(NoAST *));
    corpo1[0] = criar_no_return(criar_no_int(1, 2), 2);
    NoAST *f1 = criar_no_funcdef("f", NULL, 0, criar_no_bloco(corpo1, 1), 1);

    NoAST **params2 = malloc(sizeof(NoAST *));
    params2[0] = criar_no_id("a", 3);
    NoAST **corpo2 = malloc(sizeof(NoAST *));
    corpo2[0] = criar_no_return(criar_no_int(2, 4), 4);
    NoAST *f2 = criar_no_funcdef("f", params2, 1, criar_no_bloco(corpo2, 1), 3);

    NoAST **prog1_cmds = malloc(2 * sizeof(NoAST *));
    prog1_cmds[0] = f1;
    prog1_cmds[1] = f2;
    NoAST *prog1 = criar_no_programa(prog1_cmds, 2);
    ASSERT(contar_erros(prog1) == 1, "Should report 1 error for function redefinition in same scope");

    // Caso 2: Atribuição a nome de função já declarada (def g(): return 1; g = 10)
    NoAST **corpo_g = malloc(sizeof(NoAST *));
    corpo_g[0] = criar_no_return(criar_no_int(1, 2), 2);
    NoAST *def_g = criar_no_funcdef("g", NULL, 0, criar_no_bloco(corpo_g, 1), 1);
    NoAST *assign_g = criar_no_assign("g", criar_no_int(10, 3), 3);

    NoAST **prog2_cmds = malloc(2 * sizeof(NoAST *));
    prog2_cmds[0] = def_g;
    prog2_cmds[1] = assign_g;
    NoAST *prog2 = criar_no_programa(prog2_cmds, 2);
    ASSERT(contar_erros(prog2) == 1, "Should report 1 error for assigning to an existing function name");

    // Caso 3: Sombreamento válido em escopo filho (def h(): x = 1; def h_filho(): h = 2)
    NoAST **corpo_h_filho = malloc(sizeof(NoAST *));
    corpo_h_filho[0] = criar_no_assign("h", criar_no_int(2, 3), 3);
    NoAST *def_h_filho = criar_no_funcdef("h_filho", NULL, 0, criar_no_bloco(corpo_h_filho, 1), 2);

    NoAST **corpo_h = malloc(sizeof(NoAST *));
    corpo_h[0] = def_h_filho;
    NoAST *def_h = criar_no_funcdef("h", NULL, 0, criar_no_bloco(corpo_h, 1), 1);

    NoAST **prog3_cmds = malloc(sizeof(NoAST *));
    prog3_cmds[0] = def_h;
    NoAST *prog3 = criar_no_programa(prog3_cmds, 1);
    ASSERT(contar_erros(prog3) == 0, "Should allow shadowing function name in inner scope");

    // Caso 4: Variável declarada antes de função com o mesmo nome (k = 5; def k(): return 1)
    NoAST *assign_k = criar_no_assign("k", criar_no_int(5, 1), 1);
    NoAST **corpo_k = malloc(sizeof(NoAST *));
    corpo_k[0] = criar_no_return(criar_no_int(1, 3), 3);
    NoAST *def_k = criar_no_funcdef("k", NULL, 0, criar_no_bloco(corpo_k, 1), 2);

    NoAST **prog4_cmds = malloc(2 * sizeof(NoAST *));
    prog4_cmds[0] = assign_k;
    prog4_cmds[1] = def_k;
    NoAST *prog4 = criar_no_programa(prog4_cmds, 2);
    ASSERT(contar_erros(prog4) == 1, "Should report 1 error when function reuses an existing variable name");

    // Caso 5: Função redefinida com erro semântico dentro do segundo corpo (deve reportar ambos: 2 erros)
    NoAST **corpo_m1 = malloc(sizeof(NoAST *));
    corpo_m1[0] = criar_no_return(criar_no_int(1, 2), 2);
    NoAST *m1 = criar_no_funcdef("m", NULL, 0, criar_no_bloco(corpo_m1, 1), 1);

    NoAST **corpo_m2 = malloc(sizeof(NoAST *));
    corpo_m2[0] = criar_no_return(criar_no_id("var_inexistente", 4), 4);
    NoAST *m2 = criar_no_funcdef("m", NULL, 0, criar_no_bloco(corpo_m2, 1), 3);

    NoAST **prog5_cmds = malloc(2 * sizeof(NoAST *));
    prog5_cmds[0] = m1;
    prog5_cmds[1] = m2;
    NoAST *prog5 = criar_no_programa(prog5_cmds, 2);
    ASSERT(contar_erros(prog5) == 2, "Should report redefinition error AND analyze body of redefined function");

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
        test_divisao_resulta_float(),
        test_funcoes_e_contexto(),
        test_contagem_nao_declarada(),
        test_contagem_argumentos_de_chamada(),
        test_redefinicao_funcao_e_conflito()
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
