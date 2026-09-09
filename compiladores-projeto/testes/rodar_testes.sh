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
echo "========================================="
echo "   Resultado: $PASSOU passaram, $FALHOU falharam"
echo "========================================="

if [ "$FALHOU" -ne 0 ]; then
    exit 1
fi
