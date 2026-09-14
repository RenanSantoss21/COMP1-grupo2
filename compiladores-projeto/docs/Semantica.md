# Análise Semântica

A fase de Análise Semântica do compilador Python-C# é responsável por garantir que as construções sintáticas (que já foram validadas pelo Parser e transformadas em uma AST) façam sentido dentro das regras da linguagem.

Atualmente, o foco principal da nossa análise semântica é a **Verificação de Declaração de Variáveis** e o **Gerenciamento de Escopos**.

## Verificação de Variáveis

O nosso analisador percorre a Árvore Sintática Abstrata (AST) procurando por operações envolvendo variáveis:

- **Atribuição (`NO_ASSIGN`):** Quando uma atribuição é encontrada (ex: `x = 10`), o analisador semântico insere a variável "x" na [Tabela de Símbolos](TabelaSimbolos.md) utilizando a função `tabela_inserir`. Isso marca a variável como declarada no escopo atual.
- **Uso de Variável (`NO_ID`):** Quando uma variável é lida (por exemplo, dentro de uma expressão ou em um `print(x)`), o analisador verifica se ela já foi declarada consultando a tabela de símbolos através de `tabela_buscar`. Se a variável não existir na tabela (nem no escopo atual, nem nos escopos pais), o compilador emite um erro semântico indicando a linha em que o uso irregular ocorreu:
  
  ```text
  Erro semântico na linha X: Variável 'Y' não declarada.
  ```

## Gerenciamento de Escopos

A verificação semântica trabalha em estreita colaboração com o modelo de ambientes (environments) da tabela de símbolos para tratar blocos de código:

- **Aninhamento (`NO_BLOCO`):** Sempre que a análise adentra um bloco de código recuado (como o corpo de um `if`, `while`, `for`, ou definição de função), ela invoca a função `tabela_entrar_escopo()`.
- **Visibilidade Ascendente:** O analisador permite o uso de variáveis declaradas em um bloco pai. A busca da tabela sobe na hierarquia, tornando as globais ou variáveis externas acessíveis.
- **Encapsulamento Descendente:** Variáveis declaradas dentro do escopo interno morrem com ele e não "vazam" para o escopo externo. Ao finalizar a visita aos filhos de um `NO_BLOCO`, o analisador invoca `tabela_sair_escopo()` para destruir referências às variáveis daquele bloco.

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
