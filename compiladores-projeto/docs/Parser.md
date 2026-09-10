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
Erro sintático na linha <N>: unexpected <TOKEN>, expecting <TOKENS>
```

A mensagem vem do modo `%define parse.error verbose` do Bison, que informa o token
encontrado e (quando são poucos) os tokens esperados; `%define parse.lac full` garante que
essa lista esteja correta. Exemplo: `def soma` sem parênteses gera
`Erro sintático na linha 3: unexpected NEWLINE, expecting LPAREN`.

### Número da linha

A linha informada é a do token que causou o erro (variável `linha_token`, mantida pelo
lexer via `YY_USER_ACTION`). Não usamos `linha_atual` nem `yylineno` porque o token
`NEWLINE` consome a quebra de linha e a indentação da linha seguinte: quando o parser
reclama de um `NEWLINE` (o caso mais comum, ex.: `if` sem `:`), esses contadores já
apontam para a linha de baixo.

### Recuperação de Erros

O parser **não aborta no primeiro erro**: ele se recupera e continua a análise, reportando
todos os erros em uma única execução. Para isso a gramática usa o token especial `error`
do Bison:

```text
comando          → ... | erro_linha
                       | erro_linha INDENT comandos DEDENT
                       | erro_indentacao comandos DEDENT
erro_linha       → error NEWLINE      { yyerrok; yyclearin; }
erro_indentacao  → error INDENT       { yyerrok; yyclearin; }
```

Ao detectar um erro, o Bison desempilha estados até um ponto onde um comando pode começar,
empilha `error` e descarta tokens até encontrar um que continue uma dessas regras:

| Situação | Regra usada | O que acontece |
|---|---|---|
| Erro em um comando simples (`y = 1 +`) | `erro_linha` | descarta o resto da linha e segue na próxima |
| Erro no cabeçalho de um bloco (`if x > 1` sem `:`) | `erro_linha INDENT comandos DEDENT` | descarta o cabeçalho, mas o corpo do bloco é analisado normalmente |
| Bloco indentado sem cabeçalho (`INDENT` inesperado) | `erro_indentacao comandos DEDENT` | o conteúdo do bloco é analisado normalmente |

- `yyerrok` sai do modo de recuperação imediatamente, para que o próximo erro também seja
  reportado (sem ele o Bison silencia erros nos 3 tokens seguintes).
- `yyclearin` descarta o token de lookahead, se houver, para recomeçar com um token novo.

Não há regra `error DEDENT`: o lexer sempre emite o `NEWLINE` antes dos `DEDENT` de uma
linha, então a sincronização por `NEWLINE` já cobre o fim dos blocos sem consumir o
`DEDENT` que fecha o bloco externo.

Ao final, se houve algum erro, é impresso um resumo e o compilador sai com código 1
(com a recuperação, `yyparse()` pode retornar sucesso mesmo após erros, então o código de
saída é decidido pelo contador `erros_sintaticos`):

```
Erro sintático na linha 8: unexpected NEWLINE
Erro sintático na linha 12: unexpected NEWLINE, expecting LPAREN
Erro sintático na linha 16: unexpected NUM_INT, expecting IN
...
5 erro(s) sintático(s) encontrado(s).
```

O único caso sem recuperação é o fim do arquivo no meio de uma construção (ex.: `if x:`
na última linha, sem corpo): não há mais tokens para sincronizar, então o erro é reportado
e a análise termina.

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
