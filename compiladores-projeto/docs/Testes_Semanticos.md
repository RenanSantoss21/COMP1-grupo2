# Relatório de Testes Semânticos

Validam a análise semântica do compilador: verificação de variáveis declaradas, gerenciamento
de escopos, inferência e verificação de tipos, validação de funções (parâmetros, retorno, chamadas)
e restrições de contexto de execução.

Os testes estão divididos em duas categorias: **testes unitários (TDD)** que exercitam a API
`analisar_semantica()` diretamente com ASTs construídas em C, e **testes de integração** que
passam código Python real pelo compilador completo (lexer → parser → AST → semântica).

---

## Parte 1 — Testes Unitários (TDD)

Os testes ficam em `testes/tdd/teste_semantica.c` e são compilados/executados com:

```bash
make test_semantica
```

Cada teste constrói uma árvore AST manualmente, invoca `analisar_semantica()` e verifica
o número de erros retornado e/ou os tipos resolvidos nos nós.

---

### test_infraestrutura — Literal básico

Verifica que a infraestrutura mínima funciona: um nó literal inteiro sozinho não deve gerar erros.

| Cenário | AST construída | Erros esperados |
|---|---|---|
| Literal inteiro isolado | `criar_no_int(42, 1)` | 0 |

---

### test_declaracao_vs_uso — Declaração e uso de variáveis

Testa os dois caminhos fundamentais da verificação de variáveis.

| Cenário | AST construída | Erros esperados |
|---|---|---|
| Uso de variável `y` **sem** declaração prévia | `criar_no_id("y", 1)` | 1 |
| Declaração `x = 10` seguida de uso de `x` | `NO_PROGRAMA` com `[NO_ASSIGN(x, 10), NO_ID(x)]` | 0 |

---

### test_escopos_aninhados — Visibilidade entre escopos

Valida que escopos internos enxergam variáveis do pai, mas variáveis internas não vazam para fora.

| Cenário | AST construída | Erros esperados |
|---|---|---|
| Variável `x` declarada fora, usada dentro de um `NO_BLOCO` | `[NO_ASSIGN(x), NO_BLOCO([NO_ID(x)])]` | 0 |
| Variável `y` declarada dentro de `NO_BLOCO`, usada **fora** dele | `[NO_BLOCO([NO_ASSIGN(y)]), NO_ID(y)]` | 1 |

---

### test_inferencia_e_operacoes — Tipos e operações aritméticas

Testa a inferência de tipos e a detecção de operações com tipos incompatíveis.

| Cenário | AST construída | Erros esperados | Tipo resolvido |
|---|---|---|---|
| Promoção int→float: `1 + 2.5` | `NO_BINOP(OP_SOMA, NO_NUM_INT(1), NO_NUM_FLOAT(2.5))` | 0 | `TIPO_FLOAT` |
| Concatenação de strings: `"a" + "b"` | `NO_BINOP(OP_SOMA, NO_STRING("a"), NO_STRING("b"))` | 0 | `TIPO_STRING` |
| Tipo incompatível: `"a" + 1` | `NO_BINOP(OP_SOMA, NO_STRING("a"), NO_NUM_INT(1))` | 1 | `TIPO_DESCONHECIDO` |

---

### test_funcoes_e_contexto — Funções, chamadas e return

Testa a validação de funções: quantidade de argumentos nas chamadas e restrição de `return` fora de função.

| Cenário | AST construída | Erros esperados |
|---|---|---|
| `def dobrar(valor)` chamada com `dobrar(10)` — argumentos corretos | `[NO_FUNCDEF(dobrar, [valor]), NO_ASSIGN(resultado, NO_FUNCCALL(dobrar, [10]))]` | 0 |
| `def dobrar(valor)` chamada com `dobrar()` — falta 1 argumento | `[NO_FUNCDEF(dobrar, [valor]), NO_ASSIGN(resultado, NO_FUNCCALL(dobrar, []))]` | 1 |
| `return 1` no nível global (fora de função) | `NO_RETURN(NO_NUM_INT(1))` | 1 |

---

## Parte 2 — Testes de Integração

Os testes de integração passam código Python pelo compilador completo. Estão na pasta `testes/`
e são executados pelo script `rodar_testes.sh`.

---

### TestesSemanticaValidos1.py — Atribuição e uso correto de variáveis

Código Python:
```python
# Atribuicao e uso correto de variaveis
idade = 20
resultado = idade + 5
```

