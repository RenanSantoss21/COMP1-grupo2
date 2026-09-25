# Relatório de Testes Léxicos

## Visão Geral

Este documento descreve os testes criados para validar o analisador léxico do compilador. Os testes estão organizados em 5 arquivos, sendo 4 com código válido (que devem ser aceitos) e 1 com código inválido (que deve ser rejeitado). Um script de automação (`rodar_testes.sh`) executa todos os testes e exibe o resultado.

---

## Testes1.py — Literais, Identificadores e Operadores Básicos

Cobre os tokens mais fundamentais da linguagem.

| Token testado | Exemplo no arquivo |
|---|---|
| `NUM_INT` | `0`, `42`, `999999` |
| `NUM_FLOAT` | `3.14`, `.5`, `2.0`, `1e10`, `3.14e2`, `.5e-3` |
| `STRING_LITERAL` (aspas duplas) | `"hello world"`, `""`, `"linha\nnova"` |
| `STRING_LITERAL` (aspas simples) | `'hello world'` |
| `ID` | `x`, `y`, `nome`, `sobrenome`, `resultado` |
| `ASSIGN` | `=` |
| `PLUS` | `+` |
| `MINUS` | `-` |
| `TIMES` | `*` |
| `DIVIDE` | `/` |
| `LPAREN`, `RPAREN` | `(x + y)` |
| `TRUE`, `FALSE` | `True`, `False` |
| `NEWLINE` | quebra de linha entre comandos |

**Resultado esperado:** aceito sem erros.

---

## Testes2.py — Estruturas de Controle e Operadores Relacionais/Lógicos

Testa palavras reservadas de controle de fluxo e todos os operadores de comparação e lógica.

| Token testado | Exemplo no arquivo |
|---|---|
| `IF` | `if x == 10:` |
| `ELSE` | `else:` |
| `ELIF` | `elif x < 3:` |
| `WHILE` | `while x >= 1:` |
| `EQ` | `==` |
| `NEQ` | `!=` |
| `GT` | `>` |
| `LT` | `<` |
| `GTE` | `>=` |
| `LTE` | `<=` |
| `AND` | `True and False` |
| `OR` | `True or False` |
| `NOT` | `not False` |
| `COLON` | `:` |
| `INDENT`, `DEDENT` | blocos indentados após `:` |

**Resultado esperado:** aceito sem erros.

---

## Testes3.py — Funções, Laço For e Entrada/Saída

Testa definição de funções, laço for com range, e as funções nativas.

| Token testado | Exemplo no arquivo |
|---|---|
| `FOR` | `for i in range(10):` |
| `IN` | `in` |
| `RANGE` | `range(10)` |
| `DEF` | `def foo():` |
| `RETURN` | `return 1` |
| `PRINT` | `print("hello")` |
| `INPUT` | `input()` |
| `LCOLCH`, `RCOLCH` | `[1, 2, 3]` |
| `COMMA` | `,` |

**Resultado esperado:** aceito sem erros.

---

## Testes4.py — Blocos Aninhados

Testa a capacidade do lexer de emitir múltiplos tokens INDENT e DEDENT corretamente em estruturas aninhadas.

| Cenário testado | Exemplo no arquivo |
|---|---|
| `if` dentro de `if` | dois níveis de indentação |
| `if` dentro de `while` | mistura de estruturas |
| `for` dentro de `for` | laços aninhados |
| `if` dentro de `def` | bloco condicional dentro de função |

**Resultado esperado:** aceito sem erros.

---

## Testes5.py — Código Inválido (Erros Esperados)

Testa que o compilador rejeita corretamente entradas com erros léxicos e sintáticos.

| Erro testado | Linha |
|---|---|
| Caractere inválido `@` | `x = @` |
| Operador sem operando à direita | `y = 1 +` |
| Atribuição sem lado esquerdo | `= 5` |
| Identificador começando com número | `10abc = 1` |
| Operadores duplicados | `x == == y` |
| `if` sem expressão | `if:` |
| `def` sem nome de função | `def ()` |
| `while` sem expressão | `while:` |
| Parêntese não fechado | `print(` |

**Resultado esperado:** erro detectado.

---

## Como Executar

```bash
make clean && make
./testes/rodar_testes.sh
```

## Resultado Obtido

```
=========================================
   Rodando testes do compilador
=========================================

--- Testes que devem PASSAR (codigo valido) ---
[OK]    Testes1.py - aceito com sucesso
[OK]    Testes2.py - aceito com sucesso
[OK]    Testes3.py - aceito com sucesso
[OK]    Testes4.py - aceito com sucesso

--- Testes que devem FALHAR (codigo invalido) ---
[OK]    Testes5.py - erro detectado como esperado

=========================================
   Resultado: 5 passaram, 0 falharam
=========================================
```
