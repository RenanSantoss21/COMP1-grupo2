# Árvore Sintática Abstrata (AST)

A Árvore Sintática Abstrata é a representação intermediária produzida pelo parser após validar a sintaxe do código-fonte. Diferentemente da árvore de derivação concreta (parse tree), a AST omite detalhes sintáticos (parênteses, dois-pontos, tokens `NEWLINE`/`INDENT`/`DEDENT`) e retém apenas a estrutura semântica do programa.

## Motivação

O parser atual apenas valida a sintaxe (aceita/rejeita), sem produzir nenhuma representação intermediária. A AST é necessária para que as fases seguintes (análise semântica e geração de código) possam trabalhar sobre uma estrutura de dados bem definida.

## Módulo `ast.h` / `ast.c`

Os arquivos ficam em `src/ast.h` e `src/ast.c`, seguindo o padrão estabelecido por `src/tabela.h` e `src/tabela.c`.

---

## Tipos de Nós (`TipoNo`)

Cada nó da AST tem um tipo que identifica a construção sintática que ele representa. Os tipos são definidos pelo enum `TipoNo`:

| Valor do Enum       | Construção Representada             | Campos Relevantes                                      |
|----------------------|--------------------------------------|--------------------------------------------------------|
| `NO_NUM_INT`         | Literal inteiro (`42`)              | `valor_int`                                            |
| `NO_NUM_FLOAT`       | Literal float (`3.14`)              | `valor_float`                                          |
| `NO_STRING`          | Literal string (`"olá"`)            | `valor_string`                                         |
| `NO_BOOL`            | Literal booleano (`True` / `False`) | `valor_int` (1 = True, 0 = False)                      |
| `NO_ID`              | Identificador (`x`, `soma`)         | `nome`                                                 |
| `NO_BINOP`           | Operação binária (`a + b`)          | `operador`, `esquerda`, `direita`                      |
| `NO_UNARYOP`         | Operação unária (`not x`, `-x`)     | `operador`, `esquerda`                                 |
| `NO_ASSIGN`          | Atribuição (`x = 10`)              | `nome`, `esquerda` (expressão do valor)                |
| `NO_PRINT`           | `print(expr)`                       | `esquerda` (expressão), `argumentos` (lista)           |
| `NO_INPUT`           | `input()`                           | `esquerda` (prompt, opcional)                          |
| `NO_IF`              | `if/elif/else`                      | `condicao`, `bloco_if`, `lista_elif`, `bloco_else`     |
| `NO_ELIF`            | Cláusula `elif`                     | `condicao`, `bloco_if` (corpo do elif)                 |
| `NO_WHILE`           | `while cond:`                       | `condicao`, `bloco_if` (corpo do while)                |
| `NO_FOR`             | `for x in range(n):`               | `nome`, `esquerda` (range_expr), `bloco_if` (corpo)    |
| `NO_FUNCDEF`         | `def nome(params):`                 | `nome`, `argumentos` (parâmetros), `bloco_if` (corpo)  |
| `NO_RETURN`          | `return expr`                       | `esquerda` (expressão, pode ser NULL)                  |
| `NO_FUNCCALL`        | `func(args)`                        | `nome`, `argumentos` (lista de argumentos)             |
| `NO_BLOCO`           | Bloco de comandos                    | `argumentos` (lista de nós-comando)                    |
| `NO_PROGRAMA`        | Raiz do programa                     | `argumentos` (lista de comandos top-level)             |

---

## Estrutura do Nó (`NoAST`)

Todos os nós compartilham uma única struct `NoAST` com campos que são usados conforme o `TipoNo`:

```c
typedef struct noAST {
    TipoNo tipo;

    /* Literais e identificadores */
    int    valor_int;
    double valor_float;
    char  *valor_string;
    char  *nome;

    /* Operador para BinOp e UnaryOp (ex: '+', '-', OP_AND, OP_NOT, etc.) */
    int operador;

    /* Filhos principais (condição, expressão) */
    struct noAST *condicao;
    struct noAST *esquerda;
    struct noAST *direita;

    /* Blocos de código */
    struct noAST *bloco_if;
    struct noAST *bloco_else;

    /* Lista encadeada (elif, argumentos, parâmetros, comandos) */
    struct noAST **argumentos;
    int num_argumentos;

    /* Informação de localização para mensagens de erro */
    int linha;
} NoAST;
```

---

## Operadores (`TipoOperador`)

Operadores binários e unários são representados por um enum separado para evitar ambiguidade com os tokens do Bison:

