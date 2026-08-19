# Especificação da Linguagem: Mini-Python para C#

Este documento define o escopo, a sintaxe e a semântica da linguagem fonte (um subconjunto do Python) que será compilada para a linguagem alvo (C#), cumprindo as exigências da Sprint 1 do projeto de Compiladores.

## 1. Escopo da Linguagem (Subconjunto de Python)

Para manter a complexidade do compilador factível durante o semestre, implementaremos um subconjunto focado e rigoroso de Python, suportando as seguintes características:

- **Tipos de Dados Nativo**:
  - Numéricos: `int` (inteiros) e `float` (ponto flutuante).
  - Booleanos: `True` e `False`.
  - Texto: `str` (strings).
- **Estruturas de Controle de Fluxo**:
  - Condicionais: `if`, `elif`, `else`.
  - Laços de repetição: `while` e `for` (restrito ao uso com `range()`).
- **Funções**:
  - Definição de funções pelo usuário utilizando a palavra-chave `def`.
  - Retorno de valores com `return`.
  - Funções nativas predefinidas: `print()` e `input()`.
- **Blocos de Código (Identação)**:
  - Respeitando o padrão Python, blocos de código serão definidos através de níveis de indentação reais. O analisador léxico será responsável por gerar tokens de aumento de indentação (`INDENT`) e retorno de indentação (`DEDENT`).

---

## 2. Dicionário de Tokens Léxicos

O analisador léxico (`Flex`) deverá reconhecer e gerar os seguintes tokens:

### Palavras-Chave (Keywords)
`IF`, `ELIF`, `ELSE`, `WHILE`, `FOR`, `IN`, `RANGE`, `DEF`, `RETURN`, `AND`, `OR`, `NOT`, `TRUE`, `FALSE`, `PRINT`, `INPUT`

### Identificadores e Literais
- `ID`: Nomes de variáveis e funções (ex: `soma`, `x`, `valor1`).
- `NUM_INT`: Números inteiros (ex: `42`).
- `NUM_FLOAT`: Números de ponto flutuante (ex: `3.14`).
- `STRING_LITERAL`: Textos delimitados por aspas duplas ou simples (ex: `"Olá Mundo"`).

### Operadores e Pontuação
- Aritméticos: `PLUS` (+), `MINUS` (-), `TIMES` (*), `DIVIDE` (/)
- Atribuição: `ASSIGN` (=)
- Relacionais: `EQ` (==), `NEQ` (!=), `LT` (<), `GT` (>), `LE` (<=), `GE` (>=)
- Pontuação: `LPAREN` ( ( ), `RPAREN` ( ) ), `COMMA` ( , ), `COLON` ( : )

### Controle de Fluxo Físico (Especial para Python)
- `NEWLINE`: Fim de instrução lógica.
- `INDENT`: Abertura de um novo bloco aninhado de código.
- `DEDENT`: Fechamento de um bloco aninhado de código.

---

## 3. Exemplo de Código (Fonte vs Alvo)

### Código Fonte (Mini-Python)
```python
def saudacao(nome):
    print("Olá")
    print(nome)

limite = 5
for i in range(limite):
    if i == 3:
        saudacao("Visitante")
    else:
        print(i)
```

### Código Alvo Esperado (C#)
```csharp
using System;

void saudacao(string nome) {
    Console.WriteLine("Olá");
    Console.WriteLine(nome);
}

int limite = 5;
for (int i = 0; i < limite; i++) {
    if (i == 3) {
        saudacao("Visitante");
    } else {
        Console.WriteLine(i);
    }
}
```

---

## 4. Protótipo da Gramática Formal (Formato Bison / BNF)

Abaixo está o rascunho da gramática livre de contexto que guiará nosso arquivo `parser.y`:

```text
programa : declaracoes

declaracoes : declaracao declaracoes
            | /* vazio */

declaracao : declaracao_simples NEWLINE
           | declaracao_composta

declaracao_simples : ID ASSIGN expressao
                   | chamada_funcao
                   | RETURN expressao
                   | RETURN

chamada_funcao : ID LPAREN lista_argumentos RPAREN

lista_argumentos : expressao COMMA lista_argumentos
                 | expressao
                 | /* vazio */

declaracao_composta : condicional_if
                    | laco_while
                    | laco_for
                    | definicao_funcao

condicional_if : IF expressao COLON bloco_codigo blocos_elif bloco_else

blocos_elif : ELIF expressao COLON bloco_codigo blocos_elif
            | /* vazio */

bloco_else : ELSE COLON bloco_codigo
           | /* vazio */

laco_while : WHILE expressao COLON bloco_codigo

laco_for : FOR ID IN RANGE LPAREN expressao RPAREN COLON bloco_codigo

definicao_funcao : DEF ID LPAREN lista_parametros RPAREN COLON bloco_codigo

lista_parametros : ID COMMA lista_parametros
                 | ID
                 | /* vazio */

bloco_codigo : NEWLINE INDENT declaracoes DEDENT

expressao : expressao PLUS termo
          | termo
          // (demais regras de expressões, precedências aritméticas e lógicas)
```
