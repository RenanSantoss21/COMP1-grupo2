# Tabela de Símbolos

A Tabela de Símbolos é a estrutura responsável por armazenar informações semânticas sobre os identificadores (variáveis e funções) encontrados no código-fonte durante a compilação. Ela atua como a memória do compilador, permitindo a checagem de tipos, o controle de escopos e a validação de nomes declarados.

## Estrutura de Dados

### 1. Tipos de Dados (`tipos.h`)

A linguagem suporta os seguintes tipos, mapeados pelo enum `TipoDado`:

| Tipo | Constante do Enum | Descrição |
|---|---|---|
| Inteiro | `TIPO_INT` | Valores numéricos inteiros |
| Ponto Flutuante | `TIPO_FLOAT` | Valores numéricos decimais |
| String | `TIPO_STRING` | Textos e literais de string |
| Booleano | `TIPO_BOOL` | Valores `True` ou `False` |
| Void | `TIPO_VOID` | Retorno vazio (para procedimentos) |
| Desconhecido | `TIPO_DESCONHECIDO` | Usado para tratamentos de erro |

### 2. Símbolo (`tabela.h`)

Cada identificador encontrado no código é armazenado em uma struct `Simbolo`, que contém:

- `nome` (`char*`): O nome do identificador.
- `tipo` (`TipoDado`): O tipo associado à variável ou ao retorno da função.
- `escopo` (`int`): O nível de aninhamento onde o símbolo foi declarado.
- `linha_declaracao` (`int`): A linha do código-fonte onde ocorreu a declaração.
- `e_funcao` (`bool`): `true` se o identificador for uma função, `false` se for variável.
- `num_parametros` (`int`): O número de argumentos exigidos (se for uma função).
- `tipos_parametros` (`TipoDado*`): Um array dinâmico guardando os tipos de cada parâmetro (usado na checagem semântica).

### 3. Tabela (`TabelaSimbolos`)

A estrutura da tabela em si mantém os símbolos e o estado atual do escopo:

- `array_simbolos`: Um array dinâmico que armazena os símbolos inseridos.
- `tamanho` / `capacidade`: Variáveis de controle para o redimensionamento automático (`realloc`) do array.
- `escopo_atual`: Um inteiro que indica a profundidade do bloco de código atual. O escopo global é `0`.
- `dentro_de_funcao` (`bool`): Flag usada para indicar se o analisador se encontra dentro do escopo de uma definição de função.
- `tipo_retorno_atual` (`TipoDado`): Rastreia o tipo que a função corrente está devolvendo, para garantir consistência.

## Operações (API)

A tabela de símbolos expõe as seguintes funções principais, implementadas em `tabela.c`:

### Criação e Limpeza
- `TabelaSimbolos* tabela_criar()`: Inicializa uma nova tabela vazia, no escopo `0`.
- `void tabela_liberar(TabelaSimbolos* tab)`: Libera toda a memória alocada, prevenindo vazamentos de memória (memory leaks).

### Gerenciamento de Escopos (Shadowing)
- `void tabela_entrar_escopo(TabelaSimbolos* tab)`: Incrementa a profundidade do `escopo_atual`. Utilizado ao encontrar o início de um novo bloco (como o conteúdo de um `if` ou de uma função).
- `void tabela_sair_escopo(TabelaSimbolos* tab)`: Decrementa o `escopo_atual` e **remove** todos os símbolos que pertenciam àquele escopo, liberando seus nomes e reduzindo o tamanho lógico do array.

### Manipulação de Símbolos
- `void tabela_inserir(TabelaSimbolos* tab, char* nome, TipoDado tipo, int linha)`: Adiciona um novo identificador ao array, associando-o ao `escopo_atual` (garantindo que haja capacidade suficiente no array dinâmico).
- `Simbolo* tabela_buscar(TabelaSimbolos* tab, char* nome)`: Busca um símbolo pelo nome. A busca é feita de **trás para frente** no array, o que garante que variáveis declaradas em escopos mais internos tenham prioridade sobre variáveis com o mesmo nome declaradas em escopos mais externos (*shadowing*).

### Depuração
- `void tabela_imprimir(TabelaSimbolos* tab)`: Exibe o conteúdo atual da tabela de símbolos (nomes, tipos, e profundidade) no terminal. Utilizada principalmente durante os testes do compilador.
