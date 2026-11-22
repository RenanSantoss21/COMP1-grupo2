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

    /* def f(): return 1 e y = f(): ainda fora do escopo do gerador; if já traduzido */
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
    ASSERT(nao_traduzidos == 2, "def e chamada contados como não traduzidos");
    ASSERT(strstr(cs, "        // [não traduzido] def (linha 1)\n") != NULL,
           "def marcado no corpo da classe (fora do Main)");
    ASSERT(strstr(cs, "            if (x > 0) {\n") != NULL,
           "if traduzido dentro do Main");
    ASSERT(strstr(cs, "// [não traduzido] if") == NULL,
           "if não aparece mais como não traduzido");
    ASSERT(strstr(cs, "def") < strstr(cs, "static void Main"), "funções vêm antes do Main");
    /* Sem o def emitido, a chamada não pode referenciar f(): placeholder tipado. */
    ASSERT(strstr(cs, "            int y = /* não traduzido: chamada de função (linha 5) */ default(int);\n") != NULL,
           "chamada de função vira placeholder com o tipo de retorno inferido");

    free(cs);
    liberar_ast(raiz);

    printf("  PASSED\n");
    return 1;
}

static int test_print(void) {
    printf("Running test_print...\n");

    /* print("hello, world"), x = 10, print("x =", x), print(), print(x / 2), print("a" + 1) */
    NoAST **arg_um = malloc(sizeof(NoAST *));
    arg_um[0] = criar_no_string("\"hello, world\"", 1);
    NoAST **arg_varios = malloc(sizeof(NoAST *) * 2);
    arg_varios[0] = criar_no_string("\"x =\"", 3);
    arg_varios[1] = criar_no_id("x", 3);
    NoAST **arg_div = malloc(sizeof(NoAST *));
    arg_div[0] = criar_no_binop(OP_DIV, criar_no_id("x", 5), criar_no_int(2, 5), 5);

    NoAST *cmds[] = {
        criar_no_print(arg_um, 1, 1),
        criar_no_assign("x", criar_no_int(10, 2), 2),
        criar_no_print(arg_varios, 2, 3),
        criar_no_print(NULL, 0, 4),
        criar_no_print(arg_div, 1, 5),
    };
    NoAST *raiz = programa(5, cmds);

    int erros = 0, nao_traduzidos = 0;
    char *cs = gerar_csharp(raiz, &erros, &nao_traduzidos);

    ASSERT(erros == 0, "programa semanticamente válido");
    ASSERT(nao_traduzidos == 0, "print traduzido");
    ASSERT(strstr(cs, "        static void Main(string[] args) {\n"
                      "            System.Globalization.CultureInfo.CurrentCulture = "
                      "System.Globalization.CultureInfo.InvariantCulture;\n") != NULL,
           "Main começa com InvariantCulture quando há print");
    ASSERT(strstr(cs, "            Console.WriteLine(\"hello, world\");\n") != NULL,
           "print com 1 argumento");
    ASSERT(strstr(cs, "            Console.WriteLine(string.Join(\" \", \"x =\", x));\n") != NULL,
           "print com vários argumentos separados por espaço");
    ASSERT(strstr(cs, "            Console.WriteLine();\n") != NULL, "print sem argumentos");
    ASSERT(strstr(cs, "            Console.WriteLine((double)x / 2);\n") != NULL,
           "tipo_resolvido dos argumentos é inferido");

    free(cs);
    liberar_ast(raiz);

    /* Sem print, o Main não muda a cultura. */
    NoAST *cmds_sem_print[] = { criar_no_assign("x", criar_no_int(1, 1), 1) };
    raiz = programa(1, cmds_sem_print);
    cs = gerar_csharp(raiz, &erros, &nao_traduzidos);
    ASSERT(strstr(cs, "CultureInfo") == NULL, "sem print não há CultureInfo");
    free(cs);
    liberar_ast(raiz);

    /* print("a" + 1) é erro semântico. */
    NoAST **arg_erro = malloc(sizeof(NoAST *));
    arg_erro[0] = criar_no_binop(OP_SOMA, criar_no_string("\"a\"", 1), criar_no_int(1, 1), 1);
    NoAST *cmds_erro[] = { criar_no_print(arg_erro, 1, 1) };
    raiz = programa(1, cmds_erro);
    cs = gerar_csharp(raiz, &erros, &nao_traduzidos);
    ASSERT(erros == 1, "print(\"a\" + 1) reporta erro semântico");
    free(cs);
    liberar_ast(raiz);

    printf("  PASSED\n");
    return 1;
}

static int test_if_else(void) {
    printf("Running test_if_else...\n");

    /* if isolado e if/else com variáveis declaradas no nível raiz */
    NoAST **b_if1 = malloc(sizeof(NoAST *));
    b_if1[0] = criar_no_assign("x", criar_no_int(20, 3), 3);

    NoAST **b_if2 = malloc(sizeof(NoAST *));
    b_if2[0] = criar_no_assign("flag", criar_no_int(1, 5), 5);
    NoAST **b_else2 = malloc(sizeof(NoAST *));
    b_else2[0] = criar_no_assign("flag", criar_no_int(2, 7), 7);

    NoAST *cmds[] = {
        criar_no_assign("x", criar_no_int(10, 1), 1),
        criar_no_assign("flag", criar_no_int(0, 1), 1),
        criar_no_if(criar_no_binop(OP_GT, criar_no_id("x", 2), criar_no_int(5, 2), 2),
                    criar_no_bloco(b_if1, 1), NULL, 0, NULL, 2),
        criar_no_if(criar_no_binop(OP_EQ, criar_no_id("x", 4), criar_no_int(20, 4), 4),
                    criar_no_bloco(b_if2, 1), NULL, 0,
                    criar_no_bloco(b_else2, 1), 4),
    };
    NoAST *raiz = programa(4, cmds);

    int erros = 0, nao_traduzidos = 0;
    char *cs = gerar_csharp(raiz, &erros, &nao_traduzidos);

    ASSERT(erros == 0, "programa semanticamente válido");
    ASSERT(nao_traduzidos == 0, "if e if/else totalmente traduzidos");
    ASSERT(strstr(cs,
                  "            if (x > 5) {\n"
                  "                x = 20;\n"
                  "            }\n") != NULL,
           "if isolado com indentação de 4 espaços por nível");
    ASSERT(strstr(cs,
                  "            if (x == 20) {\n"
                  "                flag = 1;\n"
                  "            } else {\n"
                  "                flag = 2;\n"
                  "            }\n") != NULL,
           "if/else com indentação de 4 espaços por nível");

    free(cs);
    liberar_ast(raiz);

    printf("  PASSED\n");
    return 1;
}

