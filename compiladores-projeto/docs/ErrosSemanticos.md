# Catálogo de Erros Semânticos

Este documento descreve todos os erros semânticos que o compilador pode emitir, com exemplos de código que os disparam, a mensagem exata de saída e o comportamento esperado do analisador.

Todos os erros seguem o formato:
```
Erro semântico na linha <N>: <mensagem>.
```

---

## SEM-01 — Variável não declarada

**Mensagem:**
```
Erro semântico na linha <N>: Variável '<nome>' não declarada.
```

**Onde é gerado:** [`src/semantica.c`](../src/semantica.c), função `inferir_tipo` (caso `NO_ID`, linha 28) e função `analisar_semantica` (caso `NO_ID`, linha 199).

**Descrição:**  
Ocorre quando o compilador encontra o uso de um identificador que ainda não foi atribuído (declarado) no escopo atual ou em qualquer escopo pai. Como o Mini-Python adota **declaração implícita por atribuição** (`x = 10` registra `x` na tabela de símbolos), qualquer leitura de uma variável antes de sua primeira atribuição é considerada um erro.

**Código que dispara o erro:**


```python
# Uso de uma variável não declarada
resultado = valor_inexistente + 1
```

```python
# Uso antes de qualquer atribuição
print(x)

```

```python
# Erro de digitação no nome da variável
total = 10
print(totla)
```

```python
# Variável local de função usada fora do seu escopo
def calcular():
    resultado = 42

print(resultado)
```

**Saída do compilador:**
```
Erro semântico na linha 2: Variável 'valor_inexistente' não declarada.
```

**Código que NÃO dispara o erro (válido):**

```python
x = 10
print(x)
```

**Comportamento do analisador:**  
Após emitir o erro, o analisador continua a análise para reportar outros possíveis erros no mesmo arquivo. O nó `NO_ID` problemático recebe o tipo `TIPO_DESCONHECIDO`, que sinaliza para as validações posteriores que a origem do valor é inválida.

---

## SEM-02 — Operação aritmética com tipos incompatíveis.

**mensagem:**
```
Erro semântico na linha <N>: Operação aritmética com tipos incompatíveis.

```
**Onde é gerado:**
 [`src/semantica.c`](../src/semantica.c), função `inferir_tipo` (caso `NO_BINOP`, linha 45).

 **Descrição:**  
Ocorre quando o compilador identifica uma operação aritmética entre tipos incompatíveis. Operações envolvendo valores numéricos (`int` e `float`) são permitidas, inclusive entre tipos diferentes. Já operações aritméticas envolvendo `string` ou `bool` são inválidas, com exceção da concatenação de duas `string` usando o operador `+`.

**Código que dispara o erro:**

```python
# Uso de tipos incompatíveis
texto = "hello"
resultado = texto + 5
```

```python
# input retorna string: somar com inteiro é inválido
n = input("n")
m = n + 1

```
**Saída do compilador:**
```
Erro semântico na linha 3: Operação aritmética com tipos incompatíveis.
```
**Código que NÃO dispara o erro (válido):**

```python
texto = 10
resultado = texto + 5
```

**Comportamento do analisador:**  
Após emitir o erro, o analisador continua a análise para reportar outros possíveis erros no mesmo arquivo. O nó `NO_BINOP` problemático recebe o tipo `TIPO_DESCONHECIDO`, que sinaliza para as validações posteriores que a origem do valor é inválida.

---
## SEM-03 — Comparação inválida envolvendo string.

**mensagem:**
```
Erro semântico na linha <N>: Comparação inválida envolvendo string.

```
**Onde é gerado:**
 [`src/semantica.c`](../src/semantica.c), função `inferir_tipo` (caso `NO_BINOP`, linha 67).

 **Descrição:**  
Ocorre quando o compilador identifica a utilização de operadores de comparação (`<`, `>`, `<=`, `>=`) entre uma `string` e outro tipo de dado, como `int` ou `float`.

**Código que dispara o erro:**

```python
# Uma comparação incompatível
idade = "20"
if idade >= 16:
    print("pode votar")
```

**Saída do compilador:**
```
Erro semântico na linha 3: Comparação inválida envolvendo string.
```

**Código que NÃO dispara o erro (válido):**

```python
idade = 20
if idade >= 16:
    print("pode votar")
```

