# Guia do Makefile

O projeto utiliza o **GNU Make** para automatizar a compilação e a execução dos testes em suas diversas etapas. Este documento descreve como utilizar os comandos disponíveis no `Makefile`.

## Requisitos

Para que os comandos funcionem, é necessário ter instalados:
- `gcc` (Compilador C)
- `flex` (Analisador léxico)
- `bison` (Analisador sintático)

No Linux (Ubuntu/Debian) isso pode ser instalado com:
```bash
sudo apt-get install build-essential flex bison
```

No Windows, recomenda-se o uso do WSL (Windows Subsystem for Linux) ou MSYS2.

## Comandos Principais

### Compilar o Projeto
```bash
make
# ou
make all
# ou
make compilador
```
Este é o comando padrão. Ele compila o analisador léxico (`lexer/lexer.l`), o analisador sintático (`parser/parser.y`) e todos os módulos em C localizados na pasta `src/` (`ast.c`, `tabela.c`, `semantica.c` e `codegen_intermediario.c`), gerando o executável final `compilador` na raiz do projeto.

> **Nota sobre Integração Contínua (CI):** Quando a variável de ambiente `CI` está definida (`make CI=1`), o `Makefile` adiciona automaticamente a flag `-Werror` ao `CFLAGS`, tratando qualquer *warning* de compilação como erro.

---

### Rodar Todos os Testes
```bash
make test
```
Executa toda a suíte de testes unitários (TDD) e de integração na seguinte ordem:
1. **`test_ast`**: Compila e executa os testes unitários da Árvore Sintática Abstrata (`testes/tdd/teste_ast.c`).
2. **`test_tabela`**: Compila e executa os testes unitários da Tabela de Símbolos (`testes/tdd/teste_tabela.c`).
3. **`test_semantica`**: Compila e executa os testes unitários da Análise Semântica (`testes/tdd/teste_semantica.c`).
4. **`test_integracao`**: Compila o executável principal `compilador` e roda o script `./testes/rodar_testes.sh` com todos os cenários léxicos, sintáticos, semânticos e de recuperação de erros usando os arquivos `.py` da pasta `testes/`.
5. **`test_tac`**: Compila e executa os testes unitários do gerador de Código Intermediário de Três Endereços (`testes/tdd/teste_tac.c`).

A execução para imediatamente caso qualquer etapa venha a falhar.

---

### Limpar Arquivos Gerados
```bash
make clean
```
Limpa o repositório, removendo todos os arquivos gerados no processo de build, visualização e testes. Isso inclui:
- O executável principal `compilador`.
- Os arquivos autogerados pelo Flex e Bison (`lex.yy.c`, `parser.tab.c`, `parser.tab.h`).
- Os arquivos de exportação visual da AST (`ast.dot`, `ast.png`).
- Os binários e arquivos temporários dos testes unitários (`teste_tabela`, `teste_ast`, `teste_semantica`, `teste_tac`, `_ast_test_output.tmp` e `_ast_test_dot.tmp`).

## Comandos Específicos (Desenvolvimento)

Durante o desenvolvimento de uma feature isolada, você pode rodar apenas o alvo correspondente ao módulo em que está trabalhando:

### Testar apenas a AST
```bash
make test_ast
```
Compila `testes/tdd/teste_ast.c` junto com `src/ast.c` e executa a suíte unitária da AST.

### Testar apenas a Tabela de Símbolos
```bash
make test_tabela
```
Compila `testes/tdd/teste_tabela.c` junto com `src/tabela.c` e executa a suíte unitária da Tabela de Símbolos.

### Testar apenas a Análise Semântica
```bash
make test_semantica
```
Compila `testes/tdd/teste_semantica.c` junto com `src/semantica.c`, `src/ast.c` e `src/tabela.c` e executa a suíte unitária semântica.

### Testar apenas o Código Intermediário (TAC)
```bash
make test_tac
```
Compila `testes/tdd/teste_tac.c` junto com `src/codegen_intermediario.c` e executa a suíte unitária do TAC.

### Testar apenas Integração
```bash
make test_integracao
```
Compila o `compilador` e roda exclusivamente o script `./testes/rodar_testes.sh`, pulando os testes unitários em C.