static int test_if_elif_else(void) {
    printf("Running test_if_elif_else...\n");

    /* Múltiplos elif com condição composta e else */
    NoAST **b_if = malloc(sizeof(NoAST *));
    b_if[0] = criar_no_assign("res", criar_no_int(1, 3), 3);

    NoAST **b_elif1 = malloc(sizeof(NoAST *));
    b_elif1[0] = criar_no_assign("res", criar_no_int(2, 5), 5);
    NoAST *cond_elif1 = criar_no_binop(
        OP_AND,
        criar_no_binop(OP_GT, criar_no_id("x", 4), criar_no_int(5, 4), 4),
        criar_no_binop(OP_LTE, criar_no_id("x", 4), criar_no_int(10, 4), 4),
        4);

    NoAST **b_elif2 = malloc(sizeof(NoAST *));
    b_elif2[0] = criar_no_assign("res", criar_no_int(3, 7), 7);

    NoAST **elifs1 = malloc(sizeof(NoAST *) * 2);
    elifs1[0] = criar_no_elif(cond_elif1, criar_no_bloco(b_elif1, 1), 4);
    elifs1[1] = criar_no_elif(
        criar_no_binop(OP_EQ, criar_no_id("x", 6), criar_no_int(0, 6), 6),
        criar_no_bloco(b_elif2, 1), 6);

    NoAST **b_else = malloc(sizeof(NoAST *));
    b_else[0] = criar_no_assign("res", criar_no_int(4, 9), 9);

    NoAST *cmds1[] = {
        criar_no_assign("x", criar_no_int(7, 1), 1),
        criar_no_assign("res", criar_no_int(0, 1), 1),
        criar_no_if(criar_no_binop(OP_GT, criar_no_id("x", 2), criar_no_int(10, 2), 2),
                    criar_no_bloco(b_if, 1), elifs1, 2,
                    criar_no_bloco(b_else, 1), 2),
    };
    NoAST *raiz1 = programa(3, cmds1);

    int erros = 0, nao_traduzidos = 0;
    char *cs1 = gerar_csharp(raiz1, &erros, &nao_traduzidos);

    ASSERT(erros == 0, "cadeia if/elif/else válida");
    ASSERT(nao_traduzidos == 0, "if/elif/else traduzido por completo");
    ASSERT(strstr(cs1,
                  "            if (x > 10) {\n"
                  "                res = 1;\n"
                  "            } else if (x > 5 && x <= 10) {\n"
                  "                res = 2;\n"
                  "            } else if (x == 0) {\n"
                  "                res = 3;\n"
                  "            } else {\n"
                  "                res = 4;\n"
                  "            }\n") != NULL,
           "emite if / else if (composto) / else if / else com indentação exata");

    free(cs1);
    liberar_ast(raiz1);

    /* if/elif sem else */
    NoAST **b_if_sem = malloc(sizeof(NoAST *));
    b_if_sem[0] = criar_no_assign("res", criar_no_int(10, 3), 3);
    NoAST **b_elif_sem = malloc(sizeof(NoAST *));
    b_elif_sem[0] = criar_no_assign("res", criar_no_int(20, 5), 5);
    NoAST **elifs2 = malloc(sizeof(NoAST *));
    elifs2[0] = criar_no_elif(
        criar_no_binop(OP_LT, criar_no_id("x", 4), criar_no_int(0, 4), 4),
        criar_no_bloco(b_elif_sem, 1), 4);

    NoAST *cmds2[] = {
        criar_no_assign("x", criar_no_int(-1, 1), 1),
        criar_no_assign("res", criar_no_int(0, 1), 1),
        criar_no_if(criar_no_binop(OP_GT, criar_no_id("x", 2), criar_no_int(0, 2), 2),
                    criar_no_bloco(b_if_sem, 1), elifs2, 1, NULL, 2),
    };
    NoAST *raiz2 = programa(3, cmds2);

    char *cs2 = gerar_csharp(raiz2, &erros, &nao_traduzidos);
    ASSERT(erros == 0, "if/elif sem else válido");
    ASSERT(nao_traduzidos == 0, "if/elif sem else traduzido");
    ASSERT(strstr(cs2,
                  "            if (x > 0) {\n"
                  "                res = 10;\n"
                  "            } else if (x < 0) {\n"
                  "                res = 20;\n"
                  "            }\n") != NULL,
           "if/elif sem else não emite bloco else");
    ASSERT(strstr(cs2, "} else {") == NULL, "sem else final");

    free(cs2);
    liberar_ast(raiz2);

    printf("  PASSED\n");
    return 1;
}

static int test_while(void) {
    printf("Running test_while...\n");

    NoAST **corpo1 = malloc(sizeof(NoAST *) * 2);
    corpo1[0] = criar_no_assign("soma",
        criar_no_binop(OP_SOMA, criar_no_id("soma", 4), criar_no_id("n", 4), 4), 4);
    corpo1[1] = criar_no_assign("n",
        criar_no_binop(OP_SUB, criar_no_id("n", 5), criar_no_int(1, 5), 5), 5);

    NoAST **corpo2 = malloc(sizeof(NoAST *));
    corpo2[0] = criar_no_assign("n",
        criar_no_binop(OP_SOMA, criar_no_id("n", 8), criar_no_int(1, 8), 8), 8);
    NoAST *cond2 = criar_no_binop(
        OP_AND,
        criar_no_binop(OP_LT, criar_no_id("n", 7), criar_no_int(5, 7), 7),
        criar_no_id("ativo", 7),
        7);

    NoAST *cmds[] = {
        criar_no_assign("n", criar_no_int(3, 1), 1),
        criar_no_assign("soma", criar_no_int(0, 2), 2),
        criar_no_assign("ativo", criar_no_bool(1, 3), 3),
        criar_no_while(
            criar_no_binop(OP_GT, criar_no_id("n", 3), criar_no_int(0, 3), 3),
            criar_no_bloco(corpo1, 2), 3),
        criar_no_while(cond2, criar_no_bloco(corpo2, 1), 7),
    };
    NoAST *raiz = programa(5, cmds);

    int erros = 0, nao_traduzidos = 0;
    char *cs = gerar_csharp(raiz, &erros, &nao_traduzidos);

    ASSERT(erros == 0, "while semanticamente válido");
    ASSERT(nao_traduzidos == 0, "while totalmente traduzido");
    ASSERT(strstr(cs,
                  "            while (n > 0) {\n"
                  "                soma = soma + n;\n"
                  "                n = n - 1;\n"
                  "            }\n") != NULL,
           "while simples com atualização de variáveis");
    ASSERT(strstr(cs,
                  "            while (n < 5 && ativo) {\n"
                  "                n = n + 1;\n"
                  "            }\n") != NULL,
           "while com condição booleana composta");

    free(cs);
    liberar_ast(raiz);

    printf("  PASSED\n");
    return 1;
}

