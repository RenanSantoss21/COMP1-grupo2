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

Em C#, uma variável declarada dentro de `{ }` deixa de existir no `}`. Por isso, quando os blocos forem traduzidos (Issue #30), as variáveis atribuídas dentro deles devem ser **declaradas no início do método**, uma única vez, com o tipo fixado pela ADR-001:

```csharp
string situacao = default(string);
if (nota >= 5) {
    situacao = "aprovado";
} else {
    situacao = "reprovado";
}
Console.WriteLine(situacao);
```

- O `= default(...)` é necessário porque o C# exige que a variável tenha valor garantido antes do uso (`CS0165`).
- Declarar dentro do bloco também não serviria: `if (c) { int y = 1; }` seguido de `int y = 2;` não compila em C# (`CS0136`).
- Como cada variável tem um único tipo por escopo (ADR-001), uma declaração por variável basta, sem renomear (`y_1`, `y_2`).
- A variável do `for` também é declarada no início do método, porque em Python ela continua existindo depois do laço.
- Diferença restante: se o programa lê uma variável que nunca recebeu valor, o Python dá `NameError` em tempo de execução e o C# lê o valor padrão (`0`, `null`).

Identificadores que são palavras reservadas do C# recebem `@` (`string = 1` → `int @string = 1;`).

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

## Construções ainda não traduzidas

`print`, `input`, `if`/`elif`/`else`, `while`, `for`, `def`, `return`, chamadas de função e o operador `in` ficam para as próximas subtasks. Enquanto isso, o gerador as marca com comentário (`// [não traduzido] if (linha 5)`) ou, dentro de expressões, com um valor neutro do tipo inferido (`default(int)`), para que o arquivo continue compilando. O compilador avisa no `stderr` quantas construções ficaram pendentes.

Chamadas de função entram nessa lista porque, sem o `def` correspondente emitido, a chamada referenciaria um método inexistente.

## Testes

- Unitários: `make test_codegen_csharp` (`testes/tdd/teste_codegen_csharp.c`).
- Integração: seção "Geracao de codigo C#" de `testes/rodar_testes.sh`, com o programa `testes/TestesCodegenCSharp1.py`.
