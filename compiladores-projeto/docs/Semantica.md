# Análise Semântica

A fase de Análise Semântica do compilador Python-C# é responsável por garantir que as construções sintáticas (que já foram validadas pelo Parser e transformadas em uma AST) façam sentido dentro das regras da linguagem.

Atualmente, o foco principal da nossa análise semântica engloba a **Verificação de Declaração de Variáveis**, o **Gerenciamento de Escopos** e a **Inferência e Verificação de Tipos**.

## Verificação de Variáveis

O nosso analisador percorre a Árvore Sintática Abstrata (AST) procurando por operações envolvendo variáveis:

- **Atribuição (`NO_ASSIGN`):** Quando uma atribuição é encontrada (ex: `x = 10`), o analisador semântico infere o tipo do valor e insere a variável "x" na [Tabela de Símbolos](TabelaSimbolos.md) utilizando a função `tabela_inserir`. Se a variável já existir no escopo atual, seu tipo é dinamicamente atualizado, permitindo que mude de tipo conforme a execução (ex: de int para string).
- **Uso de Variável (`NO_ID`):** Quando uma variável é lida (por exemplo, dentro de uma expressão ou em um `print(x)`), o analisador verifica se ela já foi declarada consultando a tabela de símbolos através de `tabela_buscar`. Se a variável não existir na tabela (nem no escopo atual, nem nos escopos pais), o compilador emite um erro semântico indicando a linha em que o uso irregular ocorreu:
  
  ```text
  Erro semântico na linha X: Variável 'Y' não declarada.
  ```

## Gerenciamento de Escopos

A verificação semântica trabalha em estreita colaboração com o modelo de ambientes (environments) da tabela de símbolos para tratar blocos de código:

- **Aninhamento (`NO_BLOCO`):** Sempre que a análise adentra um bloco de código recuado (como o corpo de um `if`, `while`, `for`, ou definição de função), ela invoca a função `tabela_entrar_escopo()`.
- **Visibilidade Ascendente:** O analisador permite o uso de variáveis declaradas em um bloco pai. A busca da tabela sobe na hierarquia, tornando as globais ou variáveis externas acessíveis.
- **Encapsulamento Descendente:** Variáveis declaradas dentro do escopo interno morrem com ele e não "vazam" para o escopo externo. Ao finalizar a visita aos filhos de um `NO_BLOCO`, o analisador invoca `tabela_sair_escopo()` para destruir referências às variáveis daquele bloco.

## Inferência e Verificação de Tipos

Além de gerenciar variáveis, o módulo garante a integridade das operações e anota o tipo de cada nó na árvore sintática abstrata (usando o campo `tipo_resolvido` na `NoAST`), fundamental para a Geração de Código.

O motor conta com a função `inferir_tipo`, que percorre a árvore de baixo para cima resolvendo as seguintes regras:

- **Literais e Variáveis:** Identifica nativamente se um valor é inteiro, float, string ou booleano, e consulta a Tabela de Símbolos para resgatar o tipo de variáveis sendo utilizadas.
- **Operações Aritméticas (`+`, `-`, `*`, `/`):** Verifica se os operandos são numéricos, recusando operações com strings ou booleanos. Se a operação envolver um inteiro e um float, o analisador automaticamente **promove** o resultado da expressão para float (`cast` implícito).
- **Operações Relacionais e Lógicas (`==`, `<`, `and`, etc.):** Comparações verificam a compatibilidade dos operandos (barrando comparação de string com número, por exemplo) e garantem que o nó da AST resultante sempre receba o `TIPO_BOOL`.
- **Condicionais Estritos (`if` e `while`):** O analisador obriga que a expressão dentro de um laço ou condição seja puramente booleana. Ao contrário de linguagens permissivas (que aceitariam `if "texto"`), nosso compilador exige `TIPO_BOOL` e lança um erro semântico direto se isso for violado.
- **Funções:** Chamadas de função (`NO_FUNCCALL`) e retornos (`NO_RETURN`) também passam pela inferência para garantir rastreabilidade dos tipos devolvidos e utilizados ao longo do programa.

## Testes (TDD)

Toda essa implementação foi orientada a testes unitários (TDD). Para consultar e rodar os testes semânticos:
```bash
make test_semantica
```
Esses testes simulam árvores AST básicas e atestam que os cenários de sucesso passam e variáveis com escopo incorreto (ou não declaradas) disparam erros precisamente contabilizados pela API.

## API do Módulo

O módulo semântico (`src/semantica.c` e `src/semantica.h`) tem como principal interface de comunicação a seguinte função:

```c
int analisar_semantica(NoAST *raiz, TabelaSimbolos *tabela);
```

- **Entrada:** O nó raiz da sub-árvore ou programa e um ponteiro para a `TabelaSimbolos`.
- **Saída (Retorno):** Um inteiro com a quantidade total de erros semânticos detectados. Se o retorno for `0`, significa que o programa é semanticamente válido neste escopo de verificação.

Internamente, utiliza também:

```c
TipoDado inferir_tipo(NoAST *raiz, TabelaSimbolos *tabela, int *erros);
```
Que avalia uma expressão, anota o nó da AST e soma na contagem caso encontre quebras de tipagem.
