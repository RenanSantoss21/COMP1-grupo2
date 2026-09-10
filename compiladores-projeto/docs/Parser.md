# Analisador Sintático (Parser)

O analisador sintático (ou Parser) é responsável por receber a sequência de tokens produzida pelo Lexer e verificar se ela obedece às regras gramaticais da linguagem. Utilizamos a ferramenta **Bison** para definir a gramática livre de contexto.

## Visão Geral

O parser valida a estrutura do código-fonte, garantindo que os tokens estejam organizados em construções válidas da linguagem (atribuições, condicionais, laços, funções, etc.). Caso uma sequência de tokens viole alguma regra, o parser emite uma mensagem de **erro sintático**.

## Estrutura do Arquivo `parser.y`

O arquivo é dividido em três seções principais:

### 1. Declarações (Seção `%{ ... %}` e tokens)

Define a interface com o Flex e declara todos os tokens esperados:

- **Tipos de valor** (`%union`): `int ival`, `double dval`, `char *sval`
- **Tokens com valor**: `NUM_INT`, `NUM_FLOAT`, `STRING_LITERAL`
- **Tokens simples**: `ID`, operadores, delimitadores, palavras-chave, `INDENT`, `DEDENT`, `NEWLINE`

### 2. Precedência de Operadores

A tabela de precedência é definida de **menor para maior prioridade**:

| Precedência | Associatividade | Operadores |
|---|---|---|
| 1 (menor) | Direita | `ASSIGN` (`=`) |
| 2 | Esquerda | `COMMA` (`,`) |
| 3 | Esquerda | `OR` |
| 4 | Esquerda | `AND` |
| 5 | Direita | `NOT` |
| 6 | Não-associativo | `EQ`, `NEQ`, `GTE`, `LTE`, `GT`, `LT`, `IN` |
| 7 | Esquerda | `PLUS`, `MINUS` |
| 8 (maior) | Esquerda | `TIMES`, `DIVIDE` |

Além disso, são definidas regras de precedência para resolução do problema clássico do **dangling else** (`LOWER_THAN_ELSE`, `ELSE`, `ELIF`).

### 3. Regras Gramaticais

#### Programa e Comandos

```text
programa → ε | comandos
comandos → comando | comandos comando
comando  → comando_simples NEWLINE | comando_composto
```

Um programa é uma sequência de comandos. Todo **comando simples** é terminado por um
`NEWLINE`; **comandos compostos** (aqueles que abrem bloco) são terminados pelo `DEDENT`
do seu próprio bloco.

#### Blocos Indentados

```text
bloco → NEWLINE INDENT comandos DEDENT
```

O `NEWLINE` fecha a linha do cabeçalho (`if`/`while`/`for`/`def`), o `INDENT` abre o bloco
e o `DEDENT` o fecha. Como os blocos são delimitados por tokens explícitos, o problema do
**dangling else** deixa de existir: um `else` de bloco externo é sempre precedido pelos
`DEDENT` correspondentes, então o parser é determinístico (a gramática compila com **zero
conflitos** shift/reduce).

#### Comando Simples

| Construção | Regra |
|---|---|
| Expressão (inclui atribuição) | `expressao` |
| Return sem valor | `RETURN` |
| Return com valor | `RETURN expressao` |
| Print | `PRINT LPAREN argumentos_opt RPAREN` |

#### Comando Composto

| Construção | Regra |
|---|---|
| Condicional | `IF expressao COLON bloco lista_elif senao_opt` |
| Cadeia de elif | `lista_elif → ε \| lista_elif ELIF expressao COLON bloco` |
| Else opcional | `senao_opt → ε \| ELSE COLON bloco` |
| Laço while | `WHILE expressao COLON bloco` |
| Laço for | `FOR ID IN RANGE LPAREN argumentos_opt RPAREN COLON bloco` |
| Definição de função | `DEF ID LPAREN parametros_opt RPAREN COLON bloco` |

#### Parâmetros e Argumentos

```text
parametros_opt → ε | parametros
parametros     → ID | parametros COMMA ID
argumentos_opt → ε | expressao
```

A lista de argumentos é apenas uma `expressao` opcional porque a própria regra
`expressao COMMA expressao` já cobre sequências separadas por vírgula.

#### Expressão

Expressões suportam:

- **Operações aritméticas**: `+`, `-`, `*`, `/` e menos unário (`-x`)
- **Atribuição**: `=` (associativa à direita, permitindo `a = b = 0`)
- **Comparações**: `==`, `!=`, `>`, `<`, `>=`, `<=`
- **Operadores lógicos**: `and`, `or`, `not`, `in`
- **Agrupamento**: `( )`
- **Listas**: `[ ]` (vazia) e `[ expressao ]`
- **Chamada de função**: `ID LPAREN argumentos_opt RPAREN`
- **Entrada nativa**: `INPUT LPAREN argumentos_opt RPAREN`
- **Literais**: `NUM_INT`, `NUM_FLOAT`, `STRING_LITERAL`, `TRUE`, `FALSE`
- **Identificadores**: `ID`

## Tratamento de Erros

Quando uma sequência de tokens não corresponde a nenhuma regra da gramática, a função `yyerror()` é chamada, imprimindo:

```
Erro sintático na linha <N>: <mensagem>
```

Erros de indentação são detectados ainda na fase léxica:

```
Erro léxico: indentação inconsistente na linha <N>
```

## Limitações Atuais

- **Sem AST**: O parser apenas valida a sintaxe; ainda não constrói uma Árvore Sintática Abstrata (AST).
- **`for` restrito a `range`**: A regra do `for` exige `in range(...)`; iteração direta sobre listas ainda não é suportada.
- **Sem análise semântica**: Não há verificação de tipos, escopo ou de variáveis/funções não declaradas.

---

*A gramática será expandida nas próximas sprints para incluir a construção da AST e a análise semântica.*
