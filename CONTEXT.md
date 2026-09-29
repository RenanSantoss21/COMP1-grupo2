# Compilador (COMP1-grupo2)

Glossário e visão geral dos conceitos do domínio do projeto do compilador. O sistema recebe código-fonte em Python (linguagem de origem) e gera como saída código em C# (linguagem de destino/alvo). O compilador em si é implementado na linguagem C.

## Componentes do Compilador

**Lexer**:
Componente responsável por ler o código-fonte texto e dividi-lo em uma sequência de Tokens, descartando espaços em branco não significativos (porém processando indentação).
_Avoid_: Scanner, Tokenizador.

**Parser**:
O analisador sintático gerado pelo Bison que valida a estrutura gramatical sequenciada de tokens gerada pelo Lexer e constrói a Árvore Sintática Abstrata.
_Avoid_: Analisador gramatical.

**AST (Árvore Sintática Abstrata)**:
A estrutura de dados central na forma de árvore onde cada nó (`NoAST`) representa uma construção do código fonte (como laços, operadores, declarações de função, literais).
_Avoid_: Árvore de parsing.

**Tabela de Símbolos**:
A estrutura de dados que armazena o dicionário de identificadores definidos no código (como variáveis e funções) em conjunto com seus metadados semânticos (tipos, número de argumentos, níveis de escopo, etc.).
_Avoid_: Tabela de hash, dicionário de variáveis.

**Analisador Semântico**:
A etapa (`analisar_semantica`) onde a AST é percorrida recursivamente utilizando a Tabela de Símbolos para inferir tipos, validar restrições (uso de variáveis antes de declarar, checagem de argumentos em chamadas de função) e verificar o uso correto de comandos (como `return`).
_Avoid_: Validador, verificador de tipos.

## Estruturas de Dados do Domínio

**Simbolo**:
A representação individual armazenada na Tabela de Símbolos (englobando tipo resolvido, número de argumentos/parâmetros, nome do identificador).
_Avoid_: Variável (variável é um conceito da linguagem fonte; Símbolo engloba funções e variáveis armazenadas no compilador).

**TipoDado**:
O enum interno (`TIPO_INT`, `TIPO_FLOAT`, `TIPO_BOOL`, etc.) inferido e utilizado na etapa semântica para prever o tipo resultante em expressões e retornos.
_Avoid_: Type, primitive.

**Escopo**:
O contexto em que um Simbolo é visível. Ao entrar em blocos específicos (funções) o analisador aumenta o escopo, isolando declarações, e ao fechar o bloco ele os descarta (`tabela_sair_escopo`).
_Avoid_: Contexto de execução.