| Verificação semântica exercitada | Detalhe |
|---|---|
| Inserção de variável na tabela de símbolos | `idade` inserida como `TIPO_INT` na atribuição |
| Uso de variável previamente declarada | `idade` é buscada na tabela ao ser usada na expressão |
| Inferência de tipo em operação aritmética | `idade(int) + 5(int)` → resultado é `TIPO_INT` |

**Resultado esperado:** aceito sem erros.

---

### TestesSemanticaValidos2.py — Promoção de tipos e concatenação de strings

Código Python:
```python
# Operacoes compativeis e promocao int para float
inteiro = 10
fracao = 2.5
soma = inteiro + fracao
mensagem = "ola" + " mundo"
```

| Verificação semântica exercitada | Detalhe |
|---|---|
| Promoção automática int→float | `inteiro(int) + fracao(float)` → resultado promovido para `TIPO_FLOAT` |
| Concatenação de strings com `+` | `"ola"(string) + " mundo"(string)` → resultado é `TIPO_STRING` |
| Atualização dinâmica de tipo na tabela | `soma` recebe `TIPO_FLOAT`, `mensagem` recebe `TIPO_STRING` |

**Resultado esperado:** aceito sem erros.

---

### TestesSemanticaValidos3.py — Chamada de função com argumentos corretos

Código Python:
```python
# Chamada de funcao com a quantidade correta de argumentos
def identidade(valor):
    return valor

resultado = identidade(10)
```

| Verificação semântica exercitada | Detalhe |
|---|---|
| Registro de função na tabela de símbolos | `identidade` registrada com 1 parâmetro (`valor`) |
| Validação de quantidade de argumentos | `identidade(10)` — 1 argumento corresponde a 1 parâmetro |
| Contexto de `return` dentro de função | `return valor` dentro de `def` — contexto válido |

**Resultado esperado:** aceito sem erros.

---

### TestesSemanticaErros1.py — Variável não declarada

Código Python:
```python
# Uso de variavel nao declarada
resultado = valor_inexistente + 1
```

| Erro esperado | Detalhe |
|---|---|
| Variável `valor_inexistente` não declarada | Busca na tabela falha → `Erro semântico na linha X: Variável 'valor_inexistente' não declarada.` |

**Resultado esperado:** erro detectado.

---

### TestesSemanticaErros2.py — Operação com tipos incompatíveis

Código Python:
```python
# Operacao com tipos incompativeis
texto = "hello"
resultado = texto + 5
```

| Erro esperado | Detalhe |
|---|---|
| Soma de string com inteiro | `texto(string) + 5(int)` → `Erro semântico na linha X: Operação aritmética com tipos incompatíveis.` |

**Resultado esperado:** erro detectado.

---

### TestesSemanticaErros3.py — Argumentos errados e return fora de função

Código Python:
```python
# Funcao chamada com argumentos errados e return fora de funcao
def soma(valor):
    return valor

resultado = soma()
return 1
```

| Erro esperado | Detalhe |
|---|---|
| Chamada com número errado de argumentos | `soma()` chamada com 0 args, esperava 1 → erro de contagem de parâmetros |
| `return` fora de função | `return 1` no nível global → `Erro semântico... 'return' fora de função gera erro.` |

**Resultado esperado:** erro detectado.

---

## Como Executar

```bash
make clean && make
./testes/rodar_testes.sh
```

Para rodar apenas os testes unitários da semântica:
```bash
make test_semantica
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

--- Testes SEMANTICOS que devem PASSAR (codigo valido) ---
[OK]    TestesSemanticaValidos1.py - aceito com sucesso
[OK]    TestesSemanticaValidos2.py - aceito com sucesso
[OK]    TestesSemanticaValidos3.py - aceito com sucesso

--- Testes SEMANTICOS que devem FALHAR (codigo invalido) ---
[OK]    TestesSemanticaErros1.py - erro detectado como esperado
[OK]    TestesSemanticaErros2.py - erro detectado como esperado
[OK]    TestesSemanticaErros3.py - erro detectado como esperado

--- RECUPERACAO de erros (todos os erros devem ser reportados) ---
[OK]    TestesParserErro1.py - 5 erros sintaticos reportados
[OK]    Testes5.py - 9 erros sintaticos reportados

=========================================
   Resultado: 23 passaram, 0 falharam
=========================================
```
