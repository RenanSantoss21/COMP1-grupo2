#include "codegen_csharp.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define INDENTACAO "    "
#define NAMESPACE_CSHARP "CompiladorPython"

/* ── Buffer de texto dinâmico ────────────────────────────────────── */

typedef struct {
    char  *dados;
    size_t tamanho;
    size_t capacidade;
} Buffer;

static void buffer_iniciar(Buffer *b) {
    b->capacidade = 64;
    b->tamanho = 0;
    b->dados = malloc(b->capacidade);
    if (b->dados == NULL) {
        fprintf(stderr, "Erro: falha ao alocar memória no gerador C#.\n");
        exit(1);
    }
    b->dados[0] = '\0';
}

static void buffer_anexar(Buffer *b, const char *texto) {
    size_t n = strlen(texto);
    if (b->tamanho + n + 1 > b->capacidade) {
        while (b->tamanho + n + 1 > b->capacidade) b->capacidade *= 2;
        b->dados = realloc(b->dados, b->capacidade);
        if (b->dados == NULL) {
            fprintf(stderr, "Erro: falha ao alocar memória no gerador C#.\n");
            exit(1);
        }
    }
    memcpy(b->dados + b->tamanho, texto, n + 1);
    b->tamanho += n;
}

static void buffer_anexar_char(Buffer *b, char c) {
    char s[2] = { c, '\0' };
    buffer_anexar(b, s);
}

static void buffer_anexarf(Buffer *b, const char *formato, ...) {
    char temp[256];
    va_list ap;
    va_start(ap, formato);
    vsnprintf(temp, sizeof(temp), formato, ap);
    va_end(ap);
    buffer_anexar(b, temp);
}

/* ── Tipos ───────────────────────────────────────────────────────── */

const char *codegen_csharp_tipo(TipoDado tipo) {
    switch (tipo) {
        case TIPO_INT:    return "int";
        case TIPO_FLOAT:  return "double";
        case TIPO_STRING: return "string";
        case TIPO_BOOL:   return "bool";
        case TIPO_VOID:   return "void";
        default:          return "var";
    }
}

/* ── Identificadores ─────────────────────────────────────────────── */

/* Palavras reservadas do C# que são identificadores válidos em Python. */
static const char *PALAVRAS_RESERVADAS_CSHARP[] = {
    "abstract", "as", "base", "bool", "break", "byte", "case", "catch",
    "char", "checked", "class", "const", "continue", "decimal", "default",
    "delegate", "do", "double", "else", "enum", "event", "explicit",
    "extern", "false", "finally", "fixed", "float", "for", "foreach",
    "goto", "if", "implicit", "in", "int", "interface", "internal", "is",
    "lock", "long", "namespace", "new", "null", "object", "operator",
    "out", "override", "params", "private", "protected", "public",
    "readonly", "ref", "return", "sbyte", "sealed", "short", "sizeof",
    "stackalloc", "static", "string", "struct", "switch", "this", "throw",
    "true", "try", "typeof", "uint", "ulong", "unchecked", "unsafe",
    "ushort", "using", "virtual", "void", "volatile", "while", NULL
};

/* Prefixa com '@' nomes que colidem com palavras reservadas (ex.: @string). */
static void anexar_identificador(Buffer *b, const char *nome) {
    for (int i = 0; PALAVRAS_RESERVADAS_CSHARP[i] != NULL; i++) {
        if (strcmp(nome, PALAVRAS_RESERVADAS_CSHARP[i]) == 0) {
            buffer_anexar_char(b, '@');
            break;
        }
    }
    buffer_anexar(b, nome);
}

/* ── Literais ────────────────────────────────────────────────────── */

/*
 * Menor representação decimal que reconstrói exatamente o double
 * (3.14 -> "3.14", e não "3.1400000000000001"). Sempre contém '.' ou
 * expoente, para o C# não interpretar o literal como int (2.0 -> "2.0").
 */
static void anexar_double(Buffer *b, double valor) {
    char texto[64];
    for (int precisao = 1; precisao <= 17; precisao++) {
        snprintf(texto, sizeof(texto), "%.*g", precisao, valor);
        if (strtod(texto, NULL) == valor) break;
    }
    if (strpbrk(texto, ".eE") == NULL) strcat(texto, ".0");
    buffer_anexar(b, texto);
}

/*
 * Converte um literal de string Python (com as aspas originais, simples ou
 * duplas) em um literal C# entre aspas duplas.
 */