| Valor              | Operação  |
|--------------------|-----------|
| `OP_SOMA`          | `+`       |
| `OP_SUB`           | `-`       |
| `OP_MULT`          | `*`       |
| `OP_DIV`           | `/`       |
| `OP_EQ`            | `==`      |
| `OP_NEQ`           | `!=`      |
| `OP_GT`            | `>`       |
| `OP_LT`            | `<`       |
| `OP_GTE`           | `>=`      |
| `OP_LTE`           | `<=`      |
| `OP_AND`           | `and`     |
| `OP_OR`            | `or`      |
| `OP_NOT`           | `not`     |
| `OP_NEG`           | `-` (unário) |
| `OP_IN`            | `in`      |

---

## Funções da API

### Construtores

Cada tipo de nó possui uma função construtora que aloca o nó, inicializa seus campos e retorna o ponteiro:

| Função                                                              | Descrição                          |
|---------------------------------------------------------------------|------------------------------------|
| `criar_no_int(int valor, int linha)`                                | Cria nó `NO_NUM_INT`              |
| `criar_no_float(double valor, int linha)`                           | Cria nó `NO_NUM_FLOAT`            |
| `criar_no_string(char *valor, int linha)`                           | Cria nó `NO_STRING`               |
| `criar_no_bool(int valor, int linha)`                               | Cria nó `NO_BOOL`                 |
| `criar_no_id(char *nome, int linha)`                                | Cria nó `NO_ID`                   |
| `criar_no_binop(int op, NoAST *esq, NoAST *dir, int linha)`        | Cria nó `NO_BINOP`                |
| `criar_no_unaryop(int op, NoAST *operando, int linha)`              | Cria nó `NO_UNARYOP`              |
| `criar_no_assign(char *nome, NoAST *expr, int linha)`               | Cria nó `NO_ASSIGN`               |
| `criar_no_print(NoAST **args, int num_args, int linha)`             | Cria nó `NO_PRINT`                |
| `criar_no_input(NoAST *prompt, int linha)`                          | Cria nó `NO_INPUT`                |
| `criar_no_if(NoAST *cond, NoAST *bloco, NoAST **elifs, int n_elifs, NoAST *else_bloco, int linha)` | Cria nó `NO_IF` |
| `criar_no_elif(NoAST *cond, NoAST *bloco, int linha)`              | Cria nó `NO_ELIF`                 |
| `criar_no_while(NoAST *cond, NoAST *bloco, int linha)`             | Cria nó `NO_WHILE`                |
| `criar_no_for(char *var, NoAST *range, NoAST *bloco, int linha)`   | Cria nó `NO_FOR`                  |
| `criar_no_funcdef(char *nome, NoAST **params, int n_params, NoAST *bloco, int linha)` | Cria nó `NO_FUNCDEF` |
| `criar_no_return(NoAST *expr, int linha)`                           | Cria nó `NO_RETURN`               |
| `criar_no_funccall(char *nome, NoAST **args, int n_args, int linha)` | Cria nó `NO_FUNCCALL`           |
| `criar_no_bloco(NoAST **cmds, int n_cmds)`                         | Cria nó `NO_BLOCO`                |
| `criar_no_programa(NoAST **cmds, int n_cmds)`                      | Cria nó `NO_PROGRAMA`             |

### Utilitários

| Função                          | Descrição                                                          |
|---------------------------------|--------------------------------------------------------------------|
| `imprimir_ast(NoAST *raiz, int nivel)` | Imprime a árvore formatada com indentação para debug          |
| `liberar_ast(NoAST *raiz)`             | Libera toda a memória da árvore recursivamente                 |

---

## Exemplo de Saída de `imprimir_ast`

Para o código:
```python
x = 10 + 5
print(x)
```

A saída esperada seria:
```
Programa
  Assign: x
    BinOp: +
      NumInt: 10
      NumInt: 5
  Print
    Id: x
```

---

## Integração com o Parser

A integração da AST com o `parser.y` será feita na fase seguinte. As ações semânticas do Bison chamarão as funções `criar_no_*()` para construir a árvore durante o parsing. Isso envolverá:

1. Adicionar `#include "ast.h"` e `NoAST *raiz_ast;` ao prólogo do parser
2. Expandir o `%union` com o tipo `NoAST *ast`
3. Declarar `%type <ast>` para cada não-terminal
4. Adicionar ações `{ $$ = criar_no_*(...); }` em cada regra da gramática

---

## Referência

- Exemplo da disciplina: `Aulas/semana 06/src/ast.h` e `ast.c`
- Issue de origem: GitHub Issue #7

---

*Este módulo será criado na Sprint 3 e será pré-requisito para a análise semântica e geração de código.*
