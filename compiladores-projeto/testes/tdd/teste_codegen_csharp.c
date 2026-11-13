/* ═══════════════════════════════════════════════════════════════════
 * teste_codegen_csharp.c — Testes unitários do gerador de código C#
 *
 * Seam: API pública de codegen_csharp.h
 *       (codegen_csharp_gerar, codegen_csharp_expressao, codegen_csharp_tipo)
 *
 * As ASTs passam pela análise semântica antes da geração, como no
 * compilador real, para que tipo_resolvido venha da inferência.
 * ═══════════════════════════════════════════════════════════════════ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../src/ast.h"
#include "../../src/tabela.h"
#include "../../src/semantica.h"
#include "../../src/codegen_csharp.h"

static int passed = 0;
static int failed = 0;

#define ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            printf("  FAILED: %s\n", msg); \
            return 0; \
        } \
    } while(0)

/* ── Helpers ─────────────────────────────────────────────────────── */

static NoAST *programa(int n, NoAST **cmds_estaticos) {
    NoAST **cmds = malloc(sizeof(NoAST *) * n);
    for (int i = 0; i < n; i++) cmds[i] = cmds_estaticos[i];
    return criar_no_programa(cmds, n);
}

/* Roda semântica + geração e devolve o arquivo C# inteiro (liberar com free). */
static char *gerar_csharp(NoAST *raiz, int *erros_semanticos, int *nao_traduzidos) {
    TabelaSimbolos *tab = tabela_criar();
    *erros_semanticos = analisar_semantica(raiz, tab);

    FILE *tmp = tmpfile();
    *nao_traduzidos = codegen_csharp_gerar(raiz, tmp);

    long tamanho = ftell(tmp);
    rewind(tmp);
    char *texto = malloc(tamanho + 1);
    size_t lidos = fread(texto, 1, tamanho, tmp);
    texto[lidos] = '\0';
    fclose(tmp);

    tabela_liberar(tab);
    return texto;
}

/*
 * Traduz 'expr' no contexto: a = 7, b = 2 (int), f = 1.5 (float),
 * p = True, q = False (bool), s = "oi" (string).
 */
static char *traduzir_expr(NoAST *expr) {
    NoAST *cmds[] = {
        criar_no_assign("a", criar_no_int(7, 1), 1),
        criar_no_assign("b", criar_no_int(2, 2), 2),
        criar_no_assign("f", criar_no_float(1.5, 3), 3),
        criar_no_assign("p", criar_no_bool(1, 4), 4),
        criar_no_assign("q", criar_no_bool(0, 5), 5),
        criar_no_assign("s", criar_no_string("\"oi\"", 6), 6),
        criar_no_assign("x", expr, 7),
    };
    NoAST *raiz = programa(7, cmds);

    TabelaSimbolos *tab = tabela_criar();
    analisar_semantica(raiz, tab);
    char *texto = codegen_csharp_expressao(expr);

    tabela_liberar(tab);
    liberar_ast(raiz);
    return texto;
}

static int confere_expr(NoAST *expr, const char *esperado) {
    char *obtido = traduzir_expr(expr);
    int ok = strcmp(obtido, esperado) == 0;
    if (!ok) printf("  esperado: %s\n  obtido:   %s\n", esperado, obtido);
    free(obtido);
    return ok;
}

/* ── Testes ──────────────────────────────────────────────────────── */

static int test_mapeamento_tipos(void) {
    printf("Running test_mapeamento_tipos...\n");

    ASSERT(strcmp(codegen_csharp_tipo(TIPO_INT), "int") == 0, "int -> int");
    ASSERT(strcmp(codegen_csharp_tipo(TIPO_FLOAT), "double") == 0, "float -> double");
    ASSERT(strcmp(codegen_csharp_tipo(TIPO_STRING), "string") == 0, "str -> string");
    ASSERT(strcmp(codegen_csharp_tipo(TIPO_BOOL), "bool") == 0, "bool -> bool");
    ASSERT(strcmp(codegen_csharp_tipo(TIPO_DESCONHECIDO), "var") == 0, "desconhecido -> var");

    printf("  PASSED\n");
    return 1;
}

