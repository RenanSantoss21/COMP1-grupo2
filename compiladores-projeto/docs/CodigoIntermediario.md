# Geração de Código Intermediário (TAC)

O módulo `src/codegen_intermediario.c` (com cabeçalho em `src/codegen_intermediario.h`) traduz a Árvore Sintática Abstrata (AST) em **Código de Três Endereços (TAC — *Three-Address Code*)**, uma representação linear e independente de arquitetura que serve de entrada para o [Otimizador TAC](Otimizador.md).

## Uso

O compilador permite inspecionar o código intermediário diretamente pela linha de comando:

```bash
./compilador --tac < programa.py   # imprime o TAC gerado
./compilador --opt < programa.py   # imprime o TAC após otimizações
```

---

## Estruturas de Dados

O código intermediário é armazenado em um vetor dinâmico (`TAC`) composto por structs `Instrucao`:

```c
typedef struct {
    TipoInstrucao tipo;
    char *resultado;
    char *operando1;
    char *operando2;
    char *operador;
} Instrucao;

typedef struct {
    Instrucao *Instrucoes;
    int tamanho;
    int capacidade;
} TAC;
```

Os temporários são gerados sequencialmente por `novo_temporario()` (`t0`, `t1`, `t2`, ...) e os rótulos de salto por `novo_rotulo()` (`L0`, `L1`, `L2`, ...). A lista de instruções cresce dinamicamente via `realloc` (capacidade inicial 10, dobrando ao lotar) e é liberada com `liberar_tac()`, que também zera os contadores.

---

## Conjunto de Instruções (`TipoInstrucao`)

| Tipo | Formato impresso | Uso |
|---|---|---|
| `TAC_ASSIGN` | `resultado = operando1` | Atribuição simples (`x = t0`) |
| `TAC_BINOP` | `resultado = operando1 operador operando2` | Operações aritméticas, relacionais, lógicas e `in` |
| `TAC_UNOP` | `resultado = operador operando1` (ex.: `t0 = -x`) | Operadores unários (`-`, `not`) |
| `TAC_LABEL` | `resultado:` | Rótulo de destino para saltos (`L0:`) |
| `TAC_GOTO` | `GOTO resultado` | Salto incondicional |
| `TAC_IF_FALSE` | `IF_FALSE operando1 GOTO resultado` | Salto condicional quando a condição é falsa |
| `TAC_PARAM` | `PARAM operando1` | Empilha/passa argumento antes de `CALL` |
| `TAC_CALL` | `resultado = CALL operando1, operando2` (ou `CALL operando1, operando2`) | Chamada de função informando nome e número de argumentos |
| `TAC_RETURN` | `RETURN operando1` (ou `RETURN`) | Retorno de função (com ou sem valor) |
| `TAC_PRINT` | `PRINT operando1` | Emite a impressão de cada argumento de `print(...)` |
| `TAC_FUNC_BEGIN` | `FUNC_BEGIN resultado` | Marca o início do corpo de uma função |
| `TAC_FUNC_END` | `FUNC_END resultado` | Marca o fim do corpo de uma função |

---

## Tradução das Construções da AST

### Expressões e Atribuições

Expressões são traduzidas recursivamente em pós-ordem por `codegen_gerar_expressao()`: literais e identificadores retornam diretamente seu texto, enquanto operações binárias e unárias emitem uma instrução atribuída a um novo temporário.

**Python:**
```python
x = (a + b) * 2
y = -x
```

**TAC gerado:**
```text
t0 = a + b
t1 = t0 * 2
x = t1
t2 = -x
y = t2
```

### Condicionais (`if` / `elif` / `else`)

Cada ramo (`if` e cada `elif`) avalia sua condição e emite `IF_FALSE` para o rótulo do próximo ramo. Ao final de cada ramo verdadeiro, um `GOTO` salta para o rótulo final da cadeia (quando há `else`, existe um rótulo dedicado de fim após o `else`).

**Python:**
```python
if x > 0:
    y = 1
elif x < 0:
    y = 2
else:
    y = 0
```

**TAC gerado:**
```text
t0 = x > 0
IF_FALSE t0 GOTO L0
y = 1
GOTO L2
L0:
t1 = x < 0
IF_FALSE t1 GOTO L1
y = 2
GOTO L2
L1:
y = 0
L2:
```

### Laço `while`

Cria dois rótulos (`inicio` e `fim`), avalia a condição no início de cada iteração e salta de volta ao topo ao final do corpo.

**Python:**
```python
while x > 0:
    x = x - 1
```

**TAC gerado:**
```text
L0:
t0 = x > 0
IF_FALSE t0 GOTO L1
t1 = x - 1
x = t1
GOTO L0
L1:
```

### Laço `for ... in range(...)`

Suporta as três formas de `range`:
- `range(fim)` → `inicio = 0`, `passo = 1`
- `range(inicio, fim)` → `passo = 1`
- `range(inicio, fim, passo)` → usa comparador `<` para passo positivo e `>` quando `passo` é negativo (ex.: `-1`).

**Python:**
```python
for i in range(0, 10, 2):
    print(i)
```

**TAC gerado:**
```text
i = 0
L0:
t0 = i < 10
IF_FALSE t0 GOTO L1
PRINT i
t1 = i + 2
i = t1
GOTO L0
L1:
```

### Funções, Chamadas, `return` e `print`

- `def f(...)` delimita o corpo com `FUNC_BEGIN f` e `FUNC_END f`.
- `f(a, b)` avalia cada argumento emitindo `PARAM` na ordem e depois `tN = CALL f, <n_args>`.
- `return expr` emite `RETURN <valor>` (ou `RETURN` vazio quando sem expressão).
- `print(a, b)` avalia cada argumento e emite uma instrução `PRINT` por argumento.

**Python:**
```python
def dobrar(n):
    return n * 2

r = dobrar(5)
print(r)
```

**TAC gerado:**
```text
FUNC_BEGIN dobrar
t0 = n * 2
RETURN t0
FUNC_END dobrar
PARAM 5
t1 = CALL dobrar, 1
r = t1
PRINT r
```

---

## API do Módulo

Definida em `src/codegen_intermediario.h`:

| Função | Responsabilidade |
|---|---|
| `void codegen_gerar_tac(NoAST *no)` | Percorre a AST e emite as instruções TAC correspondentes. |
| `char *codegen_gerar_expressao(NoAST *no)` | Gera TAC para uma expressão e retorna o operando/temporário resultante. |
| `void codegen_gerar_atribuicao(NoAST *no)` | Gera TAC para um nó `NO_ASSIGN`. |
| `Instrucao cria_instrucao(...)` | Aloca (via `strdup`) e monta uma struct `Instrucao`. |
| `void emitir(Instrucao inst)` | Adiciona a instrução ao vetor global `lista_tac`. |
| `char *novo_temporario(void)` / `char *novo_rotulo(void)` | Gera nomes únicos `tN` e `LN`. |
| `TAC *obter_tac(void)` | Retorna ponteiro para a lista de instruções (usado pelo otimizador e testes). |
| `void imprimir_codigo_intermediario(void)` | Imprime todas as instruções emitidas em `stdout`. |
| `void liberar_tac(void)` | Libera todas as strings/vetor e reinicia contadores. |

---

## Testes

Os testes unitários estão em `testes/tdd/teste_tac.c` (12 testes cobrindo temporários, rótulos, crescimento dinâmico, expressões, `if`/`elif`/`else`, `while`, `for`/`range`, rótulos aninhados, `def`/`return`, `CALL`/`PARAM` e `print`):

```bash
make test_tac
```