static int test_hoisting_blocos(void) {
    printf("Running test_hoisting_blocos...\n");

    /*
     * 1. def f(): local_func = 99   (não vaza para Main)
     * 2. nota = 7                   (raiz: não sofre hoist)
     * 3. if nota >= 5:
     *        situacao = "aprovado"  (criada em bloco: sofre hoist como string)
     *        string = 1             (palavra reservada em bloco: int @string = default(int))
     *    elif nota > 0:
     *        nivel = 10             (criada em elif: sofre hoist como int)
     *    else:
     *        situacao = "reprovado"
     * 4. nivel = 20                 (reatribuída na raiz após bloco: sem redeclaração)
     * 5. while nota > 6:
     *        em_while = True        (criada em while: sofre hoist como bool)
     *        nota = nota - 1
     * 6. print(situacao)
     */
    NoAST **b_func = malloc(sizeof(NoAST *));
    b_func[0] = criar_no_assign("local_func", criar_no_int(99, 2), 2);

    NoAST **b_if = malloc(sizeof(NoAST *) * 2);
    b_if[0] = criar_no_assign("situacao", criar_no_string("\"aprovado\"", 5), 5);
    b_if[1] = criar_no_assign("string", criar_no_int(1, 6), 6);

    NoAST **b_elif = malloc(sizeof(NoAST *));
    b_elif[0] = criar_no_assign("nivel", criar_no_int(10, 8), 8);
    NoAST **elifs = malloc(sizeof(NoAST *));
    elifs[0] = criar_no_elif(
        criar_no_binop(OP_GT, criar_no_id("nota", 7), criar_no_int(0, 7), 7),
        criar_no_bloco(b_elif, 1), 7);

    NoAST **b_else = malloc(sizeof(NoAST *));
    b_else[0] = criar_no_assign("situacao", criar_no_string("\"reprovado\"", 10), 10);

    NoAST **b_while = malloc(sizeof(NoAST *) * 2);
    b_while[0] = criar_no_assign("em_while", criar_no_bool(1, 13), 13);
    b_while[1] = criar_no_assign("nota",
        criar_no_binop(OP_SUB, criar_no_id("nota", 14), criar_no_int(1, 14), 14), 14);

    NoAST **arg_print = malloc(sizeof(NoAST *));
    arg_print[0] = criar_no_id("situacao", 15);

    NoAST *cmds[] = {
        criar_no_funcdef("f", NULL, 0, criar_no_bloco(b_func, 1), 1),
        criar_no_assign("nota", criar_no_int(7, 3), 3),
        criar_no_if(
            criar_no_binop(OP_GTE, criar_no_id("nota", 4), criar_no_int(5, 4), 4),
            criar_no_bloco(b_if, 2), elifs, 1,
            criar_no_bloco(b_else, 1), 4),
        criar_no_assign("nivel", criar_no_int(20, 11), 11),
        criar_no_while(
            criar_no_binop(OP_GT, criar_no_id("nota", 12), criar_no_int(6, 12), 12),
            criar_no_bloco(b_while, 2), 12),
        criar_no_print(arg_print, 1, 15),
    };
    NoAST *raiz = programa(6, cmds);

    int erros = 0, nao_traduzidos = 0;
    char *cs = gerar_csharp(raiz, &erros, &nao_traduzidos);

    ASSERT(erros == 0, "programa válido com uso pós-bloco");
    ASSERT(nao_traduzidos == 1, "apenas def f() pendente");
    ASSERT(strstr(cs, "            string situacao = default(string);\n") != NULL,
           "situacao sofre hoist com default(string)");
    ASSERT(strstr(cs, "            int @string = default(int);\n") != NULL,
           "palavra reservada em bloco sofre hoist com escape @");
    ASSERT(strstr(cs, "            int nivel = default(int);\n") != NULL,
           "nivel de elif sofre hoist com default(int)");
    ASSERT(strstr(cs, "            bool em_while = default(bool);\n") != NULL,
           "em_while de while sofre hoist com default(bool)");
    ASSERT(strstr(cs, "int nota = default(int)") == NULL,
           "variável do nível raiz não sofre hoist");
    ASSERT(strstr(cs, "            int nota = 7;\n") != NULL,
           "variável do nível raiz é declarada inline");
    ASSERT(strstr(cs, "                situacao = \"aprovado\";\n") != NULL,
           "atribuição em if sem redeclarar tipo");
    ASSERT(strstr(cs, "                @string = 1;\n") != NULL,
           "palavra reservada atribuída em bloco usa @");
    ASSERT(strstr(cs, "            nivel = 20;\n") != NULL,
           "variável hoisted reatribuída na raiz não redeclara");
    ASSERT(strstr(cs, "int nivel = 20") == NULL,
           "não redeclara nivel na raiz");
    ASSERT(strstr(cs, "local_func") == NULL,
           "variável interna de NO_FUNCDEF não vaza para Main");

    /* Hoist aparece após CultureInfo e antes do primeiro comando do Main */
    const char *pos_cultura = strstr(cs, "InvariantCulture;");
    const char *pos_hoist = strstr(cs, "string situacao = default(string);");
    const char *pos_nota = strstr(cs, "int nota = 7;");
    ASSERT(pos_cultura != NULL && pos_hoist != NULL && pos_nota != NULL &&
           pos_cultura < pos_hoist && pos_hoist < pos_nota,
           "hoisting emitido após CultureInfo e antes dos comandos");

    free(cs);
    liberar_ast(raiz);

    /* Fallback TIPO_VOID / TIPO_DESCONHECIDO -> object (nunca default(var)) */
    NoAST **b_sem_ret = malloc(sizeof(NoAST *));
    b_sem_ret[0] = criar_no_assign("tmp", criar_no_int(1, 2), 2);
    NoAST **b_desc = malloc(sizeof(NoAST *));
    b_desc[0] = criar_no_assign("misterio", criar_no_funccall("sem_ret", NULL, 0, 4), 4);
    NoAST *cmds_desc[] = {
        criar_no_funcdef("sem_ret", NULL, 0, criar_no_bloco(b_sem_ret, 1), 1),
        criar_no_if(criar_no_bool(1, 3), criar_no_bloco(b_desc, 1), NULL, 0, NULL, 3),
    };
    NoAST *raiz_desc = programa(2, cmds_desc);

    char *cs_desc = gerar_csharp(raiz_desc, &erros, &nao_traduzidos);
    ASSERT(erros == 0, "def sem return com atribuição em if é válido");
    ASSERT(nao_traduzidos == 2, "def e chamada pendentes");
    ASSERT(strstr(cs_desc, "            object misterio = default(object);\n") != NULL,
           "TIPO_VOID faz fallback para object = default(object)");
    ASSERT(strstr(cs_desc, "default(var)") == NULL,
           "nunca emite default(var)");

    free(cs_desc);
    liberar_ast(raiz_desc);

    NoAST **b_unk = malloc(sizeof(NoAST *));
    b_unk[0] = criar_no_assign("misterio_unk",
        criar_no_binop(OP_SOMA, criar_no_string("\"a\"", 2), criar_no_int(1, 2), 2), 2);
    NoAST *cmds_unk[] = {
        criar_no_if(criar_no_bool(1, 1), criar_no_bloco(b_unk, 1), NULL, 0, NULL, 1),
    };
    NoAST *raiz_unk = programa(1, cmds_unk);

    char *cs_unk = gerar_csharp(raiz_unk, &erros, &nao_traduzidos);
    ASSERT(erros == 1, "operação inválida produz erro semântico e TIPO_DESCONHECIDO");
    ASSERT(nao_traduzidos == 0, "if com TIPO_DESCONHECIDO traduzido");
    ASSERT(strstr(cs_unk, "            object misterio_unk = default(object);\n") != NULL,
           "TIPO_DESCONHECIDO faz fallback para object = default(object)");
    ASSERT(strstr(cs_unk, "default(var)") == NULL,
           "nunca emite default(var) para TIPO_DESCONHECIDO");

    free(cs_unk);
    liberar_ast(raiz_unk);

    printf("  PASSED\n");
    return 1;
}