static void anexar_literal_string(Buffer *b, const char *literal_py) {
    size_t n = strlen(literal_py);
    buffer_anexar_char(b, '"');

    /* Ignora a aspa de abertura (índice 0) e a de fechamento (n - 1). */
    for (size_t i = 1; i + 1 < n; i++) {
        char c = literal_py[i];

        if (c == '"') {
            /* Aspa dupla solta dentro de 'texto' precisa de escape em C#. */
            buffer_anexar(b, "\\\"");
        } else if (c == '\\' && i + 2 < n) {
            char e = literal_py[++i];
            if (strchr("'\"\\abfnrtvuU", e) != NULL) {
                /* Escape com o mesmo significado nas duas linguagens. */
                buffer_anexar_char(b, '\\');
                buffer_anexar_char(b, e);
            } else if (e == 'x') {
                /* Python \xhh (2 dígitos) -> C# \u00hh (o \x do C# tem tamanho variável). */
                buffer_anexar(b, "\\u00");
            } else if (e >= '0' && e <= '7') {
                /* Escape octal do Python (até 3 dígitos) -> \uXXXX. */
                int valor = e - '0';
                for (int k = 0; k < 2 && i + 2 < n &&
                                literal_py[i + 1] >= '0' && literal_py[i + 1] <= '7'; k++) {
                    valor = valor * 8 + (literal_py[++i] - '0');
                }
                buffer_anexarf(b, "\\u%04x", valor);
            } else {
                /* Escape desconhecido: o Python mantém a barra literalmente. */
                buffer_anexar(b, "\\\\");
                buffer_anexar_char(b, e);
            }
        } else {
            buffer_anexar_char(b, c);
        }
    }

    buffer_anexar_char(b, '"');
}

/* ── Expressões ──────────────────────────────────────────────────── */

/* Precedência dos operadores em C# (maior valor = liga mais forte). */
enum {
    PREC_OR = 1,
    PREC_AND,
    PREC_IGUALDADE,
    PREC_RELACIONAL,
    PREC_ADITIVO,
    PREC_MULTIPLICATIVO,
    PREC_UNARIO,
    PREC_PRIMARIO
};

static int precedencia_operador(int op) {
    switch (op) {
        case OP_OR:   return PREC_OR;
        case OP_AND:  return PREC_AND;
        case OP_EQ:
        case OP_NEQ:  return PREC_IGUALDADE;
        case OP_GT:
        case OP_LT:
        case OP_GTE:
        case OP_LTE:  return PREC_RELACIONAL;
        case OP_SOMA:
        case OP_SUB:  return PREC_ADITIVO;
        case OP_MULT:
        case OP_DIV:  return PREC_MULTIPLICATIVO;
        default:      return PREC_PRIMARIO;
    }
}

static int precedencia_no(NoAST *no) {
    if (no == NULL) return PREC_PRIMARIO;
    if (no->tipo == NO_BINOP) return precedencia_operador(no->operador);
    if (no->tipo == NO_UNARYOP) return PREC_UNARIO;
    return PREC_PRIMARIO;
}

static const char *simbolo_operador(int op) {
    switch (op) {
        case OP_SOMA: return "+";
        case OP_SUB:  return "-";
        case OP_MULT: return "*";
        case OP_DIV:  return "/";
        case OP_EQ:   return "==";
        case OP_NEQ:  return "!=";
        case OP_GT:   return ">";
        case OP_LT:   return "<";
        case OP_GTE:  return ">=";
        case OP_LTE:  return "<=";
        case OP_AND:  return "&&";
        case OP_OR:   return "||";
        case OP_NOT:  return "!";
        case OP_NEG:  return "-";
        default:      return "?";
    }
}

static const char *nome_construcao(NoAST *no) {
    switch (no->tipo) {
        case NO_PRINT:    return "print";
        case NO_INPUT:    return "input";
        case NO_IF:       return "if";
        case NO_ELIF:     return "elif";
        case NO_WHILE:    return "while";
        case NO_FOR:      return "for";
        case NO_FUNCDEF:  return "def";
        case NO_FUNCCALL: return "chamada de função";
        case NO_RETURN:   return "return";
        case NO_BINOP:    return no->operador == OP_IN ? "operador in" : "operador";
        default:          return "construção";
    }
}

static void anexar_expressao(Buffer *b, NoAST *no, int *nao_suportados);

