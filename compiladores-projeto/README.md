# Projeto de Compiladores com Flex e Bison

Esta pasta contém o código-fonte, os testes e a documentação técnica do compilador de **Python** para **C#** implementado em C com **Flex** e **Bison**.

Para a visão geral do repositório (incluindo pré-requisitos e materiais de aula), consulte o [README da raiz](../README.md). A documentação completa das fases de compilação está em `docs/`.

---

## 🛠️ Estrutura de Diretórios

- `lexer/`: Analisador léxico em Flex (`lexer.l`).
- `parser/`: Analisador sintático em Bison (`parser.y`).
- `src/`: Módulos em C do compilador:
  - `ast.c` / `ast.h` — Árvore Sintática Abstrata (AST)
  - `tabela.c` / `tabela.h` — Tabela de Símbolos e escopos
  - `semantica.c` / `semantica.h` — Análise Semântica e inferência de tipos
  - `codegen_intermediario.c` / `codegen_intermediario.h` — Geração de Código Intermediário (TAC)
  - `otimizador.c` / `otimizador.h` — Otimizações sobre o TAC
  - `codegen_csharp.c` / `codegen_csharp.h` — Geração de Código Alvo (C#)
  - `tipos.h` — Tipos de dados compartilhados
- `testes/`: Arquivos `.py` de testes de integração (léxicos, sintáticos, semânticos e geração de C#), o script de integração (`rodar_testes.sh`) e o script de automação E2E (`rodar_testes_e2e.sh`).
- `testes/e2e/`: Programas de teste ponta a ponta (`.py`, `.in` e `.out`) que validam a tradução Mini-Python → C# compilado e executado.
- `testes/tdd/`: Suítes de testes unitários em C (`teste_ast.c`, `teste_tabela.c`, `teste_semantica.c`, `teste_tac.c`, `teste_otimizador.c` e `teste_codegen_csharp.c`).
- `docs/`: Documentação técnica em Markdown (publicada via MkDocs + GitHub Pages).
- `Makefile`: Automação de build, testes e limpeza.

---

## ⚙️ Como compilar e executar

```bash
make
./compilador < programa.py
```

Consulte a tabela completa de flags de linha de comando (`--ast`, `--dot`, `--tabela`, `--tac`, `--opt`, `--cs [arquivo]`) no [README da raiz](../README.md#flags-de-linha-de-comando) ou em [`docs/index.md`](docs/index.md).

### Rodando os Testes

```bash
make test                 # Roda todas as suítes unitárias e a integração
make test_integracao      # Roda apenas ./testes/rodar_testes.sh
make test_e2e             # Roda os testes ponta a ponta (Python -> C# compilado e executado)
make test_ast             # Testes unitários da AST
make test_tabela          # Testes unitários da Tabela de Símbolos
make test_semantica       # Testes unitários da Análise Semântica
make test_tac             # Testes unitários do Código Intermediário (TAC)
make test_otimizador      # Testes unitários do Otimizador TAC
make test_codegen_csharp  # Testes unitários do Gerador de Código C#
```

### Limpeza

```bash
make clean
```

---

## 👤 Integrantes

* Arthur Amaral
* Rafael Schetinger
* Andris
* Renan Santos
* Beatriz Brandão