static int test_for_range_basico(void) {
    printf("Running test_for_range_basico...\n");

    /*
     * n = 3
     * k = 0                 (declarada no nível raiz: não sofre hoist)
     * soma = 0
     * for i in range(n + 1):
     *     soma = soma + i
     * for i in range(1, 4): (reuso do mesmo índice i)
     *     soma = soma + i
     * for j in range(2, 8):
     *     soma = soma + j
     * for k in range(3):    (usa k já declarada no nível raiz)
     *     soma = soma + k
     * for class in range(2): (palavra reservada como índice)
     *     soma = soma + class
     * print(i)              (uso pós-laço)
     */
    NoAST **b_f1 = malloc(sizeof(NoAST *));
    b_f1[0] = criar_no_assign("soma",
        criar_no_binop(OP_SOMA, criar_no_id("soma", 5), criar_no_id("i", 5), 5), 5);

    NoAST **args_r2 = malloc(sizeof(NoAST *) * 2);
    args_r2[0] = criar_no_int(1, 6);
    args_r2[1] = criar_no_int(4, 6);
    NoAST **b_f2 = malloc(sizeof(NoAST *));
    b_f2[0] = criar_no_assign("soma",
        criar_no_binop(OP_SOMA, criar_no_id("soma", 7), criar_no_id("i", 7), 7), 7);

    NoAST **args_r3 = malloc(sizeof(NoAST *) * 2);
    args_r3[0] = criar_no_int(2, 8);
    args_r3[1] = criar_no_int(8, 8);
    NoAST **b_f3 = malloc(sizeof(NoAST *));
    b_f3[0] = criar_no_assign("soma",
        criar_no_binop(OP_SOMA, criar_no_id("soma", 9), criar_no_id("j", 9), 9), 9);

    NoAST **b_f4 = malloc(sizeof(NoAST *));
    b_f4[0] = criar_no_assign("soma",
        criar_no_binop(OP_SOMA, criar_no_id("soma", 11), criar_no_id("k", 11), 11), 11);

    NoAST **b_f5 = malloc(sizeof(NoAST *));
    b_f5[0] = criar_no_assign("soma",
        criar_no_binop(OP_SOMA, criar_no_id("soma", 13), criar_no_id("class", 13), 13), 13);

    NoAST **arg_print = malloc(sizeof(NoAST *));
    arg_print[0] = criar_no_id("i", 14);

    NoAST *cmds[] = {
        criar_no_assign("n", criar_no_int(3, 1), 1),
        criar_no_assign("k", criar_no_int(0, 2), 2),
        criar_no_assign("soma", criar_no_int(0, 3), 3),
        criar_no_for("i",
            criar_no_binop(OP_SOMA, criar_no_id("n", 4), criar_no_int(1, 4), 4),
            criar_no_bloco(b_f1, 1), 4),
        criar_no_for("i", criar_no_bloco(args_r2, 2), criar_no_bloco(b_f2, 1), 6),
        criar_no_for("j", criar_no_bloco(args_r3, 2), criar_no_bloco(b_f3, 1), 8),
        criar_no_for("k", criar_no_int(3, 10), criar_no_bloco(b_f4, 1), 10),
        criar_no_for("class", criar_no_int(2, 12), criar_no_bloco(b_f5, 1), 12),
        criar_no_print(arg_print, 1, 14),
    };
    NoAST *raiz = programa(9, cmds);

    int erros = 0, nao_traduzidos = 0;
    char *cs = gerar_csharp(raiz, &erros, &nao_traduzidos);

    ASSERT(erros == 0, "for range(1/2 args) semanticamente válido");
    ASSERT(nao_traduzidos == 0, "for range(1/2 args) traduzido por completo");

    const char *decl_i = strstr(cs, "int i = default(int);");
    ASSERT(decl_i != NULL, "índice i sofre hoist como int");
    ASSERT(strstr(decl_i + 1, "int i = default(int);") == NULL,
           "reuso de i em múltiplos for não gera segunda declaração");

    ASSERT(strstr(cs, "            int j = default(int);\n") != NULL,
           "índice j sofre hoist como int");
    ASSERT(strstr(cs, "            int @class = default(int);\n") != NULL,
           "palavra reservada como índice sofre hoist com @");
    ASSERT(strstr(cs, "int k = default(int)") == NULL,
           "k já declarada na raiz não sofre hoist");
    ASSERT(strstr(cs, "            int k = 0;\n") != NULL,
           "k é declarada inline no nível raiz");

    ASSERT(strstr(cs,
                  "            for (i = 0; i < n + 1; i++) {\n"
                  "                soma = soma + i;\n"
                  "            }\n") != NULL,
           "range(n + 1) emite i < n + 1 sem parênteses extras");
    ASSERT(strstr(cs,
                  "            for (i = 1; i < 4; i++) {\n"
                  "                soma = soma + i;\n"
                  "            }\n") != NULL,
           "reuso de i em range(1, 4)");
    ASSERT(strstr(cs,
                  "            for (j = 2; j < 8; j++) {\n"
                  "                soma = soma + j;\n"
                  "            }\n") != NULL,
           "range(2, 8) com 2 argumentos");
    ASSERT(strstr(cs,
                  "            for (k = 0; k < 3; k++) {\n"
                  "                soma = soma + k;\n"
                  "            }\n") != NULL,
           "for com variável já declarada na raiz");
    ASSERT(strstr(cs,
                  "            for (@class = 0; @class < 2; @class++) {\n"
                  "                soma = soma + @class;\n"
                  "            }\n") != NULL,
           "for com palavra reservada escapada com @");
    ASSERT(strstr(cs, "            Console.WriteLine(i);\n") != NULL,
           "uso pós-laço do índice i");

    free(cs);
    liberar_ast(raiz);

    printf("  PASSED\n");
    return 1;
}

