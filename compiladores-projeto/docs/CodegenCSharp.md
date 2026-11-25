# Geração de Código C#

O módulo `src/codegen_csharp.c` percorre a AST já validada pela análise semântica e emite o programa equivalente em C#. Os tipos das declarações vêm de `tipo_resolvido`, preenchido pela semântica.

## Uso

```bash
./compilador --cs saida.cs < programa.py   # grava em saida.cs
./compilador --cs < programa.py            # imprime no stdout
```

O C# só é gerado quando não há erros léxicos, sintáticos ou semânticos.

## Estrutura do arquivo gerado

```csharp
using System;

namespace CompiladorPython {
    static class Program {
        // funções definidas pelo usuário

        static void Main(string[] args) {
            // código do nível global
        }
    }
}
```

Segue o `CODING_STANDARDS.md`: o código fica dentro de um `namespace` e de uma classe `Program` estática. O nome do namespace é a constante `NAMESPACE_CSHARP` em `codegen_csharp.c`.

## Variáveis

A primeira atribuição declara a variável com o tipo inferido (ADR-001); as seguintes só reatribuem, pois C# não permite redeclarar no mesmo escopo.

| Python | C# |
|--------|-----|
| `x = 10` | `int x = 10;` |
| `x = 3.14` | `double x = 3.14;` |
| `x = "oi"` / `x = 'oi'` | `string x = "oi";` |
| `x = True` | `bool x = true;` |
| `x = 20` (de novo) | `x = 20;` |

### Escopo de variáveis em blocos

O gerador segue a regra de escopo do Python, definida na Issue #45: `if`, `elif`, `else`, `while` e `for` **não criam escopo**. Só funções (`def`) criam. Uma variável atribuída dentro de um bloco continua existindo depois dele:

```python
if nota >= 5:
    situacao = "aprovado"
else:
    situacao = "reprovado"
print(situacao)   # válido
```

Em C#, uma variável declarada dentro de `{ }` deixa de existir no `}`. Por isso, o gerador realiza **hoisting** no início do método: antes de emitir os comandos de `Main`, percorre a AST do método (sem invadir definições `def`, que abrem outro **Escopo**) e declara no topo do método, uma única vez, todas as variáveis atribuídas inicialmente dentro de blocos de controle (`if`, `elif`, `else`, `while`, `for`) e os índices de `for` que ainda não tenham sido declarados no nível raiz do método:

```csharp
string situacao = default(string);
if (nota >= 5) {
    situacao = "aprovado";
} else {
    situacao = "reprovado";
}
Console.WriteLine(situacao);
```

- O `= default(<tipo>)` é necessário porque o C# exige que a variável tenha valor garantido antes do uso (`CS0165`) e garante compatibilidade desde C# 5 (`csc.exe`) até o .NET moderno. Para nós sem tipo definido (`TIPO_DESCONHECIDO` ou `TIPO_VOID`), o tipo explícito faz fallback para `object = default(object);`.
- Declarar dentro do bloco também não serviria: `if (c) { int y = 1; }` seguido de `int y = 2;` não compila em C# (`CS0136`).
- Como cada variável tem um único tipo por escopo (ADR-001), uma declaração por variável basta, sem renomear (`y_1`, `y_2`).
- A variável do `for` também sofre hoisting como `int` no início do método (caso não tenha sido declarada no nível raiz), porque em Python ela continua existindo depois do laço.
- Diferença restante: se o programa lê uma variável que nunca recebeu valor (por exemplo, dentro de um ramo não executado), o Python dá `NameError` em tempo de execução e o C# lê o valor padrão (`0`, `false`, `null`).

Identificadores que são palavras reservadas do C# recebem `@` (`string = 1` → `int @string = 1;`, ou `int @string = default(int);` quando submetidos a hoisting).

## Expressões

