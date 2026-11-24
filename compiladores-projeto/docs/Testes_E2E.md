# Relatório de Testes End-to-End (E2E)

Os testes de ponta a ponta (*End-to-End*, Issue #18) validam o compilador completo de **Mini-Python para C#**: partem de programas Python reais na entrada, compilam para C# com `./compilador --cs`, compilam o código C# gerado com um compilador C# real (`dotnet`, `mcs` ou `csc.exe`), executam o binário resultante e comparam a saída obtida com a saída esperada (`.out`) e com a execução de referência no interpretador `python3`.

---

## Pipeline Validado

Para cada programa da suíte E2E, o script `testes/rodar_testes_e2e.sh` executa automaticamente quatro verificações:

1. **Tradução Mini-Python → C# (`--cs`)**:
   - Executa `./compilador --cs` (via `stdout`) e `./compilador --cs arquivo.cs` (em arquivo).
   - Confirma código de saída `0`, igualdade exata entre `stdout` e o arquivo gerado, ausência de mensagens `Aviso:` e ausência de marcações `não traduzido`.
2. **Compilação C# real**:
   - Compila o `.cs` gerado com 0 erros usando o primeiro compilador C# disponível no ambiente (`dotnet`, `mcs`, `csc.exe` ou `dotnet.exe` no WSL).
3. **Execução e validação contra saída esperada (`.out`)**:
   - Executa o binário C# (fornecendo entrada padrão `.in` quando o teste usa `input()`) e compara `stdout` com o arquivo `.out` correspondente.
4. **Comparação com o Python original (`python3`)**:
   - Executa o mesmo arquivo `.py` com `python3` (com o mesmo `.in`, se houver) e valida que a saída do programa C# é idêntica à do Python.

---

## Parte 1 — Suíte E2E Dedicada (`testes/e2e/`)

A pasta `testes/e2e/` reúne 7 programas independentes que cobrem todos os cenários definidos na especificação da linguagem ([Especificação da Linguagem](Especificacao_Linguagem.md)) e na Issue #18:

| Arquivo | Cenário Coberto | Arquivos Auxiliares |
|---|---|---|
| `01_hello_world.py` | **Hello World**: `print("Hello, World!")` e estrutura mínima da classe `Program` | `01_hello_world.out` |
| `02_variaveis_aritmetica.py` | **Variáveis e aritmética**: `int`, `float`, `str`, `bool`, `+`, `-`, `*`, `/`, precedência com parênteses, negação unária, concatenação, promoção `int` → `float` (ADR-001) e reatribuição | `02_variaveis_aritmetica.out` |
| `03_condicionais.py` | **Condicionais**: `if`/`elif`/`else`, operadores relacionais e lógicos (`and`, `or`, `not`), comparação ordinal de strings, `if` aninhado e *hoisting* de variáveis de bloco | `03_condicionais.out` |
| `04_lacos.py` | **Laços**: `while` (fatorial iterativo), `for ... in range(...)` com 1, 2 e 3 argumentos (passo positivo, negativo e dinâmico) e laços aninhados | `04_lacos.out` |
| `05_funcoes.py` | **Funções**: `def` sem parâmetros (`void`), com múltiplos parâmetros, retorno `int`/`double`/`string`, `return` condicional e composição de chamadas | `05_funcoes.out` |
| `06_programa_completo_especificacao.py` | **Programa completo da especificação**: exemplo oficial de [Especificação da Linguagem](Especificacao_Linguagem.md) (`def saudacao(nome)`, `for i in range(limite)`, `if`/`else` e string UTF-8 `"Olá"`) | `06_programa_completo_especificacao.out` |
| `07_entrada_interativa.py` | **Entrada interativa**: `input()` sem prompt e `input("Nome: ")` com prompt, validados com `stdin` redirecionado | `07_entrada_interativa.in`, `07_entrada_interativa.out` |

---

## Parte 2 — Regressão Codegen C#, Pipeline e Guarda de Runtime

Além dos 7 programas dedicados de `testes/e2e/`, o script `testes/rodar_testes_e2e.sh` valida:

| Etapa | Arquivos | O que é validado |
|---|---|---|
| **Regressão Codegen C#** | `TestesCodegenCSharp1.py` a `TestesCodegenCSharp6.py` | Emissão `--cs`, compilação C# com 0 erros e execução comparada com `python3` (incluindo `stdin` simulado em `TestesCodegenCSharp6.py`). |
| **Pipeline completo de fases** | `TestesCodegenCSharp4.py` | Execução com `--ast`, `--tabela`, `--tac` e `--opt`, verificando que a otimização reduz o número de instruções TAC. |
| **Bloqueio em erro semântico** | `TestesSemanticaErros5.py` | Confirma que `./compilador --cs` retorna código ≠ 0, reporta `Erro semântico` e não gera arquivo C#. |
| **Guarda de passo zero em runtime** | `TestesCodegenCSharpErroPassoZero.py` | Confirma que `range(..., 0)` falha com `ValueError` no `python3` e com `ArgumentException("range() arg 3 must not be zero")` no C# compilado. |

---

## Como Executar

### Pré-requisitos

- Compilador já compilado (`make`).
- Um compilador C# disponível no ambiente:
  - **.NET SDK (`dotnet`)** — Linux, macOS ou Windows;
  - **Mono (`mcs`)** — Linux;
  - **C# Compiler do .NET Framework (`csc.exe`)** ou `dotnet.exe` — detectados automaticamente no WSL.
- **`python3`** (recomendado para comparação direta contra o interpretador Python; os arquivos `.out` garantem a validação da suíte `testes/e2e/` mesmo sem `python3`).

### Comandos

```bash
# Via Makefile (compila o compilador e executa toda a suíte E2E):
make test_e2e

# Ou executando o script diretamente:
./testes/rodar_testes_e2e.sh

# Opcional: forçar o compilador C# usado (dotnet, mcs, csc ou dotnet_win):
CSHARP_MODO=dotnet ./testes/rodar_testes_e2e.sh
```

> **Compatibilidade:** o script `testes/rodar_e2e.sh` é mantido como atalho que delega diretamente para `testes/rodar_testes_e2e.sh`.

---

## Resultado Obtido

```text
=========================================
   Testes End-to-End (Mini-Python -> C#)
   Compilador C#: csc
=========================================

--- 1. Suite E2E dedicada (testes/e2e/) ---
[OK]    01_hello_world.py - --cs stdout == arquivo e 0 avisos nao traduzidos
[OK]    01_hello_world.py - compilacao C# (csc) com 0 erros
[OK]    01_hello_world.py - saida C# corresponde ao esperado (01_hello_world.out)
[OK]    01_hello_world.py - execucao C# identica ao python3 (exit 0)
[OK]    02_variaveis_aritmetica.py - --cs stdout == arquivo e 0 avisos nao traduzidos
[OK]    02_variaveis_aritmetica.py - compilacao C# (csc) com 0 erros
[OK]    02_variaveis_aritmetica.py - saida C# corresponde ao esperado (02_variaveis_aritmetica.out)
[OK]    02_variaveis_aritmetica.py - execucao C# identica ao python3 (exit 0)
[OK]    03_condicionais.py - --cs stdout == arquivo e 0 avisos nao traduzidos
[OK]    03_condicionais.py - compilacao C# (csc) com 0 erros
[OK]    03_condicionais.py - saida C# corresponde ao esperado (03_condicionais.out)
[OK]    03_condicionais.py - execucao C# identica ao python3 (exit 0)
[OK]    04_lacos.py - --cs stdout == arquivo e 0 avisos nao traduzidos
[OK]    04_lacos.py - compilacao C# (csc) com 0 erros
[OK]    04_lacos.py - saida C# corresponde ao esperado (04_lacos.out)
[OK]    04_lacos.py - execucao C# identica ao python3 (exit 0)
[OK]    05_funcoes.py - --cs stdout == arquivo e 0 avisos nao traduzidos
[OK]    05_funcoes.py - compilacao C# (csc) com 0 erros
[OK]    05_funcoes.py - saida C# corresponde ao esperado (05_funcoes.out)
[OK]    05_funcoes.py - execucao C# identica ao python3 (exit 0)
[OK]    06_programa_completo_especificacao.py - --cs stdout == arquivo e 0 avisos nao traduzidos
[OK]    06_programa_completo_especificacao.py - compilacao C# (csc) com 0 erros
[OK]    06_programa_completo_especificacao.py - saida C# corresponde ao esperado (06_programa_completo_especificacao.out)
[OK]    06_programa_completo_especificacao.py - execucao C# identica ao python3 (exit 0)
[OK]    07_entrada_interativa.py - --cs stdout == arquivo e 0 avisos nao traduzidos
[OK]    07_entrada_interativa.py - compilacao C# (csc) com 0 erros
[OK]    07_entrada_interativa.py - saida C# corresponde ao esperado (07_entrada_interativa.out)
[OK]    07_entrada_interativa.py - execucao C# identica ao python3 (exit 0)

--- 2. Suite de regressao Codegen C# (TestesCodegenCSharp1..6) ---
[OK]    TestesCodegenCSharp1.py - --cs stdout == arquivo e 0 avisos nao traduzidos
[OK]    TestesCodegenCSharp1.py - compilacao C# (csc) com 0 erros
[OK]    TestesCodegenCSharp1.py - execucao C# (exit 0)
[OK]    TestesCodegenCSharp2.py - --cs stdout == arquivo e 0 avisos nao traduzidos
[OK]    TestesCodegenCSharp2.py - compilacao C# (csc) com 0 erros
[OK]    TestesCodegenCSharp2.py - execucao C# identica ao python3 (exit 0)
[OK]    TestesCodegenCSharp3.py - --cs stdout == arquivo e 0 avisos nao traduzidos
[OK]    TestesCodegenCSharp3.py - compilacao C# (csc) com 0 erros
[OK]    TestesCodegenCSharp3.py - execucao C# identica ao python3 (exit 0)
[OK]    TestesCodegenCSharp4.py - --cs stdout == arquivo e 0 avisos nao traduzidos
[OK]    TestesCodegenCSharp4.py - compilacao C# (csc) com 0 erros
[OK]    TestesCodegenCSharp4.py - execucao C# identica ao python3 (exit 0)
[OK]    TestesCodegenCSharp5.py - --cs stdout == arquivo e 0 avisos nao traduzidos
[OK]    TestesCodegenCSharp5.py - compilacao C# (csc) com 0 erros
[OK]    TestesCodegenCSharp5.py - execucao C# identica ao python3 (exit 0)
[OK]    TestesCodegenCSharp6.py - --cs stdout == arquivo e 0 avisos nao traduzidos
[OK]    TestesCodegenCSharp6.py - compilacao C# (csc) com 0 erros
[OK]    TestesCodegenCSharp6.py - execucao C# identica ao python3 (exit 0)

--- 3. Pipeline completo (--ast, --tabela, --tac vs --opt) e bloqueio semantico ---
[OK]    TestesCodegenCSharp4.py (--ast) - exit 0 com Programa/If/While/For
[OK]    TestesCodegenCSharp4.py (--tabela) - exit 0 com simbolos esperados
[OK]    TestesCodegenCSharp4.py (--tac 217 vs --opt 188 linhas)
[OK]    TestesSemanticaErros5.py (--cs bloqueado com Erro semantico e sem gerar C#)

--- 4. Guarda de passo zero em runtime ---
[OK]    TestesCodegenCSharpErroPassoZero.py - --cs stdout == arquivo e 0 avisos nao traduzidos
[OK]    TestesCodegenCSharpErroPassoZero.py - compilacao C# com 0 erros
[OK]    TestesCodegenCSharpErroPassoZero.py - python3 falhou com ValueError (rc=1)
[OK]    TestesCodegenCSharpErroPassoZero.py - C# falhou com ArgumentException (rc=82)

=========================================
   Resultado E2E: 54 passaram, 0 falharam
=========================================
```
