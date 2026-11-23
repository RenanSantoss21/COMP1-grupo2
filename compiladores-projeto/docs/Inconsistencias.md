# Inconsistências entre Documentação e Código

Levantamento das divergências entre a documentação do projeto e o comportamento real do compilador, feito em 13/11/2026.

**Como foi feito:** leitura integral dos 22 arquivos de documentação (`docs/`, READMEs, `CONTEXT.md`, `CODING_STANDARDS.md` e material de apresentação). Cada afirmação verificável foi testada rodando o `./compilador`; os 17 exemplos do [Catálogo de Erros Semânticos](ErrosSemanticos.md) foram executados um a um e comparados com a saída real.

**Estado de referência do código:** 69 testes unitários (6 suítes) e 72 testes de integração, todos passando.

| Categoria | Itens |
|---|---|
| [1. Comportamento documentado que não acontece](#1-comportamento-documentado-que-nao-acontece) | 8 |
| [2. Documentação desatualizada](#2-documentacao-desatualizada) | 11 |
| [3. Links e navegação](#3-links-e-navegacao) | 3 |
| [4. Documentos que se contradizem](#4-documentos-que-se-contradizem) | 2 |
| [5. Já corrigido](#5-ja-corrigido) | 7 |

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

### D1 — README interno parado na Sprint 1
- **Onde:** `compiladores-projeto/README.md`
- **O doc diz:** `src/` está "reservado", há "5 arquivos de teste" e os testes cobrem só o lexer.
- **Na prática:** `src/` tem 6 módulos, `testes/` tem 39 arquivos `.py` e há 6 suítes unitárias.
- **Correção sugerida:** reescrever ou apontar para o README da raiz.

### D2 — README da raiz sem TAC, otimizador e C#
- **Onde:** `README.md:19-26` e `83-87`
- **O doc diz:** `src/` tem "AST, Tabela de Símbolos, Análise Semântica"; testes unitários só desses três; exercícios em "`semana 01` a `semana 06`".
- **Na prática:** faltam `codegen_intermediario`, `otimizador` e `codegen_csharp` e os alvos `test_tac`, `test_otimizador` e `test_codegen_csharp`. As aulas ficam em `Aulas/semana 01` a `semana 10`.
- **Correção sugerida:** atualizar as duas listas e o caminho das aulas.

### D3 — Flags de linha de comando incompletas
- **Onde:** `docs/Parser.md:192`, `README.md`, `docs/index.md`
- **O doc diz:** o executável aceita `--ast`, `--dot` e `--tabela`. O README e o index não citam flags.
- **Na prática:** existem também `--tac`, `--opt` e `--cs [arquivo]`.
- **Correção sugerida:** uma tabela única de flags no README.

### D4 — Guia do Makefile sem os alvos novos
- **Onde:** `docs/Makefile.md:29`, `39-44` e `58`
- **O doc diz:** `make` compila `ast`, `tabela`, `semantica` e `codegen_intermediario`; `make test` roda a integração antes do `test_tac`.
- **Na prática:** o build inclui também `otimizador.c` e `codegen_csharp.c`. A ordem real é ast, tabela, semantica, tac, otimizador, codegen_csharp e integração. Os alvos `test_otimizador` e `test_codegen_csharp`, e os binários deles no `make clean`, não aparecem.
- **Correção sugerida:** atualizar as três listas.

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

### D10 — Resultado obtido antigo nos relatórios léxico e sintático
- **Onde:** `docs/Testes_Lexicos.md:190`, `docs/Testes_Sintaticos.md:190`
- **O doc diz:** "Resultado: 39 passaram, 0 falharam".
- **Na prática:** 72 passam. As mensagens de erro léxico e sintático citadas foram conferidas e continuam corretas.
- **Correção sugerida:** atualizar o bloco, ou mostrar só o trecho da categoria de cada relatório, para não envelhecer a cada teste novo.

### D11 — Contagem e lista dos testes de integração semântica
- **Onde:** `docs/Semantica.md:102-133`
- **O doc diz:** "16 arquivos de testes de integração semântica"; a tabela lista 21.
- **Na prática:** são 23. A tabela omite `TestesSemanticaErros10.py` e `TestesSemanticaErros11.py`.
- **Correção sugerida:** completar a tabela e trocar o número fixo por "a tabela abaixo".

### D12 — Números e issues da Sprint 4
- **Onde:** `docs/Sprints.md:32`, `34` e `35`
- **O doc diz:** "39 cenários" de integração; catálogo "`SEM-01` a `SEM-14`"; *forward references* nas "Issues #49, #51".
- **Na prática:** são 72 cenários e o catálogo vai até `SEM-17`. A Issue #51 é a política de reatribuição (ADR-001), não *forward references*.
- **Correção sugerida:** ajustar os números e mover a #51 para o item de reatribuição.

### D13 — Material de apresentação com números antigos
- **Onde:** `apresentacao_p1.md`, `roteiro_topico5_testes_infra.md`
- **O doc diz:** 59 testes (P1) e 66 testes, "conferidos em 27/09/2026" (roteiro).
- **Na prática:** são 69 testes unitários e 72 de integração. Os dois documentos são registros de apresentações já feitas.
- **Correção sugerida:** não precisa reescrever; no máximo, uma linha no topo indicando que são registros datados.

---

## 3. Links e navegação

Afetam o site publicado pelo MkDocs.

### N1 — Páginas fora do menu
- **Onde:** `mkdocs.yml`
- **O doc diz:** a seção "Decisões de Design (ADRs)" do menu aponta só para a Especificação da Linguagem.
- **Na prática:** `ErrosSemanticos.md` e `ADR/001-reatribuicao-de-tipo.md` existem, mas não estão no `nav` e não aparecem no site.
- **Correção sugerida:** adicionar as duas ao `nav` (a ADR dentro da seção de ADRs).

### N2 — Link quebrado para o `CODING_STANDARDS.md`
- **Onde:** `docs/ADR/001-reatribuicao-de-tipo.md:12`
- **O doc diz:** link para `../../CODING_STANDARDS.md`.
- **Na prática:** o caminho resolve para `compiladores-projeto/CODING_STANDARDS.md`, que não existe. O arquivo está na raiz do repositório (`../../../`).
- **Correção sugerida:** corrigir o caminho ou usar o link do GitHub.

### N3 — Links para código fora de `docs/`
- **Onde:** vários arquivos de `docs/`
- **O doc diz:** links como `../src/ast.h`, `../parser/parser.y` e `../testes/tdd/...`.
- **Na prática:** o MkDocs publica só o conteúdo de `docs/`, então esses links não abrem no site; funcionam apenas na navegação pelo GitHub. (Não verificado no site publicado.)
- **Correção sugerida:** trocar por links absolutos para os arquivos no GitHub.

---

## 4. Documentos que se contradizem

Precisam de uma decisão do grupo antes de mexer no texto.

### X1 — Estrutura do arquivo C#
- **Onde:** Issues #14 e #29, `CODING_STANDARDS.md`, `docs/Especificacao_Linguagem.md`
- **O doc diz:** as issues mostram `class Program` sem namespace; o `CODING_STANDARDS.md` pede namespace e classe estática; a especificação usa `namespace CompiladorPython` com `static class Program`.
- **Na prática:** o gerador segue o `CODING_STANDARDS.md` com o nome da especificação ([Geração de Código C#](CodegenCSharp.md)). Só os textos das issues ficaram diferentes.
- **Correção sugerida:** registrar nas issues a estrutura adotada.

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

### R8 — Comentário de bloco `/* */` removido do lexer (X2 / Issue #55)
- **Estava:** O lexer aceitava comentários de bloco no estilo C (`/* ... */`), que não fazem parte do Python.
- **Ficou:** Regra removida de `lexer/lexer.l` e documentado em `docs/Lexer.md` e `docs/Especificacao_Linguagem.md` que apenas `#` é suportado.

### R9 — Origem da linha dos nós via locations do Bison (D6 / Issue #55)
- **Estava:** `docs/AST.md` e `docs/Lexer.md` diziam que os nós recebiam a variável `linha_token`.
- **Ficou:** Documentado o uso de `%locations`, `yylloc` e `@N.first_line` do Bison nos dois documentos.
