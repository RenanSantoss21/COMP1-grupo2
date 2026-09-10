# Relatório de Testes

## Visão Geral

Este documento descreve os testes criados para validar o compilador. Eles estão divididos em dois grupos:

- **Testes léxicos** (`Testes1.py` a `Testes5.py`): cobrem o reconhecimento de tokens.
- **Testes sintáticos** (`TestesParser*.py`): cobrem as construções da gramática do parser.

São 15 arquivos no total — 9 com código válido (que devem ser aceitos) e 6 com código
inválido (que devem ser rejeitados). Um script de automação (`rodar_testes.sh`) executa
todos e exibe o resultado, retornando código de saída diferente de zero se algum falhar.
Dois dos arquivos inválidos também são usados para verificar a **recuperação de erros**
(todos os erros reportados em uma única execução), totalizando 17 verificações.

---

## Parte 1 — Testes Léxicos

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

---

## Parte 2 — Testes Sintáticos

Validam a gramática do parser: terminação de comandos por `NEWLINE`, blocos delimitados
por `INDENT`/`DEDENT`, cadeias `if/elif/else`, laços, funções com parâmetros e chamadas.

### TestesParser1.py — Atribuições e Expressões

| Construção testada | Exemplo no arquivo |
|---|---|
| Atribuições simples em linhas separadas | `x = 1`, `y = 2`, `z = 3` |
| Atribuição múltipla encadeada | `a = b = c = 0` |
| Literais de todos os tipos | `"Beatriz"`, `'bia'`, `3.14`, `1e10`, `True`, `False` |
| Operadores aritméticos | `x + y`, `x - y`, `x * y`, `x / y` |
| Menos unário | `-x` |
| Precedência e agrupamento | `x + y * z`, `(x + y) * (z - x) / y` |
| Listas (vazia e com elementos) | `[]`, `[1, 2, 3]`, `[1, "dois", 3.0, True]` |
| Operadores relacionais e lógicos | `x == y`, `x >= y`, `True and False`, `not True`, `x in lista` |
| Entrada nativa | `input()` |

**Resultado esperado:** aceito sem erros.

### TestesParser2.py — Condicionais com Blocos Indentados

| Construção testada | Exemplo no arquivo |
|---|---|
| `if` isolado | `if x > y:` |
| `if`/`else` | `else:` com bloco indentado |
| Cadeia de vários `elif` | `elif x == 2:`, `elif x == 3:` |
| Bloco com múltiplos comandos | três comandos dentro do mesmo `if` |
| Condição composta | `if x > 0 and y > 0:` |
| `if` aninhado com `else` no bloco **interno** | dois níveis de indentação |
| `if` aninhado com `else` no bloco **externo** | testa o dangling else |

**Resultado esperado:** aceito sem erros.

### TestesParser3.py — Laços com Blocos Indentados

| Construção testada | Exemplo no arquivo |
|---|---|
| `while` simples e com corpo múltiplo | `while x > 0:` |
| `while` com condição composta | `while x > 0 and total < 100:` |
| `for` com `range` de 1, 2 e 3 argumentos | `range(10)`, `range(1, 10)`, `range(0, 20, 2)` |
| `range` com expressão | `range(x + 1)` |
| `while` dentro de `while` | dois níveis |
| `for` dentro de `for` | dois níveis |
| `for` dentro de `while` | mistura de laços |
| `if`/`else` dentro de `while` | condicional dentro de laço |

**Resultado esperado:** aceito sem erros.

### TestesParser4.py — Funções com Parâmetros e Chamadas

| Construção testada | Exemplo no arquivo |
|---|---|
| `def` sem parâmetros | `def sem_parametros():` |
| `def` com 1, 2 e 3 parâmetros | `def tres_parametros(a, b, c):` |
| `return` com valor e `return` vazio | `return soma`, `return` |
| Corpo com vários comandos | função com 5 comandos |
| Chamada como comando | `dois_parametros(1, 2)` |
| Chamada em atribuição | `r3 = dois_parametros(r1, r2)` |
| Chamada com expressões como argumento | `tres_parametros(1 + 1, 2 * 2, 3)` |
| Chamadas aninhadas | `dois_parametros(um_parametro(1), um_parametro(2))` |
| Chamada dentro de `if`, `while`, `for` e `print` | `print(chama_outra(1, 2))` |