**Comportamento do analisador:**  
Após emitir o erro, o analisador continua a análise para reportar outros possíveis erros no mesmo arquivo. O nó `NO_BINOP` recebe o tipo `TIPO_BOOL`, pois toda comparação produz um resultado booleano.

---
## SEM-04 — Comparação inválida envolvendo booleano.

**mensagem:**
```
Erro semântico na linha <N>: Comparação inválida envolvendo booleano.

```
**Onde é gerado:**
 [`src/semantica.c`](../src/semantica.c), função `inferir_tipo` (caso `NO_BINOP`, linha 72).

 **Descrição:**  
 Ocorre quando o compilador identifica a utilização de operadores de comparação (`<`, `>`, `<=`, `>=` etc.) entre um valor booleano (`True` ou `False`) e outro tipo de dado, como `string`.

**Código que dispara o erro:**

```python
# Uma comparação incompatível
print(True > "1") 
```

**Saída do compilador:**
```
Erro semântico na linha 2: Comparação inválida envolvendo booleano.
```

**Código que NÃO dispara o erro (válido):**

```python
print(True == False)
print(True != False)
```

**Comportamento do analisador:**  
Após emitir o erro, o analisador continua a análise para reportar outros possíveis erros no mesmo arquivo. O nó `NO_BINOP` recebe o tipo `TIPO_BOOL`, pois toda comparação produz um resultado booleano.

---
## SEM-05 — Operador lógico exige booleanos.

**mensagem:**
```
Erro semântico na linha <N>: Operador lógico exige booleanos.

```
**Onde é gerado:**
 [`src/semantica.c`](../src/semantica.c), função   `inferir_tipo` (caso `NO_BINOP`, linha 80).

 **Descrição:**  
Ocorre quando o compilador identifica a utilização dos operadores lógicos `and` ou `or` com pelo menos um operando que não é do tipo `bool`.

**Código que dispara o erro:**

```python
idade = 18
resultado = idade and True

```

**Saída do compilador:**
```
Erro semântico na linha 2: Operador lógico exige booleanos.

```

**Código que NÃO dispara o erro (válido):**

```python
idade = 18
maior_de_idade = idade >= 18
resultado = True and maior_de_idade
```

**Comportamento do analisador:**  
Após inferir os tipos dos dois operandos, o analisador verifica se ambos são do tipo `bool`. Se algum tipo for diferente, emite a mensagem de erro e incrementa o contador de erros sem interromper a análise. Em seguida, define o resultado da operação como `bool` e continua a análise do restante do programa.

---
## SEM-06 — Operador '-' exige operando numérico.

**mensagem:**
```
Erro semântico na linha <N>: Operador '-' exige operando numérico.

```
**Onde é gerado:**
 [`src/semantica.c`](../src/semantica.c), função `inferir_tipo` (caso `NO_UNARYOP`, linha 96).

 **Descrição:**  
Ocorre quando o compilador identifica o operador unário `-` sendo usado com uma variável que não possui tipo numérico (`int` ou `float`). Se o tipo for `string` ou `bool`, o analisador emite a mensagem de erro e incrementa o contador de erros.

**Código que dispara o erro:**

```python
texto = "abc"
x = -texto

```

**Saída do compilador:**
```
Erro semântico na linha 2: Operador '-' exige operando numérico.
```

**Código que NÃO dispara o erro (válido):**

```python
x = -10
y = -3.14

```

**Comportamento do analisador:**  
Após identificar um operando do tipo `string` ou `bool` com o operador `-`, o analisador emite a mensagem de erro e incrementa o contador de erros sem interromper a análise. O resultado permanece como `TIPO_DESCONHECIDO`, e a análise continua para buscar outros possíveis erros.

---

## SEM-07 — Operador 'not' exige booleano.

**mensagem:**
```
Erro semântico na linha <N>: Operador 'not' exige booleano.
```
**Onde é gerado:**
 [`src/semantica.c`](../src/semantica.c), função `inferir_tipo` (caso `NO_UNARYOP`, linha 101).

 **Descrição:**  
Ocorre quando o compilador identifica o operador `not` sendo usado com um operando que não é do tipo `bool`. Operandos com tipo desconhecido não geram uma segunda mensagem de erro.

**Código que dispara o erro:**

```python
idade = 18
resultado = not idade

```

**Saída do compilador:**
```
Erro semântico na linha 2: Operador 'not' exige booleano.
```