/*
 * Emite um operando, colocando parênteses apenas quando a precedência exige.
 * Como os operadores binários são associativos à esquerda, o operando
 * direito de mesma precedência também precisa de parênteses: a - (b - c).
 */
static void anexar_operando(Buffer *b, NoAST *filho, int prec_pai, bool lado_direito,
                            int *nao_suportados) {
    int prec = precedencia_no(filho);
    bool parenteses = prec < prec_pai || (lado_direito && prec == prec_pai);

    if (parenteses) buffer_anexar_char(b, '(');
    anexar_expressao(b, filho, nao_suportados);
    if (parenteses) buffer_anexar_char(b, ')');
}

/* Valor neutro do tipo, usado no lugar de expressões ainda não suportadas. */
static void anexar_nao_suportado(Buffer *b, NoAST *no, int *nao_suportados) {
    TipoDado tipo = no->tipo_resolvido;
    buffer_anexarf(b, "/* não traduzido: %s (linha %d) */ default(%s)",
                   nome_construcao(no), no->linha,
                   tipo == TIPO_DESCONHECIDO || tipo == TIPO_VOID ? "object"
                                                                  : codegen_csharp_tipo(tipo));
    if (nao_suportados) (*nao_suportados)++;
}

static bool e_tipo(NoAST *no, TipoDado tipo) {
    return no != NULL && no->tipo_resolvido == tipo;
}

/* Operando bool como inteiro (True = 1, False = 0), como o Python compara. */
static void anexar_bool_como_int(Buffer *b, NoAST *no, int *nao_suportados) {
    buffer_anexar_char(b, '(');
    anexar_expressao(b, no, nao_suportados);
    buffer_anexar(b, " ? 1 : 0)");
}

static void anexar_binop(Buffer *b, NoAST *no, int *nao_suportados) {
    if (no->operador == OP_IN) {
        anexar_nao_suportado(b, no, nao_suportados);
        return;
    }

    int prec = precedencia_operador(no->operador);

    /*
     * Python ordena strings e booleanos com <, >, <= e >=; em C# esses
     * operadores não existem para string nem bool (erro CS0019).
     *   "a" < "b"   -> string.CompareOrdinal("a", "b") < 0   (ordem de código, como no Python)
     *   True > False -> (true ? 1 : 0) > (false ? 1 : 0)
     * == e != funcionam nos dois tipos e seguem o caminho normal.
     */
    if (prec == PREC_RELACIONAL &&
        (e_tipo(no->esquerda, TIPO_STRING) || e_tipo(no->direita, TIPO_STRING))) {
        buffer_anexar(b, "string.CompareOrdinal(");
        anexar_expressao(b, no->esquerda, nao_suportados);
        buffer_anexar(b, ", ");
        anexar_expressao(b, no->direita, nao_suportados);
        buffer_anexarf(b, ") %s 0", simbolo_operador(no->operador));
        return;
    }
    if (prec == PREC_RELACIONAL &&
        e_tipo(no->esquerda, TIPO_BOOL) && e_tipo(no->direita, TIPO_BOOL)) {
        anexar_bool_como_int(b, no->esquerda, nao_suportados);
        buffer_anexarf(b, " %s ", simbolo_operador(no->operador));
        anexar_bool_como_int(b, no->direita, nao_suportados);
        return;
    }

    /*
     * Em Python, '/' sempre produz float; em C#, int / int trunca.
     * Converte o operando esquerdo para double, a menos que algum dos lados
     * já seja float (o cast é unário, então o operando o segue com essa
     * precedência: (double)(a + b) / c).
     */
    if (no->operador == OP_DIV &&
        (no->esquerda == NULL || no->esquerda->tipo_resolvido != TIPO_FLOAT) &&
        (no->direita == NULL || no->direita->tipo_resolvido != TIPO_FLOAT)) {
        buffer_anexar(b, "(double)");
        anexar_operando(b, no->esquerda, PREC_UNARIO, false, nao_suportados);
    } else {
        anexar_operando(b, no->esquerda, prec, false, nao_suportados);
    }

    buffer_anexarf(b, " %s ", simbolo_operador(no->operador));
    anexar_operando(b, no->direita, prec, true, nao_suportados);
}

