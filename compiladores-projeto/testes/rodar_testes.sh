#!/bin/bash

DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$DIR/.."

if [ ! -x ./compilador ]; then
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

echo ""
echo "--- Testes SEMANTICOS que devem FALHAR (codigo invalido) ---"
rodar_teste "testes/TestesSemanticaErros1.py" "sim"
rodar_teste "testes/TestesSemanticaErros2.py" "sim"
rodar_teste "testes/TestesSemanticaErros3.py" "sim"

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
rodar_teste "testes/TestesSemanticoErro2.py" "sim"

echo ""
echo "--- Contagem de erros SEMANTICOS (todos os erros devem ser reportados) ---"
rodar_teste_semantico "testes/teste_semantico.py" 4
rodar_teste_semantico "testes/TestesSemanticoErro1.py" 5

echo ""
echo "========================================="
echo "   Resultado: $PASSOU passaram, $FALHOU falharam"
echo "========================================="

if [ "$FALHOU" -ne 0 ]; then
    exit 1
fi
