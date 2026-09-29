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
Este é o comando padrão. Ele compila o analisador léxico (`lexer/lexer.l`), o analisador sintático (`parser/parser.y`) e os módulos do código-fonte em C localizados na pasta `src/` (como `ast.c` e `tabela.c`), e gera o executável final chamado `compilador` na raiz do projeto.

---

### Rodar Todos os Testes
```bash
make test
```
Executa a suíte de testes de ponta a ponta. Ele faz isso executando na seguinte ordem:
1. **`test_ast`**: Compila e roda os testes unitários do módulo da Árvore Sintática Abstrata (AST).
2. **`test_tabela`**: Compila e roda os testes unitários da Tabela de Símbolos.
3. **`test_integracao`**: Compila o executável principal `compilador` e em seguida utiliza o script `./testes/rodar_testes.sh` para testar os cenários de sucesso e falha sintática usando os arquivos `.py` na pasta de testes.

O teste vai parar imediatamente caso alguma etapa venha a falhar.

---

### Limpar Arquivos Gerados
```bash
make clean
```
Limpa o repositório, removendo todos os arquivos gerados no processo de build e de testes. Isso inclui:
- O executável principal `compilador`.
- Os arquivos autogerados pelo Flex e Bison (`lex.yy.c`, `parser.tab.c`, `parser.tab.h`).
- Os binários e arquivos temporários de teste (como `teste_tabela`, `teste_ast` e `_ast_test_output.tmp`).

## Comandos Específicos (Desenvolvimento)

Durante o desenvolvimento de uma feature isolada, você pode não querer rodar todos os testes. O `Makefile` provê alvos específicos para testes menores:

### Testar apenas AST
```bash
make test_ast
```
Compila o arquivo de testes da AST junto com o `src/ast.c` e mostra os resultados de apenas essa suite.

### Testar apenas a Tabela de Símbolos
```bash
make test_tabela
```
Compila o arquivo de testes da Tabela de Símbolos junto com o `src/tabela.c` e mostra os resultados de apenas essa suite.

### Testar apenas Integração
```bash
make test_integracao
```
Compila o `compilador` e roda exclusivamente o script bash `rodar_testes.sh`, pulando os testes unitários da AST e da tabela em C.