static NoAST *range_lista(int n, NoAST *a, NoAST *b, NoAST *c, NoAST *d) {
    NoAST **args = n > 0 ? malloc(sizeof(NoAST *) * n) : NULL;
    if (n >= 1) args[0] = a;
    if (n >= 2) args[1] = b;
    if (n >= 3) args[2] = c;
    if (n >= 4) args[3] = d;
    return criar_no_bloco(args, n);
}

static NoAST *bloco_soma_var(const char *var_soma, const char *var_i, int linha) {
    NoAST **cmds = malloc(sizeof(NoAST *));
    cmds[0] = criar_no_assign(
        (char *)var_soma,
        criar_no_binop(OP_SOMA, criar_no_id((char *)var_soma, linha),
                       criar_no_id((char *)var_i, linha), linha),
        linha);
    return criar_no_bloco(cmds, 1);
}

static int test_for_range_passo(void) {
    printf("Running test_for_range_passo...\n");

    /*
     * _passo0 = 2           (testa prevenção de colisão com temporário)
     * soma = 0
     * for i in range(0, 5, 1):      -> i++
     * for i in range(0, 10, 2):     -> i += 2
     * for i in range(10, 0, -1):    -> i > 0; i--
     * for i in range(10, 0, -2):    -> i > 0; i -= 2
     * for i in range(0, 5, 0):      -> guarda _passo1 == 0
     * for i in range(0, 6, _passo0): -> guarda _passo2 == 0 + ternário
     */
    NoAST *cmds[] = {
        criar_no_assign("_passo0", criar_no_int(2, 1), 1),
        criar_no_assign("soma", criar_no_int(0, 2), 2),
        criar_no_for("i", range_lista(3, criar_no_int(0, 3), criar_no_int(5, 3), criar_no_int(1, 3), NULL),
                     bloco_soma_var("soma", "i", 3), 3),
        criar_no_for("i", range_lista(3, criar_no_int(0, 4), criar_no_int(10, 4), criar_no_int(2, 4), NULL),
                     bloco_soma_var("soma", "i", 4), 4),
        criar_no_for("i", range_lista(3, criar_no_int(10, 5), criar_no_int(0, 5),
                                      criar_no_unaryop(OP_NEG, criar_no_int(1, 5), 5), NULL),
                     bloco_soma_var("soma", "i", 5), 5),
        criar_no_for("i", range_lista(3, criar_no_int(10, 6), criar_no_int(0, 6),
                                      criar_no_unaryop(OP_NEG, criar_no_int(2, 6), 6), NULL),
                     bloco_soma_var("soma", "i", 6), 6),
        criar_no_for("i", range_lista(3, criar_no_int(0, 7), criar_no_int(5, 7), criar_no_int(0, 7), NULL),
                     bloco_soma_var("soma", "i", 7), 7),
        criar_no_for("i", range_lista(3, criar_no_int(0, 8), criar_no_int(6, 8), criar_no_id("_passo0", 8), NULL),
                     bloco_soma_var("soma", "i", 8), 8),
    };
    NoAST *raiz = programa(8, cmds);

    int erros = 0, nao_traduzidos = 0;
    char *cs = gerar_csharp(raiz, &erros, &nao_traduzidos);

    ASSERT(erros == 0, "for range(a, b, passo) semanticamente válido");
    ASSERT(nao_traduzidos == 0, "todos os casos de 3 argumentos traduzidos");

    ASSERT(strstr(cs, "            for (i = 0; i < 5; i++) {\n") != NULL,
           "passo constante +1 usa i++");
    ASSERT(strstr(cs, "            for (i = 0; i < 10; i += 2) {\n") != NULL,
           "passo constante +2 usa i += 2");
    ASSERT(strstr(cs, "            for (i = 10; i > 0; i--) {\n") != NULL,
           "passo constante -1 usa i > 0 e i--");
    ASSERT(strstr(cs, "            for (i = 10; i > 0; i -= 2) {\n") != NULL,
           "passo constante -2 usa i > 0 e i -= 2");

    /* Passo 0 constante: pula _passo0 (já declarada pelo usuário) e usa _passo1 */
    ASSERT(strstr(cs,
                  "            int _passo1 = 0;\n"
                  "            if (_passo1 == 0) {\n"
                  "                throw new ArgumentException(\"range() arg 3 must not be zero\");\n"
                  "            }\n"
                  "            for (i = 0; _passo1 > 0 ? i < 5 : i > 5; i += _passo1) {\n") != NULL,
           "passo 0 emite guarda ArgumentException sem colisão com _passo0");

    /* Passo dinâmico: avança para _passo2 */
    ASSERT(strstr(cs,
                  "            int _passo2 = _passo0;\n"
                  "            if (_passo2 == 0) {\n"
                  "                throw new ArgumentException(\"range() arg 3 must not be zero\");\n"
                  "            }\n"
                  "            for (i = 0; _passo2 > 0 ? i < 6 : i > 6; i += _passo2) {\n") != NULL,
           "passo dinâmico usa temporário único _passo2 e operador ternário");

    free(cs);
    liberar_ast(raiz);

    /* Passo dinâmico sem colisão usa _passo0 e limite com PREC_RELACIONAL */
    NoAST *cmds_din[] = {
        criar_no_assign("acc", criar_no_int(0, 1), 1),
        criar_no_assign("n", criar_no_int(4, 1), 1),
        criar_no_assign("p", criar_no_int(2, 1), 1),
        criar_no_for("j",
            range_lista(3, criar_no_int(0, 2),
                        criar_no_binop(OP_SOMA, criar_no_id("n", 2), criar_no_int(1, 2), 2),
                        criar_no_binop(OP_SOMA, criar_no_id("p", 2), criar_no_int(1, 2), 2),
                        NULL),
            bloco_soma_var("acc", "j", 3), 2),
    };
    NoAST *raiz_din = programa(4, cmds_din);
    char *cs_din = gerar_csharp(raiz_din, &erros, &nao_traduzidos);
    ASSERT(erros == 0 && nao_traduzidos == 0, "passo dinâmico com expressão traduzido");
    ASSERT(strstr(cs_din,
                  "            int _passo0 = p + 1;\n"
                  "            if (_passo0 == 0) {\n"
                  "                throw new ArgumentException(\"range() arg 3 must not be zero\");\n"
                  "            }\n"
                  "            for (j = 0; _passo0 > 0 ? j < n + 1 : j > n + 1; j += _passo0) {\n") != NULL,
           "passo dinâmico sem colisão gera _passo0 e preserva precedência");
    free(cs_din);
    liberar_ast(raiz_din);

    /*
     * Caso de borda: _passo0 declarada no nível raiz DEPOIS de for com passo
     * dinâmico dentro de um bloco if. Temporário deve evitar _passo0 -> _passo1
     * e a variável da raiz deve manter declaração explícita 'int _passo0 = 10;'.
     */
    NoAST **b_if_for = malloc(sizeof(NoAST *));
    b_if_for[0] = criar_no_for("i",
        range_lista(3, criar_no_int(0, 4), criar_no_int(4, 4), criar_no_id("p", 4), NULL),
        bloco_soma_var("soma", "i", 5), 4);
    NoAST *cmds_pos[] = {
        criar_no_assign("p", criar_no_int(2, 1), 1),
        criar_no_assign("soma", criar_no_int(0, 2), 2),
        criar_no_if(criar_no_bool(1, 3), criar_no_bloco(b_if_for, 1), NULL, 0, NULL, 3),
        criar_no_assign("_passo0", criar_no_int(10, 6), 6),
    };
    NoAST *raiz_pos = programa(4, cmds_pos);
    char *cs_pos = gerar_csharp(raiz_pos, &erros, &nao_traduzidos);
    ASSERT(erros == 0 && nao_traduzidos == 0, "for em bloco com _passo0 posterior na raiz");
    ASSERT(strstr(cs_pos, "                int _passo1 = p;\n") != NULL,
           "temporário evita colisão com _passo0 declarada após o bloco");
    ASSERT(strstr(cs_pos, "                for (i = 0; _passo1 > 0 ? i < 4 : i > 4; i += _passo1) {\n") != NULL,
           "for usa _passo1 na condição e incremento");
    ASSERT(strstr(cs_pos, "            int _passo0 = 10;\n") != NULL,
           "variável _passo0 posterior na raiz mantém declaração int");
    ASSERT(strstr(cs_pos, "int _passo0 = p") == NULL,
           "não usa _passo0 como temporário do for");
    free(cs_pos);
    liberar_ast(raiz_pos);

    /* Aridade inválida: 0 e 4 argumentos devem cair em não traduzido */
    NoAST *cmds_inv[] = {
        criar_no_for("x", range_lista(0, NULL, NULL, NULL, NULL),
                     criar_no_bloco(NULL, 0), 1),
        criar_no_for("y", range_lista(4, criar_no_int(1, 2), criar_no_int(2, 2),
                                      criar_no_int(3, 2), criar_no_int(4, 2)),
                     criar_no_bloco(NULL, 0), 2),
    };
    NoAST *raiz_inv = programa(2, cmds_inv);
    char *cs_inv = gerar_csharp(raiz_inv, &erros, &nao_traduzidos);
    ASSERT(nao_traduzidos == 2, "range com 0 e 4 argumentos são contabilizados como não traduzidos");
    ASSERT(strstr(cs_inv, "// [não traduzido] for (linha 1)") != NULL,
           "range() com 0 args marcado como não traduzido");
    ASSERT(strstr(cs_inv, "// [não traduzido] for (linha 2)") != NULL,
           "range() com 4 args marcado como não traduzido");
    free(cs_inv);
    liberar_ast(raiz_inv);

    printf("  PASSED\n");
    return 1;
}

