# Manual de Uso do Compilador

Este manual explica como compilar, executar e inspecionar o compilador **Mini-Python → C#** (`COMP1-grupo2`), além de apresentar exemplos práticos e as limitações conhecidas da versão atual.

---

## 1. Pré-requisitos

Para compilar o compilador em si, são necessários:

- **GCC** (`gcc`)
- **Flex** (`flex` e `libfl-dev`)
- **Bison** (`bison`)
- **GNU Make** (`make`)

No Linux (Ubuntu/Debian):

```bash
sudo apt-get update
sudo apt-get install -y build-essential flex bison libfl-dev
```

No Windows, recomenda-se usar **WSL** (Ubuntu) ou **MSYS2**.

### Ferramentas opcionais

- **Graphviz (`dot`)**: necessário apenas para gerar a imagem `ast.png` ao usar a flag `--dot`.
- **.NET SDK (`dotnet`)**: necessário apenas para compilar e executar o código C# gerado.

---

## 2. Como compilar o compilador

Entre na pasta do projeto e execute `make`:

```bash
cd compiladores-projeto
make
```

O comando executa o Flex (`lexer/lexer.l`), o Bison (`parser/parser.y`) e compila todos os módulos C de `src/`, gerando o executável `./compilador`.

Outros alvos úteis (detalhados no [Guia do Makefile](Makefile.md)):

```bash
make CI=1       # compila com -Werror (trata warnings como erro)
make test       # executa todas as suítes unitárias e os testes de integração
make test_e2e   # executa os testes ponta a ponta (Python -> C# compilado e executado)
make clean      # remove executáveis e arquivos gerados
```

---

## 3. Como usar

O compilador lê o código-fonte em Python a partir da **entrada padrão (`stdin`)** e escreve diagnósticos de erro em `stderr`.

### Apenas validar o código (léxico, sintático e semântico)

```bash
./compilador < entrada.py
```

- Se o programa for válido, encerra silenciosamente com código de saída **`0`**.
- Se houver erros léxicos, sintáticos ou semânticos, imprime todos os erros encontrados em `stderr` e encerra com código de saída **`1`**.

### Compilar Python para C# (`--cs`)

Para emitir o código-alvo em C#, passe a flag `--cs`. Há duas formas equivalentes:

```bash
# Redirecionando o stdout para o arquivo de saída:
./compilador --cs < entrada.py > saida.cs

# Ou informando o arquivo de saída diretamente na flag:
./compilador --cs saida.cs < entrada.py

# Gerar C# aplicando as otimizações diretamente na AST:
./compilador --opt --cs saida.cs < entrada.py
```

> O código C# só é gerado quando o programa passa sem erros nas fases léxica, sintática e semântica.

### Flags de linha de comando disponíveis

As flags abaixo podem ser usadas isoladamente ou combinadas:

| Flag | O que faz |
|---|---|
| `--ast` | Imprime a Árvore Sintática Abstrata (AST) indentada com o tipo inferido (`tipo_resolvido`) de cada nó. |
| `--dot` | Exporta a AST em formato Graphviz DOT para `stdout` e `ast.dot` (gera `ast.png` se `dot` estiver instalado). |
| `--tabela` | Exibe a Tabela de Símbolos a cada mudança de escopo durante a análise semântica. |
| `--tac` | Gera e imprime o Código Intermediário de Três Endereços (TAC). |
| `--opt` | Sem `--cs`, gera o TAC, executa as otimizações (*constant folding*, simplificação algébrica, propagação e remoção de código morto) e imprime o TAC otimizado. Combinada com `--cs`, otimiza a AST via `otimizar_ast` antes de gerar o C# (sem imprimir TAC, salvo se `--tac` também for informada). |
| `--cs [arquivo]` | Gera o código-alvo em C# em `arquivo` (ou em `stdout` se nenhum nome for informado). |

---

## 4. Exemplos de uso

### Exemplo 1 — Traduzir Python para C#

Dado um arquivo `entrada.py`:

```python
x = 10
y = 3.14
nome = 'bia'
media = x / 4
print("oi,", nome)
print("media =", media)
```

Execute:

```bash
./compilador --cs < entrada.py > Program.cs
```

Saída gerada em `Program.cs`:

```csharp
using System;

namespace CompiladorPython {
    static class Program {
        static void Main(string[] args) {
            System.Globalization.CultureInfo.CurrentCulture = System.Globalization.CultureInfo.InvariantCulture;
            int x = 10;
            double y = 3.14;
            string nome = "bia";
            double media = (double)x / 4;
            Console.WriteLine(string.Join(" ", "oi,", nome));
            Console.WriteLine(string.Join(" ", "media =", media));
        }
    }
}
```

### Exemplo 2 — Compilar e executar o C# gerado com .NET