**Código que NÃO dispara o erro (válido):**

```python
ativo = True
resultado = not ativo

```

**Comportamento do analisador:**  
Após identificar um tipo diferente de `bool` com o operador `not`, o analisador emite a mensagem de erro e incrementa o contador de erros sem interromper a análise. O resultado da operação recebe o tipo `TIPO_BOOL`, e a análise continua para buscar outros possíveis erros.

---
## SEM-08 — <nome> não é uma função.

**mensagem:**
```
Erro semântico na linha <N>: <nome> não é uma função.

```
**Onde é gerado:**
 [`src/semantica.c`](../src/semantica.c), função  `analisar_semantica` (caso `NO_FUNCCALL`, linha 116).

 **Descrição:**  
Ocorre quando o compilador tenta chamar como função uma variável ou outro identificador que não foi declarado como função.

**Código que dispara o erro:**

```python
numero = 10
resultado = numero()
```

**Saída do compilador:**
```
Erro semântico na linha 2: 'numero' não é uma função.

```

**Código que NÃO dispara o erro (válido):**

```python
def dobrar(valor):
    return valor * 2

resultado = dobrar(10)
```

**Comportamento do analisador:**  
Ao encontrar uma chamada para um identificador registrado na tabela de símbolos como variável, e não como função, o analisador emite a mensagem de erro e incrementa o contador de erros. Em seguida, continua a análise do restante do programa. O tipo associado à chamada recebe o tipo armazenado no símbolo.

---
## SEM-09 — Número errado de argumentos.

**mensagem:**
```
Erro semântico na linha <N>: Número errado de argumentos.

```
**Onde é gerado:**
 [`src/semantica.c`](../src/semantica.c), função  `analisar_semantica` (caso `NO_FUNCCALL`, linha 119).

 **Descrição:**  
Ocorre quando o compilador chama uma função com um número de argumentos diferente do número de parâmetros declarados.

**Código que dispara o erro:**

```python
def somar(a, b):
  return a + b

somar(1, 2, 3)  # Erro: espera 2 argumentos, mas recebeu 3
```

**Saída do compilador:**
```
Erro semântico na linha 3: Número errado de argumentos.

```

**Código que NÃO dispara o erro (válido):**

```python
def calcular_area(largura, altura):
    return largura * altura
```

**Comportamento do analisador:**  
Ao encontrar uma chamada para um identificador registrado na tabela de símbolos como função, o analisador compara o número de argumentos com o número de parâmetros. Se os números forem diferentes, emite a mensagem de erro e incrementa o contador de erros. Em seguida, continua a análise do restante do programa. O tipo associado à chamada recebe o tipo armazenado no símbolo.

---
## SEM-10 — Tipos dos argumentos são incompatíveis.

**mensagem:**
```
Erro semântico na linha <N>: Tipos dos argumentos são incompatíveis.

```
**Onde é gerado:**
 [`src/semantica.c`](../src/semantica.c), função  `analisar_semantica` (caso `NO_FUNCCALL`, linha 125).

 **Descrição:**  
Ocorre quando uma função é chamada com argumentos cujos tipos não correspondem aos tipos esperados pelos seus parâmetros.

**Código que dispara o erro:**

```python
def dobrar(numero):
    return numero * 2

dobrar("texto")
```

**Saída do compilador:**
```
Erro semântico na linha 4: Tipos dos argumentos são incompatíveis.

```

**Código que NÃO dispara o erro (válido):**

```python
def dobrar(numero):
    return numero * 2

dobrar(10)
```

**Comportamento do analisador:**  
Ao analisar uma chamada de função, o analisador verifica o tipo de cada argumento em relação ao tipo esperado pelo parâmetro correspondente. Quando encontra uma incompatibilidade, emite a mensagem de erro e incrementa o contador de erros. Em seguida, continua analisando o restante do programa. O tipo da chamada recebe o tipo de retorno registrado para a função.

**Observação:**  
Atualmente, os tipos dos parâmetros são inicializados como `TIPO_DESCONHECIDO`. Portanto, o erro só é emitido quando o tipo esperado do parâmetro já estiver registrado na tabela de símbolos.

---

## SEM-11 — Chamada a função não declarada <nome>.

**mensagem:**
```
Erro semântico na linha <N>: Chamada a função não declarada '<nome>'.

```
**Onde é gerado:**
 [`src/semantica.c`](../src/semantica.c), função  `analisar_semantica` (caso `NO_FUNCCALL`, linha 132).

 **Descrição:**  