static int test_estruturas_aninhadas(void) {
    printf("Running test_estruturas_aninhadas...\n");

    /*
     * n = 3
     * total = 0
     * while n > 0:
     *     if n == 2:
     *         achou = True
     *     n = n - 1
     * for i in range(2):
     *     for j in range(3):
     *         total = total + i + j
     * for k in range(3):
     *     if k == 0:
     *         cat = "zero"
     *     elif k == 1:
     *         cat = "um"
     *     else:
     *         cat = "outro"
     *     print(cat)
     */
    NoAST **b_if_in_w = malloc(sizeof(NoAST *));
    b_if_in_w[0] = criar_no_assign("achou", criar_no_bool(1, 5), 5);
    NoAST **b_while = malloc(sizeof(NoAST *) * 2);
    b_while[0] = criar_no_if(
        criar_no_binop(OP_EQ, criar_no_id("n", 4), criar_no_int(2, 4), 4),
        criar_no_bloco(b_if_in_w, 1), NULL, 0, NULL, 4);
    b_while[1] = criar_no_assign("n",
        criar_no_binop(OP_SUB, criar_no_id("n", 6), criar_no_int(1, 6), 6), 6);

    NoAST **b_for_j = malloc(sizeof(NoAST *));
    b_for_j[0] = criar_no_assign("total",
        criar_no_binop(OP_SOMA,
            criar_no_binop(OP_SOMA, criar_no_id("total", 9), criar_no_id("i", 9), 9),
            criar_no_id("j", 9), 9),
        9);
    NoAST **b_for_i = malloc(sizeof(NoAST *));
    b_for_i[0] = criar_no_for("j", criar_no_int(3, 8), criar_no_bloco(b_for_j, 1), 8);

    NoAST **b_if_k = malloc(sizeof(NoAST *));
    b_if_k[0] = criar_no_assign("cat", criar_no_string("\"zero\"", 12), 12);
    NoAST **b_elif_k = malloc(sizeof(NoAST *));
    b_elif_k[0] = criar_no_assign("cat", criar_no_string("\"um\"", 14), 14);
    NoAST **elifs_k = malloc(sizeof(NoAST *));
    elifs_k[0] = criar_no_elif(
        criar_no_binop(OP_EQ, criar_no_id("k", 13), criar_no_int(1, 13), 13),
        criar_no_bloco(b_elif_k, 1), 13);
    NoAST **b_else_k = malloc(sizeof(NoAST *));
    b_else_k[0] = criar_no_assign("cat", criar_no_string("\"outro\"", 16), 16);
    NoAST **arg_print = malloc(sizeof(NoAST *));
    arg_print[0] = criar_no_id("cat", 17);
    NoAST **b_for_k = malloc(sizeof(NoAST *) * 2);
    b_for_k[0] = criar_no_if(
        criar_no_binop(OP_EQ, criar_no_id("k", 11), criar_no_int(0, 11), 11),
        criar_no_bloco(b_if_k, 1), elifs_k, 1,
        criar_no_bloco(b_else_k, 1), 11);
    b_for_k[1] = criar_no_print(arg_print, 1, 17);

    NoAST *cmds[] = {
        criar_no_assign("n", criar_no_int(3, 1), 1),
        criar_no_assign("total", criar_no_int(0, 2), 2),
        criar_no_while(
            criar_no_binop(OP_GT, criar_no_id("n", 3), criar_no_int(0, 3), 3),
            criar_no_bloco(b_while, 2), 3),
        criar_no_for("i", criar_no_int(2, 7), criar_no_bloco(b_for_i, 1), 7),
        criar_no_for("k", criar_no_int(3, 10), criar_no_bloco(b_for_k, 2), 10),
    };
    NoAST *raiz = programa(5, cmds);

    int erros = 0, nao_traduzidos = 0;
    char *cs = gerar_csharp(raiz, &erros, &nao_traduzidos);

    ASSERT(erros == 0, "estruturas aninhadas válidas");
    ASSERT(nao_traduzidos == 0, "estruturas aninhadas totalmente traduzidas");
    ASSERT(strstr(cs, "InvariantCulture;") != NULL,
           "print dentro de laço dispara InvariantCulture");
    ASSERT(strstr(cs, "            bool achou = default(bool);\n") != NULL,
           "achou de if dentro de while sofre hoist");
    ASSERT(strstr(cs, "            int i = default(int);\n") != NULL,
           "i sofre hoist");
    ASSERT(strstr(cs, "            int j = default(int);\n") != NULL,
           "j de for interno sofre hoist");
    ASSERT(strstr(cs, "            int k = default(int);\n") != NULL,
           "k sofre hoist");
    ASSERT(strstr(cs, "            string cat = default(string);\n") != NULL,
           "cat de if/elif/else dentro de for sofre hoist");

    ASSERT(strstr(cs,
                  "            while (n > 0) {\n"
                  "                if (n == 2) {\n"
                  "                    achou = true;\n"
                  "                }\n"
                  "                n = n - 1;\n"
                  "            }\n") != NULL,
           "if dentro de while com indentação de 12/16/20 espaços");

    ASSERT(strstr(cs,
                  "            for (i = 0; i < 2; i++) {\n"
                  "                for (j = 0; j < 3; j++) {\n"
                  "                    total = total + i + j;\n"
                  "                }\n"
                  "            }\n") != NULL,
           "for dentro de for com indentação de 12/16/20 espaços");

    ASSERT(strstr(cs,
                  "            for (k = 0; k < 3; k++) {\n"
                  "                if (k == 0) {\n"
                  "                    cat = \"zero\";\n"
                  "                } else if (k == 1) {\n"
                  "                    cat = \"um\";\n"
                  "                } else {\n"
                  "                    cat = \"outro\";\n"
                  "                }\n"
                  "                Console.WriteLine(cat);\n"
                  "            }\n") != NULL,
           "if/elif/else dentro de for com indentação de 12/16/20 espaços");

    int aberturas = 0, fechamentos = 0;
    for (const char *p = cs; *p != '\0'; p++) {
        if (*p == '{') aberturas++;
        else if (*p == '}') fechamentos++;
    }
    ASSERT(aberturas == fechamentos && aberturas > 0, "chaves { } balanceadas");

    free(cs);
    liberar_ast(raiz);

    printf("  PASSED\n");
    return 1;
}