Com o `.NET SDK` instalado, você pode rodar o `Program.cs` gerado dentro de um projeto console temporário:

```bash
mkdir -p /tmp/teste-cs && cd /tmp/teste-cs
dotnet new console --force
cp /caminho/para/Program.cs Program.cs
dotnet run
```

Saída esperada:

```text
oi, bia
media = 2.5
```

> Para validar automaticamente múltiplos programas Python traduzidos, compilados e executados em C# contra o `python3`, execute `make test_e2e` (ou `./testes/rodar_testes_e2e.sh`). Consulte o [Relatório de Testes End-to-End (E2E)](Testes_E2E.md).

### Exemplo 3 — Inspecionar fases intermediárias (AST, Tabela de Símbolos e TAC)

```bash
# Ver AST anotada e Tabela de Símbolos:
./compilador --ast --tabela < entrada.py

# Comparar o TAC original com o TAC otimizado:
./compilador --tac < entrada.py
./compilador --opt < entrada.py

# Gerar C# com otimizações aplicadas na AST (sem imprimir TAC):
./compilador --opt --cs Program.cs < entrada.py
```

### Exemplo 4 — Diagnóstico de erros

Dado um arquivo com erro semântico (`erro.py`):

```python
texto = "ola"
resultado = texto + 5
```

Executando `./compilador --cs < erro.py > saida.cs`:

```text
Erro semântico na linha 2: Operação aritmética com tipos incompatíveis.
1 erro(s) semântico(s) encontrado(s).
```

O compilador retorna código de saída `1` e não gera código C#. A lista completa de erros está no [Catálogo de Erros Semânticos](ErrosSemanticos.md).

---

## 5. Limitações conhecidas

### Subconjunto da linguagem fonte

O compilador implementa um subconjunto focado de Python (ver [Especificação da Linguagem](Especificacao_Linguagem.md)):

- **Não suportados pela gramática/lexer:** classes (`class`), exceções (`try`/`except`), módulos (`import`), dicionários, tuplas, conjuntos, f-strings, fatiamento (*slicing*), *list comprehensions*, `break`/`continue`, `pass`, operadores `//`, `%`, `**`, atribuição composta (`+=`, `-=`) e parâmetros com valor padrão.
- **Listas (`[1, 2, 3]`):** são reconhecidas sintaticamente pelo parser, mas ainda não possuem nó dedicado nem tipagem/tradução para `List<T>` no gerador C#.

### Diferenças semânticas em relação ao Python padrão

- **Tipagem estática por variável ([ADR-001](ADR/001-reatribuicao-de-tipo.md)):** uma variável assume o tipo inferido na primeira atribuição. Reatribuições só são permitidas com o mesmo tipo ou promoção `int` → `float`; trocar para tipo incompatível (ex.: `int` para `string`) gera erro semântico (`SEM-17`).
- **Condições estritamente booleanas:** `if`, `elif` e `while` exigem expressão do tipo `bool`. Expressões numéricas ou strings como condição (ex.: `if x:`) geram erro (`SEM-12`), diferentemente do *truthiness* implícito do Python.
- **Redefinição e conflito de nomes:** não é permitido redefinir funções no mesmo escopo (`SEM-15`) nem usar o mesmo identificador como variável e função no mesmo escopo (`SEM-16`).

### Construções traduzidas e pendentes na geração de código C#

O analisador léxico, sintático, semântico, o gerador de TAC e o otimizador suportam toda a linguagem especificada. No módulo de **geração de código C#** (`src/codegen_csharp.c`):

- **Já traduzidos para C#:** estrutura base (`namespace CompiladorPython`, `static class Program`, `Main`), variáveis/atribuições (`int`, `double`, `string`, `bool`) com *hoisting* de variáveis de blocos (`= default(<tipo>)`), expressões aritméticas/lógicas/relacionais (incluindo divisão real `(double)`, comparações de `string` e `bool`, negação e escape de palavras reservadas com `@`), estruturas de controle (`if`/`elif`/`else`, `while` e `for ... in range(...)` com 1, 2 e 3 argumentos, passo negativo/dinâmico e guarda contra passo zero), definições e chamadas de função (`def` como métodos `static`, `return` com e sem valor), `print(...)` (`Console.WriteLine`) e `input(...)` (`Console.ReadLine`, com e sem prompt).
- **Ainda não traduzidos para C#:** operador `in` fora do cabeçalho do `for` (e listas). Quando presentes no código-fonte, são emitidos como comentários `// [não traduzido]` ou `default(<tipo>)` com aviso em `stderr`.

### Outras observações de execução

- Na execução do C# gerado, resultados `double` com parte fracionária zero (ex.: `print(4 / 2)`) são impressos como `2` no C#, enquanto o Python imprime `2.0`.