Ocorre quando o programa tenta chamar uma função que não foi declarada anteriormente.

**Código que dispara o erro:**

```python
resultado = calcular(10)
```

**Saída do compilador:**
```
Erro semântico na linha 1: Chamada a função não declarada 'calcular'.

```

**Código que NÃO dispara o erro (válido):**

```python
def calcular(valor):
    return valor * 2

resultado = calcular(10)
```

**Comportamento do analisador:**  
Ao encontrar uma chamada de função, o analisador procura o nome da função na tabela de símbolos. Caso não encontre o identificador, emite a mensagem de erro, incrementa o contador de erros e atribui `TIPO_DESCONHECIDO` ao resultado da chamada. Depois, continua analisando o restante do programa.

---

## SEM-12 — A condição deve ser estritamente booleana.

**mensagem:**
```
Erro semântico na linha <N>: A condição deve ser estritamente booleana.

```
**Onde é gerado:**
 [`src/semantica.c`](../src/semantica.c), função  `analisar_semantica` (casos `NO_IF`, `NO_WHILE`, `NO_ELIF`, linha 230).

 **Descrição:**  
Ocorre quando o compilador identifica um momento em que uma estrutura condicional recebe um valor que não é `booleano`.

**Código que dispara o erro:**

```python
idade = 18
if idade:
    print("maior de idade")
#A condição do if precisa ser booleana
```

**Saída do compilador:**
```
Erro semântico na linha 3: A condição deve ser estritamente booleana.

```

**Código que NÃO dispara o erro (válido):**

```python
idade = 18

if idade >= 18:
    print("Maior de idade")
```

**Comportamento do analisador:**  
Após identificar que a condição não possui tipo `bool`, o analisador emite a mensagem de erro e incrementa o contador de erros sem interromper a análise. Em seguida, continua analisando a condição e os blocos associados ao `if`, `while` ou `elif`, permitindo identificar outros erros no mesmo arquivo. Se a condição tiver tipo `TIPO_DESCONHECIDO`, o analisador não emite este erro novamente, pois a causa já foi reportada anteriormente.

---

## SEM-13 — 'return' fora de função gera erro.

**mensagem:**
```
Erro semântico na linha <N>: 'return' fora de função gera erro.
```
**Onde é gerado:**
 [`src/semantica.c`](../src/semantica.c), função  `analisar_semantica` (casos `NO_RETURN`, linha 288).

 **Descrição:**  
Ocorre quando o comando `return` é utilizado no escopo principal do programa, fora da declaração de uma função.

**Código que dispara o erro:**

```python
valor = 10
return valor
```

**Saída do compilador:**
```
Erro semântico na linha 2: 'return' fora de função gera erro.

```

**Código que NÃO dispara o erro (válido):**

```python
def obter_valor():
    return 10
```

**Comportamento do analisador:**  
Ao encontrar um comando `return`, o analisador verifica se está dentro do escopo de uma função. Caso contrário, emite a mensagem de erro e incrementa o contador de erros. Mesmo após o erro, continua analisando a expressão de retorno e o restante do programa.

--- 

## SEM-14 — Tipo do retorno é inconsistente.

**mensagem:**
```
Erro semântico na linha <N>: Tipo do retorno é inconsistente.
```
**Onde é gerado:**
 [`src/semantica.c`](../src/semantica.c), função `analisar_semantica` (caso `NO_RETURN`, linha 302).

 **Descrição:**  
Ocorre quando uma mesma função possui comandos `return` com tipos diferentes.

**Código que dispara o erro:**

```python
def verificar(valor):
    if valor > 0:
        return 1
    else:
        return "negativo"
```

**Saída do compilador:**
```
Erro semântico na linha 5: Tipo do retorno é inconsistente.

```

**Código que NÃO dispara o erro (válido):**

```python
def dobrar(valor):
    if valor > 0:
        return valor * 2
    else:
        return 0
```

**Comportamento do analisador:**  
Ao analisar o primeiro `return` da função, o analisador registra o tipo do valor retornado. Nos `return` seguintes, compara o tipo encontrado com o tipo já registrado. Se os tipos forem diferentes, emite a mensagem de erro e incrementa o contador de erros. A análise continua normalmente para encontrar outros possíveis erros.

--- 