static int test_hoisting_promocao_e_condicoes_especiais(void) {
    printf("Running test_hoisting_promocao_e_condicoes_especiais...\n");

    /*
     * cond = True
     * s = "a"
     * p = True
     * q = False
     * a = 1
     * b = 3
     * soma = 0
     * if cond:
     *     f_prom = 1.5
     * else:
     *     f_prom = 2
     * if s < "z":
     *     soma = soma + 1
     * while p > q:
     *     p = False
     * for i in range(0):
     *     soma = soma + i
     * for j in range(a + 1, b * 2):
     *     soma = soma + j
     */
    NoAST **b_if_prom = malloc(sizeof(NoAST *));
    b_if_prom[0] = criar_no_assign("f_prom", criar_no_float(1.5, 9), 9);

    NoAST **b_else_prom = malloc(sizeof(NoAST *));
    b_else_prom[0] = criar_no_assign("f_prom", criar_no_int(2, 11), 11);

    NoAST **b_if_str = malloc(sizeof(NoAST *));
    b_if_str[0] = criar_no_assign("soma",
        criar_no_binop(OP_SOMA, criar_no_id("soma", 13), criar_no_int(1, 13), 13), 13);

    NoAST **b_while_bool = malloc(sizeof(NoAST *));
    b_while_bool[0] = criar_no_assign("p", criar_no_bool(0, 15), 15);

    NoAST *range_comp = range_lista(2,
        criar_no_binop(OP_SOMA, criar_no_id("a", 18), criar_no_int(1, 18), 18),
        criar_no_binop(OP_MULT, criar_no_id("b", 18), criar_no_int(2, 18), 18),
        NULL, NULL);

    NoAST *cmds[] = {
        criar_no_assign("cond", criar_no_bool(1, 1), 1),
        criar_no_assign("s", criar_no_string("\"a\"", 2), 2),
        criar_no_assign("p", criar_no_bool(1, 3), 3),
        criar_no_assign("q", criar_no_bool(0, 4), 4),
        criar_no_assign("a", criar_no_int(1, 5), 5),
        criar_no_assign("b", criar_no_int(3, 6), 6),
        criar_no_assign("soma", criar_no_int(0, 7), 7),
        criar_no_if(criar_no_id("cond", 8), criar_no_bloco(b_if_prom, 1),
                    NULL, 0, criar_no_bloco(b_else_prom, 1), 8),
        criar_no_if(criar_no_binop(OP_LT, criar_no_id("s", 12), criar_no_string("\"z\"", 12), 12),
                    criar_no_bloco(b_if_str, 1), NULL, 0, NULL, 12),
        criar_no_while(criar_no_binop(OP_GT, criar_no_id("p", 14), criar_no_id("q", 14), 14),
                       criar_no_bloco(b_while_bool, 1), 14),
        criar_no_for("i", criar_no_int(0, 16), bloco_soma_var("soma", "i", 17), 16),
        criar_no_for("j", range_comp, bloco_soma_var("soma", "j", 19), 18),
    };
    NoAST *raiz = programa(12, cmds);

    int erros = 0, nao_traduzidos = 0;
    char *cs = gerar_csharp(raiz, &erros, &nao_traduzidos);

    ASSERT(erros == 0, "promoção ADR-001 e condições especiais válidas");
    ASSERT(nao_traduzidos == 0, "todos os comandos traduzidos");
    ASSERT(strstr(cs, "            double f_prom = default(double);\n") != NULL,
           "f_prom sofre hoist como double (promoção ADR-001)");
    ASSERT(strstr(cs, "                f_prom = 1.5;\n") != NULL,
           "atribuição float em if sem redeclarar");
    ASSERT(strstr(cs, "                f_prom = 2;\n") != NULL,
           "atribuição int promovida em else sem redeclarar");
    ASSERT(strstr(cs, "double f_prom = 1.5") == NULL && strstr(cs, "int f_prom") == NULL,
           "f_prom nunca é redeclarada");
    ASSERT(strstr(cs, "            if (string.CompareOrdinal(s, \"z\") < 0) {\n") != NULL,
           "if usa string.CompareOrdinal para comparação de strings");
    ASSERT(strstr(cs, "            while ((p ? 1 : 0) > (q ? 1 : 0)) {\n") != NULL,
           "while usa comparação ordinal de booleanos");
    ASSERT(strstr(cs,
                  "            for (i = 0; i < 0; i++) {\n"
                  "                soma = soma + i;\n"
                  "            }\n") != NULL,
           "for com 0 iterações range(0)");
    ASSERT(strstr(cs,
                  "            for (j = a + 1; j < b * 2; j++) {\n"
                  "                soma = soma + j;\n"
                  "            }\n") != NULL,
           "for com limites compostos range(a + 1, b * 2)");

    free(cs);
    liberar_ast(raiz);

    printf("  PASSED\n");
    return 1;
}

