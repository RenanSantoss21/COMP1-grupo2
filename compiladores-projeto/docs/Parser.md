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
programa → comandos
comandos → comando | comandos comando
```

O programa é uma sequência de um ou mais comandos.

#### Comando

Um comando pode ser qualquer uma das seguintes construções:

| Construção | Regra |
|---|---|
| Expressão simples | `expressao` |
| Condicional if | `IF expressao COLON comando` |
| Bloco else | `ELSE COLON comando` |
| Bloco elif | `ELIF expressao COLON comando` |
| Laço while | `WHILE expressao COLON comando` |
| Laço for | `FOR ID IN RANGE LPAREN expressao RPAREN COLON comando` |
| Print | `PRINT LPAREN expressao RPAREN` |
| Return | `RETURN expressao` |
| Definição de função | `DEF ID LPAREN RPAREN COLON comando` |

#### Expressão

Expressões suportam:

- **Operações aritméticas**: `+`, `-`, `*`, `/`
- **Atribuição**: `=`
- **Comparações**: `==`, `!=`, `>`, `<`, `>=`, `<=`
- **Operadores lógicos**: `and`, `or`, `not`, `in`
- **Agrupamento**: `( )` e `[ ]`
- **Chamadas nativas**: `input()`
- **Literais**: `NUM_INT`, `NUM_FLOAT`, `STRING_LITERAL`, `TRUE`, `FALSE`
- **Identificadores**: `ID`
- **Listas com vírgula**: `expressao COMMA expressao`

## Tratamento de Erros

Quando uma sequência de tokens não corresponde a nenhuma regra da gramática, a função `yyerror()` é chamada, imprimindo:

```
Erro sintático: <mensagem>
```

## Limitações Atuais

- **Funções sem parâmetros**: A regra `DEF ID LPAREN RPAREN` não suporta lista de parâmetros.
- **Sem blocos com INDENT/DEDENT**: Os tokens `INDENT`, `DEDENT` e `NEWLINE` são declarados mas ainda não são utilizados nas regras gramaticais para definir blocos de código.
- **Sem AST**: O parser apenas valida a sintaxe; ainda não constrói uma Árvore Sintática Abstrata (AST).

---

*A gramática será expandida nas próximas sprints para incluir parâmetros de função, blocos indentados e construção da AST.*
