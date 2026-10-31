# Especificação da Linguagem: Mini-Python para C\#

Este documento define o escopo, a sintaxe e a semântica da linguagem fonte (um subconjunto do Python) que será compilada para a linguagem alvo (**C#**), cumprindo as exigências de especificação do projeto de Compiladores.

## 1. Escopo da Linguagem (Subconjunto de Python)

Para manter a complexidade do compilador factível durante o semestre, implementaremos um subconjunto focado e rigoroso de Python, suportando as seguintes características:

- **Tipos de Dados Nativos**:
  - Numéricos: `int` (inteiros) e `float` (ponto flutuante, mapeado para `double` em C#).
  - Booleanos: `True` e `False` (`bool`).
  - Texto: `str` (`string`).
  - Listas: literais de lista como `[]` e `[1, 2, 3]`.
- **Estruturas de Controle de Fluxo**:
  - Condicionais: `if`, `elif`, `else`.
  - Laços de repetição: `while` e `for` (utilizado com `in range(...)` de 1, 2 ou 3 argumentos).
- **Funções**:
  - Definição de funções pelo usuário utilizando a palavra-chave `def`.
  - Retorno de valores com `return` (com ou sem expressão).
  - Funções nativas predefinidas: `print()` e `input()`.
- **Blocos de Código (Indentação)**:
  - Respeitando o padrão Python, blocos de código são definidos através de níveis de indentação reais. O analisador léxico gera tokens de fim de linha lógica (`NEWLINE`), aumento de indentação (`INDENT`) e retorno de indentação (`DEDENT`).

---

## 2. Dicionário de Tokens Léxicos

O analisador léxico (`Flex`) reconhece e gera os seguintes tokens:

### Palavras-Chave (Keywords)
`IF`, `ELIF`, `ELSE`, `WHILE`, `FOR`, `IN`, `RANGE`, `DEF`, `RETURN`, `AND`, `OR`, `NOT`, `TRUE`, `FALSE`, `PRINT`, `INPUT`

### Identificadores e Literais
- `ID`: Nomes de variáveis e funções (ex: `soma`, `x`, `valor1`).
- `NUM_INT`: Números inteiros (ex: `42`).
- `NUM_FLOAT`: Números de ponto flutuante e notação científica (ex: `3.14`, `.5`, `1e10`).
- `STRING_LITERAL`: Textos delimitados por aspas duplas ou simples (ex: `"Olá Mundo"`, `'texto'`).

### Operadores e Pontuação
- Aritméticos: `PLUS` (`+`), `MINUS` (`-`), `TIMES` (`*`), `DIVIDE` (`/`)
- Atribuição: `ASSIGN` (`=`)
- Relacionais: `EQ` (`==`), `NEQ` (`!=`), `LT` (`<`), `GT` (`>`), `LTE` (`<=`), `GTE` (`>=`)
- Pontuação e Delimitadores: `LPAREN` (`(`), `RPAREN` (`)`), `LCOLCH` (`[`), `RCOLCH` (`]`), `COMMA` (`,`), `COLON` (`:`)

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

namespace CompiladorPython {
    static class Program {
        static void saudacao(string nome) {
            Console.WriteLine("Olá");
            Console.WriteLine(nome);
        }

        static void Main(string[] args) {
            int limite = 5;
            for (int i = 0; i < limite; i++) {
                if (i == 3) {
                    saudacao("Visitante");
                } else {
                    Console.WriteLine(i);
                }
            }
        }
    }
}
```

---

## 4. Gramática Formal Implementada (Formato Bison / BNF)

Abaixo está a gramática livre de contexto implementada no arquivo [`parser/parser.y`](../parser/parser.y):

```text
programa : %empty
         | comandos

comandos : comando
         | comandos comando

comando : comando_simples NEWLINE
        | comando_composto
        | erro_linha
        | erro_linha INDENT comandos DEDENT
        | erro_indentacao comandos DEDENT

bloco : NEWLINE INDENT comandos DEDENT

comando_simples : expressao
                | RETURN
                | RETURN expressao
                | PRINT LPAREN argumentos_opt RPAREN

comando_composto : IF expressao COLON bloco lista_elif senao_opt
                 | WHILE expressao COLON bloco
                 | FOR ID IN RANGE LPAREN argumentos_opt RPAREN COLON bloco
                 | DEF ID LPAREN parametros_opt RPAREN COLON bloco

lista_elif : %empty
           | lista_elif ELIF expressao COLON bloco

senao_opt : %empty %prec LOWER_THAN_ELSE
          | ELSE COLON bloco

parametros_opt : %empty
               | parametros

parametros : ID
           | parametros COMMA ID

argumentos_opt : %empty
               | expressao

expressao : expressao PLUS expressao
          | expressao MINUS expressao
          | expressao TIMES expressao
          | expressao DIVIDE expressao
          | expressao ASSIGN expressao
          | expressao EQ expressao
          | expressao NEQ expressao
          | expressao GTE expressao
          | expressao LTE expressao
          | expressao GT expressao
          | expressao LT expressao
          | expressao AND expressao
          | expressao OR expressao
          | NOT expressao
          | expressao IN expressao
          | MINUS expressao %prec NOT
          | expressao COMMA expressao
          | LPAREN expressao RPAREN
          | LCOLCH RCOLCH
          | LCOLCH expressao RCOLCH
          | INPUT LPAREN argumentos_opt RPAREN
          | ID LPAREN argumentos_opt RPAREN
          | FALSE | TRUE | NUM_INT | NUM_FLOAT | STRING_LITERAL | ID
```