static int test_literais(void) {
    printf("Running test_literais...\n");

    ASSERT(confere_expr(criar_no_int(10, 7), "10"), "int literal");
    ASSERT(confere_expr(criar_no_float(3.14, 7), "3.14"), "float com representação mínima");
    ASSERT(confere_expr(criar_no_float(2.0, 7), "2.0"), "float inteiro mantém '.0'");
    ASSERT(confere_expr(criar_no_bool(1, 7), "true"), "True -> true");
    ASSERT(confere_expr(criar_no_bool(0, 7), "false"), "False -> false");
    ASSERT(confere_expr(criar_no_string("\"oi\"", 7), "\"oi\""), "string com aspas duplas");
    ASSERT(confere_expr(criar_no_string("'oi'", 7), "\"oi\""), "aspas simples -> duplas");
    ASSERT(confere_expr(criar_no_string("'diz \"oi\"'", 7), "\"diz \\\"oi\\\"\""),
           "aspa dupla dentro de string com aspas simples é escapada");
    ASSERT(confere_expr(criar_no_string("'a\\nb'", 7), "\"a\\nb\""), "escape \\n preservado");
    ASSERT(confere_expr(criar_no_string("'\\x41'", 7), "\"\\u0041\""), "\\xhh -> \\u00hh");

    printf("  PASSED\n");
    return 1;
}

static int test_operadores_aritmeticos(void) {
    printf("Running test_operadores_aritmeticos...\n");

    ASSERT(confere_expr(criar_no_binop(OP_SOMA, criar_no_id("a", 7), criar_no_id("b", 7), 7),
                        "a + b"), "a + b");
    ASSERT(confere_expr(criar_no_binop(OP_SUB, criar_no_id("a", 7), criar_no_id("b", 7), 7),
                        "a - b"), "a - b");
    ASSERT(confere_expr(criar_no_binop(OP_MULT, criar_no_id("a", 7), criar_no_id("b", 7), 7),
                        "a * b"), "a * b");
    ASSERT(confere_expr(criar_no_binop(OP_SOMA, criar_no_id("s", 7), criar_no_string("'!'", 7), 7),
                        "s + \"!\""), "concatenação de strings");

    printf("  PASSED\n");
    return 1;
}

static int test_divisao_python(void) {
    printf("Running test_divisao_python...\n");

    /* int / int em Python dá float: precisa do cast para não truncar em C#. */
    ASSERT(confere_expr(criar_no_binop(OP_DIV, criar_no_id("a", 7), criar_no_id("b", 7), 7),
                        "(double)a / b"), "int / int recebe cast");
    ASSERT(confere_expr(criar_no_binop(OP_DIV, criar_no_id("f", 7), criar_no_id("b", 7), 7),
                        "f / b"), "float / int não precisa de cast");
    ASSERT(confere_expr(criar_no_binop(OP_DIV,
                            criar_no_binop(OP_SOMA, criar_no_id("a", 7), criar_no_id("b", 7), 7),
                            criar_no_int(2, 7), 7),
                        "(double)(a + b) / 2"), "cast sobre subexpressão entre parênteses");

    printf("  PASSED\n");
    return 1;
}

static int test_operadores_logicos(void) {
    printf("Running test_operadores_logicos...\n");

    ASSERT(confere_expr(criar_no_binop(OP_AND, criar_no_id("p", 7), criar_no_id("q", 7), 7),
                        "p && q"), "and -> &&");
    ASSERT(confere_expr(criar_no_binop(OP_OR, criar_no_id("p", 7), criar_no_id("q", 7), 7),
                        "p || q"), "or -> ||");
    ASSERT(confere_expr(criar_no_unaryop(OP_NOT, criar_no_id("p", 7), 7), "!p"), "not -> !");
    ASSERT(confere_expr(criar_no_binop(OP_GTE, criar_no_id("a", 7), criar_no_id("b", 7), 7),
                        "a >= b"), "comparação");

    printf("  PASSED\n");
    return 1;
}

static int test_comparacoes_string_e_bool(void) {
    printf("Running test_comparacoes_string_e_bool...\n");

    /* C# não tem <, >, <=, >= para string nem bool (CS0019). */
    ASSERT(confere_expr(criar_no_binop(OP_LT, criar_no_id("s", 7), criar_no_string("'abd'", 7), 7),
                        "string.CompareOrdinal(s, \"abd\") < 0"), "string < string");
    ASSERT(confere_expr(criar_no_binop(OP_GTE, criar_no_id("s", 7), criar_no_id("s", 7), 7),
                        "string.CompareOrdinal(s, s) >= 0"), "string >= string");
    ASSERT(confere_expr(criar_no_binop(OP_GT, criar_no_id("p", 7), criar_no_id("q", 7), 7),
                        "(p ? 1 : 0) > (q ? 1 : 0)"), "bool > bool (True = 1, False = 0)");
    /* == e != já funcionam em C# para string e bool. */
    ASSERT(confere_expr(criar_no_binop(OP_EQ, criar_no_id("s", 7), criar_no_string("'oi'", 7), 7),
                        "s == \"oi\""), "string == string sem conversão");
    ASSERT(confere_expr(criar_no_binop(OP_NEQ, criar_no_id("p", 7), criar_no_id("q", 7), 7),
                        "p != q"), "bool != bool sem conversão");
    /* Dentro de expressão lógica, sem parênteses extras. */
    ASSERT(confere_expr(criar_no_binop(OP_AND,
                            criar_no_binop(OP_LT, criar_no_id("s", 7), criar_no_string("'z'", 7), 7),
                            criar_no_id("p", 7), 7),
                        "string.CompareOrdinal(s, \"z\") < 0 && p"), "comparação de string dentro de and");

    printf("  PASSED\n");
    return 1;
}

