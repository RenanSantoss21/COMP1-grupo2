# Inconsistências entre Documentação e Código

Levantamento das divergências entre a documentação do projeto e o comportamento real do compilador, feito em 13/11/2026.

**Como foi feito:** leitura integral dos 22 arquivos de documentação (`docs/`, READMEs, `CONTEXT.md`, `CODING_STANDARDS.md` e material de apresentação). Cada afirmação verificável foi testada rodando o `./compilador`; os 17 exemplos do [Catálogo de Erros Semânticos](ErrosSemanticos.md) foram executados um a um e comparados com a saída real.

**Estado de referência do código:** 69 testes unitários (6 suítes) e 72 testes de integração, todos passando.

| Categoria | Itens |
|---|---|
| [1. Comportamento documentado que não acontece](#1-comportamento-documentado-que-nao-acontece) | 8 |
| [2. Documentação desatualizada](#2-documentacao-desatualizada) | 2 |
| [3. Links e navegação](#3-links-e-navegacao) | 0 |
| [4. Documentos que se contradizem](#4-documentos-que-se-contradizem) | 0 |
| [5. Já corrigido](#5-ja-corrigido) | 21 |

---

## 1. Comportamento documentado que não acontece

São os casos mais graves: ou o documento está errado, ou o código tem um bug. Em cada um é preciso decidir qual dos dois corrigir.

### C1 — Atribuição encadeada `a = b = 0`
- **Onde:** `docs/Parser.md:101`
- **O doc diz:** o `=` é associativo à direita, "permitindo `a = b = 0`".
- **Na prática:** o parser aceita, mas o `b` nunca é declarado: um `print(b)` depois dá "Variável 'b' não declarada", e `a` fica com tipo desconhecido.
- **Correção sugerida:** tratar `NO_ASSIGN` como expressão na semântica, ou remover a frase do doc e rejeitar o encadeamento.

### C2 — Atribuição a algo que não é variável
- **Onde:** `docs/Parser.md:197`
- **O doc diz:** o `=` "valida se o operando à esquerda é um identificador".
- **Na prática:** `1 = 2` é aceito sem erro e cria na tabela uma variável chamada `invalido` (`parser/parser.y:221`).
- **Correção sugerida:** emitir erro quando o lado esquerdo não for `NO_ID`.

### C3 — `range` com 1, 2 ou 3 argumentos
- **Onde:** `docs/Parser.md:196`, `docs/Especificacao_Linguagem.md:16`
- **O doc diz:** o `for` aceita `range` com 1, 2 ou 3 argumentos.
- **Na prática:** `range()` e `range(1, 2, 3, 4)` também são aceitos, sem erro.
- **Correção sugerida:** validar a quantidade de argumentos do `range` na semântica (novo código SEM).

### C5 — Listas fazem parte da linguagem
- **Onde:** `docs/Especificacao_Linguagem.md:13`, `docs/Parser.md:105`, `CODING_STANDARDS.md`
- **O doc diz:** listas como `[]` e `[1, 2, 3]` são tipos suportados, traduzidos para `List<T>`.
- **Na prática:** não existe nó de lista. `[1, 2, 3]` vira um `NO_BLOCO` sem tipo e sem número de linha; a semântica não tipa listas e o gerador C# não as traduz.
- **Correção sugerida:** criar `NO_LISTA` e o tipo correspondente, ou retirar listas da especificação.

### C6 — Argumentos do `print` não têm o tipo verificado
- **Onde:** `docs/Semantica.md` (Inferência e Verificação de Tipos), `docs/ErrosSemanticos.md`
- **O doc diz:** operações com tipos incompatíveis geram erro semântico, sem exceções.
- **Na prática:** `NO_PRINT` cai no percurso genérico, que só verifica se as variáveis existem. `print("a" + 1)` é aceito sem erro.
- **Correção sugerida:** chamar `inferir_tipo` em cada argumento do `print` (Issue #54).

### C7 — Exemplo da SEM-04 não produz o erro documentado
- **Onde:** `docs/ErrosSemanticos.md`, SEM-04
- **O doc diz:** `print(True > "1")` produz "Comparação inválida envolvendo booleano."
- **Na prática:** não produz erro nenhum (ver C6). Mesmo com a verificação no `print`, o exemplo cairia na regra de string (SEM-03), que é testada antes da de booleano.
- **Correção sugerida:** trocar o exemplo por `x = True > 1`.

### C8 — Linha errada na saída da SEM-09
- **Onde:** `docs/ErrosSemanticos.md`, SEM-09
- **O doc diz:** "Erro semântico na linha 3: Número errado de argumentos."
- **Na prática:** o exemplo tem uma linha em branco antes da chamada, então o erro sai na linha 4.
- **Correção sugerida:** mudar a saída esperada para a linha 4.

### C10 — Teste documentado que nunca roda
- **Onde:** `docs/Semantica.md:130`, `testes/rodar_testes.sh`
- **O doc diz:** `TestesSemanticaErros9.py` faz parte da suíte (argumento incompatível com o tipo inferido do parâmetro).
- **Na prática:** o arquivo existe, mas o `rodar_testes.sh` não o chama. É o único `.py` de `testes/` fora do runner.
- **Correção sugerida:** adicionar `rodar_teste_semantico "testes/TestesSemanticaErros9.py" 1` ao runner.

---

## 2. Documentação desatualizada

O código evoluiu depois que o texto foi escrito. Nenhum destes casos muda o comportamento do compilador, mas eles confundem quem chega ao projeto.

### D5 — Cabeçalhos e não-terminais do `parser.y`
- **Onde:** `docs/Parser.md:15` e `20`
- **O doc diz:** o parser importa `ast.h`, `tabela.h` e `semantica.h`, e lista os `%type <ast>`.
- **Na prática:** também importa `codegen_intermediario.h`, `otimizador.h` e `codegen_csharp.h`. A lista de tipos omite `erro_linha` e `erro_indentacao`.
- **Correção sugerida:** completar as duas listas.

### D6 — Origem da linha dos nós da AST
- **Onde:** `docs/AST.md:425`, `docs/Lexer.md:176`
- **O doc diz:** `linha_token` é repassada a todos os `criar_no_*`; o `YY_USER_ACTION` só atualiza `linha_token`.
- **Na prática:** desde a Issue #46 o parser usa `@N.first_line`, e o `YY_USER_ACTION` também preenche `yylloc`.
- **Correção sugerida:** descrever o uso de *locations* do Bison nos dois documentos.

### D9 — Testes unitários e resultado da suíte semântica
- **Onde:** `docs/Testes_Semanticos.md`
- **O doc diz:** 8 testes unitários de semântica; resultado "41 passaram".
- **Na prática:** são 12 testes. Faltam `test_contagem_nao_declarada`, `test_contagem_argumentos_de_chamada`, `test_inferencia_parametros_e_void` e `test_politica_reatribuicao_tipos`. A integração passa 72.
- **Correção sugerida:** completar a Parte 1 e atualizar a saída do runner.

---

## 3. Links e navegação

Todos os 3 itens desta categoria (`N1`, `N2` e `N3`) foram resolvidos nas Issues #20 e #59 (ver **R17**, **R20** e **R21** na [seção 5](#5-ja-corrigido)).

---

## 4. Documentos que se contradizem

Precisam de uma decisão do grupo antes de mexer no texto.

### X2 — Comentário de bloco `/* */` não é Python
- **Onde:** `lexer/lexer.l:135`, `docs/Lexer.md:111`, `docs/Especificacao_Linguagem.md`
- **O doc diz:** o `Lexer.md` documenta comentários `/* ... */`; a especificação, que define a linguagem fonte, não os menciona.
- **Na prática:** o lexer aceita `/* */`, que é inválido em Python. Um programa que usa esse comentário compila aqui e falha no Python.
- **Correção sugerida:** decidir se é uma extensão da linguagem (e registrar na especificação) ou remover a regra do lexer.

---

## 5. Já corrigido

### R1 — Rastreio do programa de exemplo em `VidaDeUmPrograma.md`
- **Estava:** o `if` abria escopo, o `y` era apagado e o exemplo terminava em "Variável 'y' não declarada", contradizendo a própria seção 4 do documento. Tipos `DESCONHECIDO` por toda parte e as Issues #39, #46 e #52 descritas como abertas.
- **Ficou:** seções 3, 4 e 5 reescritas a partir da saída real do `--ast`, do `--dot` e do `--tabela`: sem escopo de bloco, `n` inferido como `int`, programa aceito com código de saída 0.

### R2 — Nome do namespace no C# gerado
- **Estava:** o gerador usava um nome de namespace diferente do exemplo da especificação.
- **Ficou:** `namespace CompiladorPython`, como em `docs/Especificacao_Linguagem.md`.

### R3 — Escopo de variáveis em blocos no C# gerado
- **Estava:** a página do gerador dizia que variáveis de bloco deixam de existir no `}`, contrariando a Issue #45.
- **Ficou:** segue a Issue #45: variáveis atribuídas dentro de blocos serão declaradas no início do método, com `default(...)` (ver [Geração de Código C#](CodegenCSharp.md)).

### R4 — Chamada antes do `def` no nível do módulo (C4 / Issue #57)
- **Estava:** `x = g()` no nível global antes de `def g(): return 1` era aceito por causa do pré-registro em duas passadas.
- **Ficou:** quando `!tabela->dentro_de_funcao` e `sim->pre_registrada`, `NO_FUNCCALL` reporta `"Chamada a função não declarada"` (mantendo *forward references* válidas dentro do corpo de funções).

### R5 — `tipo_resolvido` em `NO_FUNCDEF` e `NO_RETURN` (C9 / Issue #57)
- **Estava:** o tipo de retorno só era salvo na tabela de símbolos; `NO_FUNCDEF` e `NO_RETURN` ficavam com `TIPO_DESCONHECIDO` na AST.
- **Ficou:** `semantica.c` preenche `raiz->tipo_resolvido` tanto em `NO_FUNCDEF` quanto em `NO_RETURN`, e `ast.c` exibe o tipo em `--ast` e `--dot`.

### R6 — Imagens da AST atualizadas com os tipos inferidos (D7 / Issue #57)
- **Estava:** `docs/img/ast_def_chamada.png` e `docs/img/ast_vida_programa.png` não exibiam os tipos inferidos nos parâmetros, retorno e chamadas de função.
- **Ficou:** imagens regeneradas a partir da saída atual de `./compilador --dot`.

### R7 — Struct, API e exemplo em `docs/TabelaSimbolos.md` (D8 / Issue #57)
- **Estava:** faltavam `pre_registrada` na struct `Simbolo` e `tabela_buscar_escopo_atual` na tabela da API; o texto dizia que blocos abrem escopo e o exemplo registrava `x` antes do pré-registro de `calcular`.
- **Ficou:** documentação de `TabelaSimbolos.md` alinhada com a implementação atual.

### R8 — README interno atualizado (`compiladores-projeto/README.md`) (D1 / Issue #58)
- **Estava:** parado na Sprint 1, citando `src/` reservado, 5 arquivos de teste e cobertura apenas do lexer.
- **Ficou:** reescrito com os 6 módulos em `src/`, todas as suítes unitárias e de integração, alvos do `Makefile` e referência ao README da raiz e à pasta `docs/`.

### R9 — README da raiz atualizado (`README.md`) (D2 / Issue #58)
- **Estava:** sem menção a TAC, otimizador e gerador C# na estrutura e nos testes, e listando `semana 01` a `semana 06`.
- **Ficou:** atualizados o resumo do compilador, a estrutura (`src/`, `testes/`, `testes/tdd/`), o caminho `Aulas/semana 01` a `semana 10` e todos os alvos de teste (`make test`, `test_tac`, `test_otimizador`, `test_codegen_csharp`).

### R10 — Tabela completa de flags de CLI (D3 / Issue #58)
- **Estava:** `docs/Parser.md` listava apenas `--ast`, `--dot` e `--tabela`; `README.md` e `docs/index.md` não documentavam as flags.
- **Ficou:** tabela completa com `--ast`, `--dot`, `--tabela`, `--tac`, `--opt` e `--cs [arquivo]` adicionada em `README.md` e `docs/index.md`, e lista atualizada em `docs/Parser.md`.

### R11 — Guia do Makefile atualizado (`docs/Makefile.md`) (D4 / Issue #58)
- **Estava:** omitia `otimizador.c` e `codegen_csharp.c` no build, apresentava ordem antiga em `make test` e não listava `test_otimizador` nem `test_codegen_csharp` (inclusive no `make clean`).
- **Ficou:** alinhado ao `Makefile` atual (build com os 6 módulos, ordem real de `make test`, seções individuais para todos os alvos e lista completa do `make clean`).

### R12 — Resultado obtido nos relatórios léxico e sintático (D10 / Issue #58)
- **Estava:** `docs/Testes_Lexicos.md` e `docs/Testes_Sintaticos.md` reproduziam a saída inteira antiga do runner ("Resultado: 39 passaram, 0 falharam").
- **Ficou:** cada relatório exibe apenas o trecho correspondente à sua categoria (léxico em `Testes_Lexicos.md`; sintático e recuperação de erros em `Testes_Sintaticos.md`), evitando defasagem a cada novo teste.

### R13 — Números e issues da Sprint 4 (`docs/Sprints.md`) (D12 / Issue #58)
- **Estava:** citava "39 cenários", catálogo até `SEM-14` e a Issue #51 em *forward references*.
- **Ficou:** catálogo corrigido para `SEM-01` a `SEM-17`, Issue #51 movida para o item de reatribuição (ADR-001) e descrição da suíte no `rodar_testes.sh` sem contagem fixa desatualizada.

### R14 — Material de apresentação (`apresentacao_p1.md`, `roteiro_topico5_testes_infra.md`) (D13 / Issue #58)
- **Situação:** os arquivos de apresentação citados no levantamento não estão versionados neste repositório; por serem registros históricos externos de apresentações já concluídas, nenhuma alteração foi necessária no repositório.

### R15 — Alinhamento da estrutura do arquivo C# (X1 / Issue #58)
- **Estava:** `CODING_STANDARDS.md` pedia apenas "um `namespace` e uma classe `Program` estática", sem explicitar o nome do namespace adotado.
- **Ficou:** `CODING_STANDARDS.md` atualizado para especificar `namespace CompiladorPython` e `static class Program`, alinhado a `docs/Especificacao_Linguagem.md`, `docs/CodegenCSharp.md` e `src/codegen_csharp.c` (restando apenas registrar em comentário nas Issues #14 e #29 no GitHub).

### R16 — Tabela de testes de integração em `docs/Semantica.md` (D11 / Issue #20)
- **Estava:** citava "16 arquivos de testes de integração semântica" e omitia `TestesSemanticaErros10.py`, `TestesSemanticaErros11.py` e `TestesSemanticaErros12.py`.
- **Ficou:** texto ajustado para referenciar as tabelas sem contagem fixa desatualizada, com os três arquivos incluídos e link para o `Catálogo de Erros Semânticos`.

### R17 — Navegação do MkDocs para Semântica, ADR-001 e TAC (N1 / Issues #20 e #59)
- **Estava:** `ErrosSemanticos.md` e `ADR/001-reatribuicao-de-tipo.md` estavam fora do `nav` do `mkdocs.yml`, e o módulo de Código Intermediário (TAC) ainda não tinha página própria.
- **Ficou:** adicionados ao `nav` em `mkdocs.yml` (`Catálogo de Erros Semânticos`, `ADR-001` e a nova página `CodigoIntermediario.md`).

### R18 — Comentário de bloco `/* */` removido do lexer (X2 / Issue #55)
- **Estava:** O lexer aceitava comentários de bloco no estilo C (`/* ... */`), que não fazem parte do Python.
- **Ficou:** Regra removida de `lexer/lexer.l` e documentado em `docs/Lexer.md` e `docs/Especificacao_Linguagem.md` que apenas `#` é suportado.

### R19 — Origem da linha dos nós via locations do Bison (D6 / Issue #55)
- **Estava:** `docs/AST.md` e `docs/Lexer.md` diziam que os nós recebiam a variável `linha_token`.
- **Ficou:** Documentado o uso de `%locations`, `yylloc` e `@N.first_line` do Bison nos dois documentos.

### R20 — Link quebrado para `CODING_STANDARDS.md` na ADR-001 (N2 / Issue #59)
- **Estava:** `docs/ADR/001-reatribuicao-de-tipo.md:12` apontava para `../../CODING_STANDARDS.md`, resolvendo para o caminho inexistente `compiladores-projeto/CODING_STANDARDS.md`.
- **Ficou:** link corrigido para a URL absoluta do arquivo na raiz do repositório no GitHub (`https://github.com/RenanSantoss21/COMP1-grupo2/blob/main/CODING_STANDARDS.md`).

### R21 — Links para código fora de `docs/` substituídos por URLs do GitHub (N3 / Issue #59)
- **Estava:** 33 links relativos com `../` (como `../src/ast.h`, `../parser/parser.y` e `../testes/tdd/...`) em `AST.md`, `ErrosSemanticos.md`, `Especificacao_Linguagem.md`, `Parser.md`, `TabelaSimbolos.md` e `VidaDeUmPrograma.md` apontavam para fora da raiz publicada pelo MkDocs.
- **Ficou:** todos os 33 links convertidos para URLs absolutas em `https://github.com/RenanSantoss21/COMP1-grupo2/blob/main/compiladores-projeto/...`, preservando as imagens relativas (`img/*.png`), os links internos entre páginas de `docs/` e as âncoras `#...`.
