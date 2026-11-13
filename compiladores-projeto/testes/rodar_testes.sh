#!/bin/bash

DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$DIR/.."

if [ ! -x ./compilador ] && [ ! -x ./compilador.exe ]; then
    echo "Executavel './compilador' nao encontrado. Rode 'make' antes dos testes."
    exit 1
fi

PASSOU=0
FALHOU=0

rodar_teste() {
    arquivo=$1
    espera_erro=$2
    nome=$(basename "$arquivo")

    saida=$(./compilador < "$arquivo" 2>&1)
    tem_erro=$?

    if echo "$saida" | grep -q "Erro\|error\|inválido"; then
        tem_erro=1
    fi

    if [ "$espera_erro" = "sim" ]; then
        if [ "$tem_erro" -ne 0 ]; then
            echo "[OK]    $nome - erro detectado como esperado"
            PASSOU=$((PASSOU + 1))
        else
            echo "[FALHA] $nome - deveria dar erro mas passou"
            FALHOU=$((FALHOU + 1))
        fi
    else
        if [ "$tem_erro" -eq 0 ] && [ -z "$saida" ]; then
            echo "[OK]    $nome - aceito com sucesso"
            PASSOU=$((PASSOU + 1))
        else
            echo "[FALHA] $nome - deveria passar mas deu erro:"
            echo "        $saida"
            FALHOU=$((FALHOU + 1))
        fi
    fi
}

# Verifica a recuperacao de erros: o parser deve reportar TODOS os erros
# sintaticos do arquivo, e nao apenas o primeiro.
rodar_teste_recuperacao() {
    arquivo=$1
    esperados=$2
    nome=$(basename "$arquivo")

    obtidos=$(./compilador < "$arquivo" 2>&1 | grep -c "Erro sintático")

    if [ "$obtidos" -eq "$esperados" ]; then
        echo "[OK]    $nome - $obtidos erros sintaticos reportados"
        PASSOU=$((PASSOU + 1))
    else
        echo "[FALHA] $nome - esperava $esperados erros sintaticos, obteve $obtidos"
        FALHOU=$((FALHOU + 1))
    fi
}

# Verifica a analise semantica: o compilador deve reportar exatamente a
# quantidade esperada de erros semanticos, sem erros lexicos ou sintaticos.
rodar_teste_semantico() {
    arquivo=$1
    esperados=$2
    nome=$(basename "$arquivo")

    saida=$(./compilador < "$arquivo" 2>&1)
    obtidos=$(echo "$saida" | grep -c "Erro semântico")
    outros=$(echo "$saida" | grep -c "Erro sintático\|Erro léxico\|inválido")

    if [ "$obtidos" -eq "$esperados" ] && [ "$outros" -eq 0 ]; then
        echo "[OK]    $nome - $obtidos erros semanticos reportados"
        PASSOU=$((PASSOU + 1))
    else
        echo "[FALHA] $nome - esperava $esperados erros semanticos, obteve $obtidos (outros erros: $outros)"
        FALHOU=$((FALHOU + 1))
    fi
}

# Verifica se a mensagem de erro contem exatamente o trecho esperado (ex: linha correta).
rodar_teste_linha_erro() {
    arquivo=$1
    padrao=$2
    descricao=$3
    nome=$(basename "$arquivo")

    saida=$(./compilador < "$arquivo" 2>&1)

    if echo "$saida" | grep -Fq "$padrao"; then
        echo "[OK]    $nome ($descricao) - '$padrao'"
        PASSOU=$((PASSOU + 1))
    else
        echo "[FALHA] $nome ($descricao) - esperava encontrar '$padrao' em:"
        echo "        $saida"
        FALHOU=$((FALHOU + 1))
    fi
}