| Python | C# | Observação |
|--------|-----|------------|
| `a + b`, `a - b`, `a * b` | igual | |
| `a / b` (ambos `int`) | `(double)a / b` | Em Python `/` sempre resulta em float; em C# `int / int` trunca. |
| `a and b` | `a && b` | |
| `a or b` | `a \|\| b` | |
| `not a` | `!a` | |
| `not a == b` | `!(a == b)` | Em Python `not` tem precedência menor que `==`; em C# `!` tem maior. |
| `- -a` | `-(-a)` | Evita gerar `--a` (decremento em C#). |
| `s < t` (strings) | `string.CompareOrdinal(s, t) < 0` | C# não tem `<`, `>`, `<=`, `>=` para `string`; a comparação ordinal segue a ordem de código, como no Python. |
| `b > c` (booleanos) | `(b ? 1 : 0) > (c ? 1 : 0)` | C# não ordena `bool`; no Python, `True` vale 1 e `False` vale 0. |

Os parênteses são emitidos apenas quando a precedência do C# exige (`(a + b) * 2`, `a - (b - c)`), e não em volta de toda subexpressão.

## Saída (`print`) e Entrada (`input`)

| Python | C# | Observação |
|--------|-----|------------|
| `print()` | `Console.WriteLine();` | Imprime linha em branco. |
| `print(x)` | `Console.WriteLine(x);` | Um único argumento vai direto para `Console.WriteLine`. |
| `print("x =", x)` | `Console.WriteLine(string.Join(" ", "x =", x));` | Múltiplos argumentos são separados por espaço, como no Python. |
| `input()` | `Console.ReadLine()` | Em comando solto emite `Console.ReadLine();`. |
| `x = input()` | `string x = Console.ReadLine();` | `NO_INPUT` tem tipo `TIPO_STRING` inferido pela semântica. |
| `x = input("Nome: ")` | `Console.Write("Nome: ");` + `string x = Console.ReadLine();` | Quando há prompt, emite `Console.Write(prompt)` antes do `ReadLine()` (`CODING_STANDARDS.md`). |

- Quando o programa contém ao menos um `print`, o gerador emite no início de `Main`:
  ```csharp
  System.Globalization.CultureInfo.CurrentCulture = System.Globalization.CultureInfo.InvariantCulture;
  ```
  Isso garante que números `double` sempre usem ponto decimal (`3.14`), independentemente da cultura do sistema operacional (como `pt-BR`, que imprimiria `3,14`).
- Valores `bool` já são formatados pelo C# como `True` e `False`, coincidindo com a saída do Python.
- Se `input(prompt)` for usado dentro de uma expressão composta (ex.: `"Olá, " + input("Nome: ")`), o gerador emite um método auxiliar `static string _input(object prompt)` na classe `Program` que chama `Console.Write(prompt)` e retorna `Console.ReadLine()`.

## Funções (`def`, `return` e chamadas)

Definições de funções (`NO_FUNCDEF`) são emitidas como métodos `static` da classe `Program`, antes de `Main`:

| Python | C# | Observação |
|--------|-----|------------|
| `def saudacao(nome):` (sem `return` com valor) | `static void saudacao(string nome) { ... }` | Retorno `void` e tipo do parâmetro inferido pelo uso ou pela 1ª chamada. |
| `def soma(x, y): return x + y` | `static int soma(int x, int y) { return x + y; }` | Tipo de retorno e parâmetros lidos de `tipo_resolvido`. |
| `return` (sem expressão) | `return;` | |
| `return expr` | `return expr;` | |
| `foo(a, b)` | `foo(a, b)` / `foo(a, b);` | Funciona tanto como expressão quanto como comando solto. |

- Cada função possui seu próprio contexto de variáveis declaradas e seu próprio **hoisting** de variáveis criadas dentro de blocos (`if`, `while`, `for`) no início do corpo do método, sem interferir nas variáveis de `Main`.
- Se uma função com tipo de retorno não-`void` tiver caminhos que não terminam em `return` (ex.: `return` apenas dentro de um `if` sem `else`), o gerador acrescenta `return default(<tipo>);` ao final do método para satisfazer a verificação de fluxo do C# (`CS0161`).
- **Restrições do modelo estático em C#:**
  - Como as variáveis globais do script são emitidas como variáveis locais de `Main`, os métodos `static` gerados para `def` não têm acesso direto às variáveis globais de `Main` (os valores devem ser passados via parâmetros).
  - As funções `def` devem ser declaradas no nível global do programa (funções aninhadas dentro de outro `def` não são suportadas na emissão para C#) e seus parâmetros devem ter tipo inferível pelo uso no corpo ou por ao menos uma chamada no programa (caso contrário, recebem o fallback `object`).

## Estruturas de controle

O gerador traduz `if`/`elif`/`else`, `while` e `for ... in range(...)`, mantendo indentação de 4 espaços por nível mesmo em blocos aninhados:

| Python | C# | Observação |
|--------|-----|------------|
| `if c:` / `elif c2:` / `else:` | `if (c) { ... } else if (c2) { ... } else { ... }` | Cada `NO_ELIF` em `argumentos` vira `} else if (...) {`. |
| `while cond:` | `while (cond) { ... }` | Corpo emitido em `nivel + 1`. |
| `for i in range(n):` | `for (i = 0; i < n; i++) { ... }` | `n` emitido respeitando precedência relacional (`range(n + 1)` → `i < n + 1`). |
| `for i in range(a, b):` | `for (i = a; i < b; i++) { ... }` | `i` já foi declarado no nível raiz ou via hoisting no topo do método. |
| `for i in range(a, b, k):` (`k > 0`) | `for (i = a; i < b; i++)` ou `i += k` | Usa `i++` quando `k == 1`. |
| `for i in range(a, b, k):` (`k < 0`) | `for (i = a; i > b; i--)` ou `i -= -k` | Inverte o comparador para `>` e usa `i--` quando `k == -1`. |
| `for i in range(a, b, passo):` (`passo == 0` ou expressão dinâmica) | `int _passoN = passo;` + guarda `if (_passoN == 0) { throw new ArgumentException("range() arg 3 must not be zero"); }` + `for (i = a; _passoN > 0 ? i < b : i > b; i += _passoN)` | Avalia o passo uma única vez, impede laço infinito para passo zero (equivalente ao `ValueError` do Python) e gera `_passoN` sem colidir com variáveis do usuário. |

### Diferença pós-laço no índice do `for`

Em Python, após um laço `for i in range(n):` que executa ao menos uma vez, a variável `i` mantém o **último valor iterado** (`n - 1` para passo `1`), ou não chega a ser definida caso `range(0)` execute zero vezes (`NameError`). Em C#, o incremento executa ao final da última iteração antes da falha da condição: ao sair de `for (i = 0; i < n; i++)`, `i` passa a valer `n` (`fim`), ou mantém `default(int)` (`0`) se o laço não executar nenhuma vez.

## Construções ainda não traduzidas

O operador `in` fora do cabeçalho do `for` (e listas) é marcado com comentário ou, dentro de expressões, com um valor neutro do tipo inferido (`default(bool)`), para que o arquivo continue compilando. O compilador avisa no `stderr` quantas construções ficaram pendentes.

## Testes

- Unitários: `make test_codegen_csharp` (`testes/tdd/teste_codegen_csharp.c`).
- Integração: seção "Geracao de codigo C#" de `testes/rodar_testes.sh`, com os programas `testes/TestesCodegenCSharp1.py` a `testes/TestesCodegenCSharp6.py` e `testes/TestesCodegenCSharpErroPassoZero.py`.
- End-to-End (E2E): `make test_e2e` ou `./testes/rodar_testes_e2e.sh` (com atalho de compatibilidade em `testes/rodar_e2e.sh`), executando a suíte dedicada em `testes/e2e/` (`01_hello_world.py` a `07_entrada_interativa.py`), os testes `TestesCodegenCSharp1.py` a `TestesCodegenCSharp6.py`, a validação de pipeline completo (`--ast`/`--tabela`/`--tac`/`--opt`/`--cs`) e a guarda de passo zero em runtime, com compilação C# real e comparação contra `.out` e `python3`. Veja o relatório completo em [Relatório de Testes End-to-End (E2E)](Testes_E2E.md).
