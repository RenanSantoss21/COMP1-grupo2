# Relatório de Testes Sintáticos

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