# Verifica a geracao de C# (--cs): a saida deve conter o trecho esperado.
rodar_teste_csharp() {
    arquivo=$1
    padrao=$2
    descricao=$3
    nome=$(basename "$arquivo")

    saida=$(./compilador --cs < "$arquivo" 2>&1)

    if echo "$saida" | grep -Fq -- "$padrao"; then
        echo "[OK]    $nome ($descricao) - '$padrao'"
        PASSOU=$((PASSOU + 1))
    else
        echo "[FALHA] $nome ($descricao) - esperava encontrar '$padrao' em:"
        echo "$saida" | sed 's/^/        /'
        FALHOU=$((FALHOU + 1))
    fi
}

echo "========================================="
echo "   Rodando testes do compilador"
echo "========================================="

echo ""
echo "--- Testes LEXICOS que devem PASSAR (codigo valido) ---"
rodar_teste "testes/Testes1.py" "nao"
rodar_teste "testes/Testes2.py" "nao"
rodar_teste "testes/Testes3.py" "nao"
rodar_teste "testes/Testes4.py" "nao"

echo ""
echo "--- Testes LEXICOS que devem FALHAR (codigo invalido) ---"
rodar_teste "testes/Testes5.py" "sim"

echo ""
echo "--- Testes SINTATICOS que devem PASSAR (codigo valido) ---"
rodar_teste "testes/TestesParser1.py" "nao"
rodar_teste "testes/TestesParser2.py" "nao"
rodar_teste "testes/TestesParser3.py" "nao"
rodar_teste "testes/TestesParser4.py" "nao"
rodar_teste "testes/TestesParser5.py" "nao"

echo ""
echo "--- Testes SINTATICOS que devem FALHAR (codigo invalido) ---"
rodar_teste "testes/TestesParserErro1.py" "sim"
rodar_teste "testes/TestesParserErro2.py" "sim"
rodar_teste "testes/TestesParserErro3.py" "sim"
rodar_teste "testes/TestesParserErro4.py" "sim"
rodar_teste "testes/TestesParserErro5.py" "sim"

echo ""
echo "--- Testes SEMANTICOS que devem PASSAR (codigo valido) ---"
rodar_teste "testes/TestesSemanticaValidos1.py" "nao"
rodar_teste "testes/TestesSemanticaValidos2.py" "nao"
rodar_teste "testes/TestesSemanticaValidos3.py" "nao"
rodar_teste "testes/TestesSemanticaValidos4.py" "nao"
rodar_teste "testes/TestesSemanticaValidos5.py" "nao"
rodar_teste "testes/TestesSemanticaValidos6.py" "nao"
rodar_teste "testes/TestesSemanticaValidos7.py" "nao"

echo ""
echo "--- Testes SEMANTICOS que devem FALHAR (codigo invalido) ---"
rodar_teste "testes/TestesSemanticaErros1.py" "sim"
rodar_teste "testes/TestesSemanticaErros2.py" "sim"
rodar_teste "testes/TestesSemanticaErros3.py" "sim"
rodar_teste "testes/TestesSemanticaErros7.py" "sim"
rodar_teste "testes/TestesSemanticaErros8.py" "sim"

echo ""
echo "--- RECUPERACAO de erros (todos os erros devem ser reportados) ---"
rodar_teste_recuperacao "testes/TestesParserErro1.py" 5
rodar_teste_recuperacao "testes/Testes5.py" 9

echo ""
echo "--- Testes SEMANTICOS que devem PASSAR (codigo valido) ---"
rodar_teste "testes/TestesSemantico1.py" "nao"
rodar_teste "testes/TestesSemantico2.py" "nao"

echo ""
echo "--- Testes SEMANTICOS que devem FALHAR (codigo invalido) ---"
rodar_teste "testes/teste_semantico.py" "sim"
rodar_teste "testes/TestesSemanticoErro1.py" "sim"