static void anexar_unaryop(Buffer *b, NoAST *no, int *nao_suportados) {
    /*
     * Gera o operando à parte para decidir os parênteses:
     *  - 'not a == b' em Python é 'not (a == b)'; em C# '!' liga mais forte
     *    que '==', então operandos de menor precedência vão entre parênteses;
     *  - '- -x' não pode virar '--x' (decremento em C#): operando começando
     *    com '-' também vai entre parênteses.
     */
    Buffer operando;
    buffer_iniciar(&operando);
    anexar_expressao(&operando, no->esquerda, nao_suportados);

    bool parenteses = precedencia_no(no->esquerda) < PREC_UNARIO ||
                      (no->operador == OP_NEG && operando.dados[0] == '-');

    buffer_anexar(b, simbolo_operador(no->operador));
    if (parenteses) buffer_anexar_char(b, '(');
    buffer_anexar(b, operando.dados);
    if (parenteses) buffer_anexar_char(b, ')');

    free(operando.dados);
}

/*
 * Chamadas de função ainda caem no caso não suportado: como os 'def' ainda
 * não são emitidos, uma chamada traduzida referenciaria um método inexistente
 * e o arquivo deixaria de compilar. Entram junto com a tradução de funções.
 */
static void anexar_expressao(Buffer *b, NoAST *no, int *nao_suportados) {
    if (no == NULL) return;

    switch (no->tipo) {
        case NO_NUM_INT:   buffer_anexarf(b, "%d", no->valor_int);          break;
        case NO_NUM_FLOAT: anexar_double(b, no->valor_float);               break;
        case NO_STRING:    anexar_literal_string(b, no->valor_string);      break;
        case NO_BOOL:      buffer_anexar(b, no->valor_int ? "true" : "false"); break;
        case NO_ID:        anexar_identificador(b, no->nome);               break;
        case NO_BINOP:     anexar_binop(b, no, nao_suportados);             break;
        case NO_UNARYOP:   anexar_unaryop(b, no, nao_suportados);           break;
        default:           anexar_nao_suportado(b, no, nao_suportados);     break;
    }
}

char *codegen_csharp_expressao(NoAST *expr) {
    Buffer b;
    buffer_iniciar(&b);
    anexar_expressao(&b, expr, NULL);
    return b.dados;
}

/* ── Comandos ────────────────────────────────────────────────────── */

/* Nomes já declarados no método atual: só a 1ª atribuição leva o tipo. */
typedef struct {
    char **nomes;
    int    tamanho;
    int    capacidade;
} ListaNomes;

static bool lista_contem(ListaNomes *l, const char *nome) {
    for (int i = 0; i < l->tamanho; i++) {
        if (strcmp(l->nomes[i], nome) == 0) return true;
    }
    return false;
}

static void lista_adicionar(ListaNomes *l, const char *nome) {
    if (l->tamanho == l->capacidade) {
        l->capacidade = l->capacidade == 0 ? 8 : l->capacidade * 2;
        l->nomes = realloc(l->nomes, sizeof(char *) * l->capacidade);
        if (l->nomes == NULL) {
            fprintf(stderr, "Erro: falha ao alocar memória no gerador C#.\n");
            exit(1);
        }
    }
    l->nomes[l->tamanho++] = strdup(nome);
}

static void lista_liberar(ListaNomes *l) {
    for (int i = 0; i < l->tamanho; i++) free(l->nomes[i]);
    free(l->nomes);
    l->nomes = NULL;
    l->tamanho = l->capacidade = 0;
}

typedef struct {
    FILE      *saida;
    int        nao_suportados;
    ListaNomes declaradas;
} Contexto;

static void emitir_indentacao(Contexto *ctx, int nivel) {
    for (int i = 0; i < nivel; i++) fputs(INDENTACAO, ctx->saida);
}

static void emitir_nao_suportado(Contexto *ctx, NoAST *no, int nivel) {
    emitir_indentacao(ctx, nivel);
    fprintf(ctx->saida, "// [não traduzido] %s (linha %d)\n", nome_construcao(no), no->linha);
    ctx->nao_suportados++;
}

static void emitir_atribuicao(Contexto *ctx, NoAST *no, int nivel) {
    Buffer linha;
    buffer_iniciar(&linha);

    if (!lista_contem(&ctx->declaradas, no->nome)) {
        /* Declaração implícita (ADR-001): o tipo vem da expressão. */
        TipoDado tipo = no->esquerda ? no->esquerda->tipo_resolvido : TIPO_DESCONHECIDO;
        buffer_anexar(&linha, codegen_csharp_tipo(tipo));
        buffer_anexar_char(&linha, ' ');
        lista_adicionar(&ctx->declaradas, no->nome);
    }

    anexar_identificador(&linha, no->nome);
    buffer_anexar(&linha, " = ");
    anexar_expressao(&linha, no->esquerda, &ctx->nao_suportados);
    buffer_anexar_char(&linha, ';');

    emitir_indentacao(ctx, nivel);
    fprintf(ctx->saida, "%s\n", linha.dados);
    free(linha.dados);
}