static int test_precedencia_e_parenteses(void) {
    printf("Running test_precedencia_e_parenteses...\n");

    /* (a + b) * 2 : parênteses obrigatórios */
    ASSERT(confere_expr(criar_no_binop(OP_MULT,
                            criar_no_binop(OP_SOMA, criar_no_id("a", 7), criar_no_id("b", 7), 7),
                            criar_no_int(2, 7), 7),
                        "(a + b) * 2"), "soma dentro de multiplicação");

    /* a + b * 2 : sem parênteses desnecessários */
    ASSERT(confere_expr(criar_no_binop(OP_SOMA, criar_no_id("a", 7),
                            criar_no_binop(OP_MULT, criar_no_id("b", 7), criar_no_int(2, 7), 7), 7),
                        "a + b * 2"), "multiplicação dentro de soma");

    /* a - (b - 1) : associatividade à esquerda */
    ASSERT(confere_expr(criar_no_binop(OP_SUB, criar_no_id("a", 7),
                            criar_no_binop(OP_SUB, criar_no_id("b", 7), criar_no_int(1, 7), 7), 7),
                        "a - (b - 1)"), "subtração à direita mantém parênteses");

    /* not a == b (Python) == not (a == b); em C# '!' liga mais forte */
    ASSERT(confere_expr(criar_no_unaryop(OP_NOT,
                            criar_no_binop(OP_EQ, criar_no_id("a", 7), criar_no_id("b", 7), 7), 7),
                        "!(a == b)"), "not sobre comparação");

    /* p or q and p : && tem precedência maior, sem parênteses */
    ASSERT(confere_expr(criar_no_binop(OP_OR, criar_no_id("p", 7),
                            criar_no_binop(OP_AND, criar_no_id("q", 7), criar_no_id("p", 7), 7), 7),
                        "p || q && p"), "and dentro de or");

    /* (p or q) and p */
    ASSERT(confere_expr(criar_no_binop(OP_AND,
                            criar_no_binop(OP_OR, criar_no_id("p", 7), criar_no_id("q", 7), 7),
                            criar_no_id("p", 7), 7),
                        "(p || q) && p"), "or dentro de and");

    printf("  PASSED\n");
    return 1;
}

static int test_negacao(void) {
    printf("Running test_negacao...\n");

    ASSERT(confere_expr(criar_no_unaryop(OP_NEG, criar_no_id("a", 7), 7), "-a"), "-a");
    /* '- -a' não pode virar '--a' (decremento em C#) */
    ASSERT(confere_expr(criar_no_unaryop(OP_NEG,
                            criar_no_unaryop(OP_NEG, criar_no_id("a", 7), 7), 7),
                        "-(-a)"), "negação dupla");
    ASSERT(confere_expr(criar_no_unaryop(OP_NEG,
                            criar_no_binop(OP_SOMA, criar_no_id("a", 7), criar_no_id("b", 7), 7), 7),
                        "-(a + b)"), "negação de soma");

    printf("  PASSED\n");
    return 1;
}

static int test_palavra_reservada(void) {
    printf("Running test_palavra_reservada...\n");

    NoAST *id = criar_no_id("string", 1);
    char *texto = codegen_csharp_expressao(id);
    ASSERT(strcmp(texto, "@string") == 0, "identificador 'string' vira '@string'");
    free(texto);
    liberar_ast(id);

    id = criar_no_id("contador", 1);
    texto = codegen_csharp_expressao(id);
    ASSERT(strcmp(texto, "contador") == 0, "identificador comum fica igual");
    free(texto);
    liberar_ast(id);

    printf("  PASSED\n");
    return 1;
}

static int test_estrutura_programa_vazio(void) {
    printf("Running test_estrutura_programa_vazio...\n");

    FILE *tmp = tmpfile();
    int nao_traduzidos = codegen_csharp_gerar(NULL, tmp);
    long tamanho = ftell(tmp);
    rewind(tmp);
    char texto[512];
    size_t lidos = fread(texto, 1, sizeof(texto) - 1, tmp);
    texto[lidos] = '\0';
    fclose(tmp);
    (void)tamanho;

    const char *esperado =
        "using System;\n"
        "\n"
        "namespace CompiladorPython {\n"
        "    static class Program {\n"
        "        static void Main(string[] args) {\n"
        "        }\n"
        "    }\n"
        "}\n";

    ASSERT(nao_traduzidos == 0, "programa vazio não tem construções pendentes");
    ASSERT(strcmp(texto, esperado) == 0, "estrutura base: using, namespace, static class Program, Main");

    printf("  PASSED\n");
    return 1;
}