echo ""
echo "--- Contagem de erros SEMANTICOS (todos os erros devem ser reportados) ---"
rodar_teste_semantico "testes/teste_semantico.py" 4
rodar_teste_semantico "testes/TestesSemanticoErro1.py" 5
rodar_teste_semantico "testes/TestesSemanticoErro2.py" 2
rodar_teste_semantico "testes/TestesSemanticaErros4.py" 1
rodar_teste_semantico "testes/TestesSemanticaErros5.py" 1
rodar_teste_semantico "testes/TestesSemanticaErros6.py" 3
rodar_teste_semantico "testes/TestesSemanticaErros7.py" 1
rodar_teste_semantico "testes/TestesSemanticaErros8.py" 1
rodar_teste_semantico "testes/TestesSemanticaErros10.py" 5
rodar_teste_semantico "testes/TestesSemanticaErros11.py" 1

echo ""
echo "--- Numero de linha nos erros de estruturas compostas (Issue #46) ---"
rodar_teste_linha_erro "testes/TestesSemanticaErros5.py" "Erro semântico na linha 3: A condição deve ser estritamente booleana." "linha do if"
rodar_teste_linha_erro "testes/TestesSemanticaErros8.py" "Erro semântico na linha 3: Função 'f' já declarada na linha 1." "linhas do def"
rodar_teste_linha_erro "testes/teste_semantico.py" "Erro semântico na linha 19: A condição deve ser estritamente booleana." "linha do while"
rodar_teste_linha_erro "testes/TestesSemanticaErros10.py" "Erro semântico na linha 2: A condição deve ser estritamente booleana." "if na linha 2"
rodar_teste_linha_erro "testes/TestesSemanticaErros10.py" "Erro semântico na linha 5: A condição deve ser estritamente booleana." "elif na linha 5"
rodar_teste_linha_erro "testes/TestesSemanticaErros10.py" "Erro semântico na linha 9: A condição deve ser estritamente booleana." "while na linha 9"
rodar_teste_linha_erro "testes/TestesSemanticaErros10.py" "Erro semântico na linha 17: Identificador 'f' já declarado como função na linha 13." "for na linha 17 e def na linha 13"
rodar_teste_linha_erro "testes/TestesSemanticaErros10.py" "Erro semântico na linha 21: Função 'f' já declarada na linha 13." "def redeclarado na linha 21"

echo ""
echo "--- Geracao de codigo C# (--cs) ---"
CS="testes/TestesCodegenCSharp1.py"
rodar_teste "$CS" "nao"
rodar_teste_csharp "$CS" "using System;" "cabecalho"
rodar_teste_csharp "$CS" "namespace CompiladorPython {" "namespace"
rodar_teste_csharp "$CS" "    static class Program {" "classe estatica"
rodar_teste_csharp "$CS" "static void Main(string[] args) {" "metodo Main"
rodar_teste_csharp "$CS" "int x = 10;" "int inferido"
rodar_teste_csharp "$CS" "double y = 3.14;" "float -> double"
rodar_teste_csharp "$CS" 'string nome = "oi";' "string"
rodar_teste_csharp "$CS" 'string apelido = "bia";' "aspas simples -> duplas"
rodar_teste_csharp "$CS" "bool ativo = true;" "True -> true"
rodar_teste_csharp "$CS" "bool inativo = false;" "False -> false"
rodar_teste_csharp "$CS" "int produto = (x + 2) * 3;" "parenteses preservados"
rodar_teste_csharp "$CS" "double media = (double)x / 4;" "divisao int/int"
rodar_teste_csharp "$CS" "bool e_logico = ativo && inativo;" "and -> &&"
rodar_teste_csharp "$CS" "bool ou_logico = ativo || inativo;" "or -> ||"
rodar_teste_csharp "$CS" "bool negacao = !ativo;" "not -> !"
rodar_teste_csharp "$CS" "bool comparacao = !(x == 10);" "precedencia do not"
rodar_teste_csharp "$CS" "int negativo = -(-x);" "negacao dupla"
rodar_teste_csharp "$CS" "            x = 20;" "reatribuicao sem redeclarar"

echo ""
echo "========================================="
echo "   Resultado: $PASSOU passaram, $FALHOU falharam"
echo "========================================="

if [ "$FALHOU" -ne 0 ]; then
    exit 1
fi