/*
 * print(...) -> Console.WriteLine(...);
 * Com vários argumentos, string.Join(" ", ...) separa por espaço, como o
 * Python. bool já sai como True/False no C#.
 */
static void emitir_print(Contexto *ctx, NoAST *no, int nivel) {
    Buffer linha;
    buffer_iniciar(&linha);

    buffer_anexar(&linha, "Console.WriteLine(");
    if (no->num_argumentos > 1) buffer_anexar(&linha, "string.Join(\" \", ");
    for (int i = 0; i < no->num_argumentos; i++) {
        if (i > 0) buffer_anexar(&linha, ", ");
        anexar_expressao(&linha, no->argumentos[i], &ctx->nao_suportados);
    }
    if (no->num_argumentos > 1) buffer_anexar_char(&linha, ')');
    buffer_anexar(&linha, ");");

    emitir_indentacao(ctx, nivel);
    fprintf(ctx->saida, "%s\n", linha.dados);
    free(linha.dados);
}

static bool contem_print(NoAST *no) {
    if (no == NULL) return false;
    if (no->tipo == NO_PRINT) return true;
    if (contem_print(no->esquerda) || contem_print(no->direita) ||
        contem_print(no->condicao) || contem_print(no->bloco_if) ||
        contem_print(no->bloco_else)) return true;
    for (int i = 0; i < no->num_argumentos; i++) {
        if (contem_print(no->argumentos[i])) return true;
    }
    return false;
}

static void emitir_comando(Contexto *ctx, NoAST *no, int nivel) {
    if (no == NULL) return;

    switch (no->tipo) {
        case NO_ASSIGN:
            emitir_atribuicao(ctx, no, nivel);
            break;

        case NO_PRINT:
            emitir_print(ctx, no, nivel);
            break;

        case NO_BLOCO:
            for (int i = 0; i < no->num_argumentos; i++) {
                emitir_comando(ctx, no->argumentos[i], nivel);
            }
            break;

        default:
            emitir_nao_suportado(ctx, no, nivel);
            break;
    }
}

/* ── Estrutura do arquivo ────────────────────────────────────────── */

int codegen_csharp_gerar(NoAST *raiz, FILE *saida) {
    Contexto ctx = { saida, 0, { NULL, 0, 0 } };

    NoAST **comandos = NULL;
    int num_comandos = 0;
    if (raiz != NULL) {
        comandos = raiz->argumentos;
        num_comandos = raiz->num_argumentos;
    }

    fprintf(saida, "using System;\n\n");
    fprintf(saida, "namespace " NAMESPACE_CSHARP " {\n");
    fprintf(saida, INDENTACAO "static class Program {\n");

    /* Funções definidas pelo usuário: viram métodos static da classe. */
    bool tem_funcoes = false;
    for (int i = 0; i < num_comandos; i++) {
        if (comandos[i] != NULL && comandos[i]->tipo == NO_FUNCDEF) {
            emitir_nao_suportado(&ctx, comandos[i], 2);
            tem_funcoes = true;
        }
    }
    if (tem_funcoes) fprintf(saida, "\n");

    /* Código do nível global: vai para o Main. */
    fprintf(saida, INDENTACAO INDENTACAO "static void Main(string[] args) {\n");
    /* Ponto decimal em qualquer cultura (pt-BR imprimiria 3,14). */
    if (contem_print(raiz)) {
        fprintf(saida, INDENTACAO INDENTACAO INDENTACAO
                "System.Globalization.CultureInfo.CurrentCulture = "
                "System.Globalization.CultureInfo.InvariantCulture;\n");
    }
    for (int i = 0; i < num_comandos; i++) {
        if (comandos[i] != NULL && comandos[i]->tipo != NO_FUNCDEF) {
            emitir_comando(&ctx, comandos[i], 3);
        }
    }
    fprintf(saida, INDENTACAO INDENTACAO "}\n");
    fprintf(saida, INDENTACAO "}\n");
    fprintf(saida, "}\n");

    lista_liberar(&ctx.declaradas);
    return ctx.nao_suportados;
}