static int test_nao_suportado_em_estruturas_controle(void) {
    printf("Running test_nao_suportado_em_estruturas_controle...\n");

    /*
     * def f():
     *     return 1
     * if f() > 0:
     *     x = 1
     * for i in range(f()):
     *     x = 2
     */
    NoAST **b_func = malloc(sizeof(NoAST *));
    b_func[0] = criar_no_return(criar_no_int(1, 2), 2);

    NoAST **b_if = malloc(sizeof(NoAST *));
    b_if[0] = criar_no_assign("x", criar_no_int(1, 4), 4);

    NoAST **b_for = malloc(sizeof(NoAST *));
    b_for[0] = criar_no_assign("x", criar_no_int(2, 6), 6);

    NoAST *cmds[] = {
        criar_no_funcdef("f", NULL, 0, criar_no_bloco(b_func, 1), 1),
        criar_no_if(
            criar_no_binop(OP_GT, criar_no_funccall("f", NULL, 0, 3), criar_no_int(0, 3), 3),
            criar_no_bloco(b_if, 1), NULL, 0, NULL, 3),
        criar_no_for("i", criar_no_funccall("f", NULL, 0, 5),
                     criar_no_bloco(b_for, 1), 5),
    };
    NoAST *raiz = programa(3, cmds);

    int erros = 0, nao_traduzidos = 0;
    char *cs = gerar_csharp(raiz, &erros, &nao_traduzidos);

    ASSERT(erros == 0, "def + chamada em if e range sem erro semântico");
    ASSERT(nao_traduzidos == 3,
           "exatamente 3 construções não traduzidas (def, f() em if, f() em for)");
    ASSERT(strstr(cs, "// [não traduzido] def (linha 1)") != NULL,
           "def marcado como não traduzido");
    ASSERT(strstr(cs,
                  "            if (/* não traduzido: chamada de função (linha 3) */ default(int) > 0) {\n"
                  "                x = 1;\n"
                  "            }\n") != NULL,
           "chamada não suportada em condição if emite placeholder default(int)");
    ASSERT(strstr(cs,
                  "            for (i = 0; i < /* não traduzido: chamada de função (linha 5) */ default(int); i++) {\n"
                  "                x = 2;\n"
                  "            }\n") != NULL,
           "chamada não suportada em range() emite placeholder default(int)");

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
        test_construcoes_pendentes(),
        test_print(),
        test_if_else(),
        test_if_elif_else(),
        test_while(),
        test_hoisting_blocos(),
        test_for_range_basico(),
        test_for_range_passo(),
        test_estruturas_aninhadas(),
        test_hoisting_promocao_e_condicoes_especiais(),
        test_nao_suportado_em_estruturas_controle()
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