static int test_declaracao_e_reatribuicao(void) {
    printf("Running test_declaracao_e_reatribuicao...\n");

    NoAST *cmds[] = {
        criar_no_assign("x", criar_no_int(10, 1), 1),
        criar_no_assign("y", criar_no_float(3.14, 2), 2),
        criar_no_assign("s", criar_no_string("'oi'", 3), 3),
        criar_no_assign("b", criar_no_bool(1, 4), 4),
        criar_no_assign("x", criar_no_int(20, 5), 5),
        criar_no_assign("m", criar_no_binop(OP_DIV, criar_no_id("x", 6), criar_no_int(4, 6), 6), 6),
    };
    NoAST *raiz = programa(6, cmds);

    int erros = 0, nao_traduzidos = 0;
    char *cs = gerar_csharp(raiz, &erros, &nao_traduzidos);

    ASSERT(erros == 0, "programa semanticamente válido");
    ASSERT(nao_traduzidos == 0, "tudo traduzido");
    ASSERT(strstr(cs, "            int x = 10;\n") != NULL, "x = 10 -> int x = 10;");
    ASSERT(strstr(cs, "            double y = 3.14;\n") != NULL, "y = 3.14 -> double y = 3.14;");
    ASSERT(strstr(cs, "            string s = \"oi\";\n") != NULL, "s = 'oi' -> string s = \"oi\";");
    ASSERT(strstr(cs, "            bool b = true;\n") != NULL, "b = True -> bool b = true;");
    ASSERT(strstr(cs, "            x = 20;\n") != NULL, "reatribuição sem tipo");
    ASSERT(strstr(cs, "int x = 20") == NULL, "não redeclara x");
    ASSERT(strstr(cs, "            double m = (double)x / 4;\n") != NULL, "divisão int/int gera double");

    free(cs);
    liberar_ast(raiz);

    printf("  PASSED\n");
    return 1;
}

static int test_construcoes_pendentes(void) {
    printf("Running test_construcoes_pendentes...\n");

    /* def f(): return 1, if x > 0: x = 2 e y = f(): ainda fora do escopo do gerador */
    NoAST **corpo = malloc(sizeof(NoAST *));
    corpo[0] = criar_no_return(criar_no_int(1, 2), 2);
    NoAST **corpo_if = malloc(sizeof(NoAST *));
    corpo_if[0] = criar_no_assign("x", criar_no_int(2, 4), 4);

    NoAST *cmds[] = {
        criar_no_funcdef("f", NULL, 0, criar_no_bloco(corpo, 1), 1),
        criar_no_assign("x", criar_no_int(1, 3), 3),
        criar_no_if(criar_no_binop(OP_GT, criar_no_id("x", 4), criar_no_int(0, 4), 4),
                    criar_no_bloco(corpo_if, 1), NULL, 0, NULL, 4),
        criar_no_assign("y", criar_no_funccall("f", NULL, 0, 5), 5),
    };
    NoAST *raiz = programa(4, cmds);

    int erros = 0, nao_traduzidos = 0;
    char *cs = gerar_csharp(raiz, &erros, &nao_traduzidos);

    ASSERT(erros == 0, "programa semanticamente válido");
    ASSERT(nao_traduzidos == 3, "def, if e chamada contados como não traduzidos");
    ASSERT(strstr(cs, "        // [não traduzido] def (linha 1)\n") != NULL,
           "def marcado no corpo da classe (fora do Main)");
    ASSERT(strstr(cs, "            // [não traduzido] if (linha 4)\n") != NULL,
           "if marcado dentro do Main");
    ASSERT(strstr(cs, "def") < strstr(cs, "static void Main"), "funções vêm antes do Main");
    /* Sem o def emitido, a chamada não pode referenciar f(): placeholder tipado. */
    ASSERT(strstr(cs, "            int y = /* não traduzido: chamada de função (linha 5) */ default(int);\n") != NULL,
           "chamada de função vira placeholder com o tipo de retorno inferido");

    free(cs);
    liberar_ast(raiz);

    printf("  PASSED\n");
    return 1;
}

int main(void) {
    printf("=======================================\n");
    printf(" TESTES DO GERADOR DE CÓDIGO C#\n");
    printf("=======================================\n");

    int test_results[] = {
        test_mapeamento_tipos(),
        test_literais(),
        test_operadores_aritmeticos(),
        test_divisao_python(),
        test_operadores_logicos(),
        test_comparacoes_string_e_bool(),
        test_precedencia_e_parenteses(),
        test_negacao(),
        test_palavra_reservada(),
        test_estrutura_programa_vazio(),
        test_declaracao_e_reatribuicao(),
        test_construcoes_pendentes()
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