**Resultado esperado:** aceito sem erros.

### TestesParser5.py — Estruturas Aninhadas Complexas

| Cenário testado | Descrição |
|---|---|
| `if`/`elif`/`else` dentro de `for` dentro de `def` | três níveis de indentação |
| `if` dentro de `if` dentro de `for` dentro de `while` dentro de `def` | quatro níveis |
| `for` → `while` → `if` dentro de função | fechamento de múltiplos `DEDENT` seguidos |
| `return` em cada ramo de um `if`/`elif`/`else` | dentro de função |
| Chamadas de função dentro de blocos aninhados no nível global | `print(processa(i))` |

**Resultado esperado:** aceito sem erros.

### TestesParserErro1.py a TestesParserErro5.py — Erros Sintáticos

O `TestesParserErro1.py` reúne os cinco erros exigidos pela issue. Com a **recuperação de
erros** o parser continua após cada erro e reporta todos eles em uma única execução; os
arquivos `TestesParserErro2.py` a `TestesParserErro5.py` isolam cada caso individualmente.

| Arquivo | Erro testado | Exemplo | Mensagem |
|---|---|---|---|
| Erro1 | os cinco erros abaixo juntos | — | 5 erros sintáticos (linhas 8, 12, 16, 25 e 26) + 1 léxico (linha 22) |
| Erro2 | `def` sem `():` | `def soma` | `Erro sintático na linha 3: unexpected NEWLINE, expecting LPAREN` |
| Erro3 | `for` sem `in range` | `for i in 10:` | `Erro sintático na linha 3: unexpected NUM_INT, expecting RANGE` |
| Erro4 | Indentação inconsistente | bloco com 4 e depois 2 espaços | `Erro léxico: indentação inconsistente na linha 6` |
| Erro5 | Parênteses não balanceados | `print(x` | `Erro sintático na linha 4: unexpected NEWLINE` |

**Resultado esperado:** erro detectado em todos.

### Recuperação de Erros

Além de detectar o erro, a suíte verifica que o parser **não aborta no primeiro**: conta as
linhas `Erro sintático` da saída e compara com o número de erros do arquivo.

| Arquivo | Erros sintáticos esperados |
|---|---|
| `TestesParserErro1.py` | 5 |
| `Testes5.py` | 9 (um por linha inválida; o `@` da linha 1 é erro léxico e também deixa `x =` incompleto) |

**Resultado esperado:** todos os erros reportados em uma única execução.

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

--- Testes LEXICOS que devem PASSAR (codigo valido) ---
[OK]    Testes1.py - aceito com sucesso
[OK]    Testes2.py - aceito com sucesso
[OK]    Testes3.py - aceito com sucesso
[OK]    Testes4.py - aceito com sucesso

--- Testes LEXICOS que devem FALHAR (codigo invalido) ---
[OK]    Testes5.py - erro detectado como esperado

--- Testes SINTATICOS que devem PASSAR (codigo valido) ---
[OK]    TestesParser1.py - aceito com sucesso
[OK]    TestesParser2.py - aceito com sucesso
[OK]    TestesParser3.py - aceito com sucesso
[OK]    TestesParser4.py - aceito com sucesso
[OK]    TestesParser5.py - aceito com sucesso

--- Testes SINTATICOS que devem FALHAR (codigo invalido) ---
[OK]    TestesParserErro1.py - erro detectado como esperado
[OK]    TestesParserErro2.py - erro detectado como esperado
[OK]    TestesParserErro3.py - erro detectado como esperado
[OK]    TestesParserErro4.py - erro detectado como esperado
[OK]    TestesParserErro5.py - erro detectado como esperado

--- RECUPERACAO de erros (todos os erros devem ser reportados) ---
[OK]    TestesParserErro1.py - 5 erros sintaticos reportados
[OK]    Testes5.py - 9 erros sintaticos reportados

=========================================
   Resultado: 17 passaram, 0 falharam
=========================================
```
