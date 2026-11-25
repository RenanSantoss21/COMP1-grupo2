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

# Compila e executa o C# gerado e compara a saida com a do Python.
# Pulado quando o .NET SDK nao esta instalado.
rodar_teste_csharp_execucao() {
    arquivo=$1
    nome=$(basename "$arquivo")

    if ! command -v dotnet >/dev/null 2>&1 || [ -z "$(dotnet --list-sdks 2>/dev/null)" ] ||
       ! command -v python3 >/dev/null 2>&1; then
        echo "[PULADO] $nome (execucao) - .NET SDK ou python3 nao encontrado"
        return
    fi

    projeto=$(mktemp -d)
    versao_net=$(dotnet --version | cut -d. -f1)
    cat > "$projeto/Teste.csproj" <<CSPROJ
<Project Sdk="Microsoft.NET.Sdk">
  <PropertyGroup>
    <OutputType>Exe</OutputType>
    <TargetFramework>net$versao_net.0</TargetFramework>
    <Nullable>disable</Nullable>
  </PropertyGroup>
</Project>
CSPROJ
    ./compilador --cs < "$arquivo" > "$projeto/Program.cs" 2>/dev/null

    esperado=$(python3 "$arquivo" 2>&1)
    obtido=$(LANG=pt_BR.UTF-8 DOTNET_CLI_TELEMETRY_OPTOUT=1 DOTNET_NOLOGO=1 \
             dotnet run --project "$projeto" 2>&1)

    if [ "$obtido" = "$esperado" ]; then
        echo "[OK]    $nome (execucao) - saida do C# identica a do Python"
        PASSOU=$((PASSOU + 1))
    else
        echo "[FALHA] $nome (execucao) - saida do C# difere da do Python"
        diff <(echo "$esperado") <(echo "$obtido") | sed 's/^/        /'
        FALHOU=$((FALHOU + 1))
    fi
    rm -rf "$projeto"
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
rodar_teste_semantico "testes/TestesSemanticaErros12.py" 1

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
rodar_teste_csharp "$CS" "bool ordem_texto = string.CompareOrdinal(nome, apelido) < 0;" "comparacao de strings"
rodar_teste_csharp "$CS" "bool ordem_bool = (ativo ? 1 : 0) > (inativo ? 1 : 0);" "comparacao de booleanos"
rodar_teste_csharp "$CS" "            x = 20;" "reatribuicao sem redeclarar"

CS2="testes/TestesCodegenCSharp2.py"
rodar_teste "$CS2" "nao"
rodar_teste_csharp "$CS2" "System.Globalization.CultureInfo.CurrentCulture = System.Globalization.CultureInfo.InvariantCulture;" "cultura invariante"
rodar_teste_csharp "$CS2" 'Console.WriteLine("hello, world");' "print com 1 argumento"
rodar_teste_csharp "$CS2" 'Console.WriteLine(string.Join(" ", "x =", x));' "print com varios argumentos"
rodar_teste_csharp "$CS2" "Console.WriteLine();" "print sem argumentos"
rodar_teste_csharp "$CS2" "Console.WriteLine((double)x / 4);" "divisao no print"
rodar_teste_csharp_execucao "$CS2"

CS3="testes/TestesCodegenCSharp3.py"
rodar_teste "$CS3" "nao"
rodar_teste_csharp "$CS3" "string situacao = default(string);" "hoisting de variavel criada em if/elif/else"
rodar_teste_csharp "$CS3" "int @string = default(int);" "hoisting com escape @ em palavra reservada"
rodar_teste_csharp "$CS3" "int @switch = default(int);" "hoisting de indice for com escape @"
rodar_teste_csharp "$CS3" "} else if (nota >= 5 && nota < 9) {" "elif com condicao composta"
rodar_teste_csharp "$CS3" "while (n > 0) {" "laco while"
rodar_teste_csharp "$CS3" "for (i = 0; i < 4; i++) {" "for range com 1 argumento"
rodar_teste_csharp "$CS3" "for (i = 2; i < 6; i++) {" "for range com 2 argumentos e reuso de indice"
rodar_teste_csharp "$CS3" "for (j = 0; j < 10; j += 2) {" "for range com passo constante positivo"
rodar_teste_csharp "$CS3" "for (j = 6; j > 0; j -= 2) {" "for range com passo constante negativo"
rodar_teste_csharp "$CS3" "for (j = 3; j > 0; j--) {" "for range com passo -1"
rodar_teste_csharp "$CS3" "int _passo1 = _passo0;" "passo dinamico evita colisao com _passo0"
rodar_teste_csharp "$CS3" "int _passo4 = p_din;" "passo dinamico em bloco evita colisao com _passo3 posterior"
rodar_teste_csharp "$CS3" "            int _passo3 = 10;" "variavel _passo3 posterior na raiz preserva declaracao int"
rodar_teste_csharp "$CS3" "throw new ArgumentException(\"range() arg 3 must not be zero\");" "guarda contra passo zero dinamico"
rodar_teste_csharp "$CS3" "for (k = 0; _passo1 > 0 ? k < 10 : k > 10; k += _passo1) {" "condicao ternaria para passo dinamico"
rodar_teste_csharp "$CS3" "                for (b = 0; b < 2; b++) {" "estruturas aninhadas com indentacao"
rodar_teste_csharp_execucao "$CS3"

CS4="testes/TestesCodegenCSharp4.py"
rodar_teste "$CS4" "nao"
rodar_teste_csharp "$CS4" "string.CompareOrdinal(" "comparacao ordinal de string em if/elif"
rodar_teste_csharp "$CS4" "(b_cmp1 ? 1 : 0)" "comparacao ordinal de bool em while/if"
rodar_teste_csharp "$CS4" "double val_prom = default(double);" "hoisting com promocao ADR-001 float <- int"
rodar_teste_csharp "$CS4" "a + 1" "limite inicial composto em range()"
rodar_teste_csharp "$CS4" "b * 2" "limite final composto em range()"
rodar_teste_csharp "$CS4" "int @double = default(int);" "escape @ para palavra reservada double em bloco"
rodar_teste_csharp "$CS4" "int @struct = default(int);" "escape @ para palavra reservada struct em bloco"
rodar_teste_csharp "$CS4" "int @namespace = default(int);" "escape @ para palavra reservada namespace em for"
rodar_teste_csharp "$CS4" "                    if (lvl1 == 0 && lvl2 == 0) {" "aninhamento em 3 niveis (for -> while -> if)"

cs4_stdout=$(mktemp)
cs4_arquivo=$(mktemp)
./compilador --cs < "$CS4" > "$cs4_stdout" 2>/dev/null
rc_stdout=$?
./compilador --cs "$cs4_arquivo" < "$CS4" >/dev/null 2>&1
rc_arquivo=$?
if [ "$rc_stdout" -eq 0 ] && [ "$rc_arquivo" -eq 0 ] && diff -q "$cs4_stdout" "$cs4_arquivo" >/dev/null 2>&1; then
    echo "[OK]    TestesCodegenCSharp4.py (--cs arquivo.cs) - identico ao stdout"
    PASSOU=$((PASSOU + 1))
else
    echo "[FALHA] TestesCodegenCSharp4.py (--cs arquivo.cs) - difere do stdout ou falhou"
    FALHOU=$((FALHOU + 1))
fi
rm -f "$cs4_stdout" "$cs4_arquivo"

saida_sem_cs=$(./compilador --cs < testes/TestesSemanticaErros5.py 2>&1)
rc_sem_cs=$?
if [ "$rc_sem_cs" -ne 0 ] && ! echo "$saida_sem_cs" | grep -Fq "namespace CompiladorPython"; then
    echo "[OK]    TestesSemanticaErros5.py (--cs bloqueado em erro semantico)"
    PASSOU=$((PASSOU + 1))
else
    echo "[FALHA] TestesSemanticaErros5.py (--cs nao bloqueou geracao C#)"
    FALHOU=$((FALHOU + 1))
fi

rodar_teste_csharp_execucao "$CS4"

CS_ZERO="testes/TestesCodegenCSharpErroPassoZero.py"
rodar_teste "$CS_ZERO" "nao"
rodar_teste_csharp "$CS_ZERO" 'throw new ArgumentException("range() arg 3 must not be zero");' "guarda passo zero"

CS5="testes/TestesCodegenCSharp5.py"
rodar_teste "$CS5" "nao"
rodar_teste_csharp "$CS5" "static void saudacao(string nome) {" "def void com parametro string inferido"
rodar_teste_csharp "$CS5" "static int soma(int x, int y) {" "def com retorno int e parametros inferidos"
rodar_teste_csharp "$CS5" "return x + y;" "return com expressao"
rodar_teste_csharp "$CS5" "saudacao(\"Visitante\");" "chamada de funcao como comando"
rodar_teste_csharp "$CS5" "int total = soma(3, 4);" "chamada de funcao em atribuicao"
rodar_teste_csharp_execucao "$CS5"

CS6="testes/TestesCodegenCSharp6.py"
rodar_teste "$CS6" "nao"
rodar_teste_csharp "$CS6" "string x = Console.ReadLine();" "x = input()"
rodar_teste_csharp "$CS6" 'Console.Write("Nome: ");' "prompt do input()"
rodar_teste_csharp "$CS6" "string nome = Console.ReadLine();" "nome = input(prompt)"
rodar_teste_csharp "$CS6" "Console.ReadLine();" "input() solto"

echo ""
echo "--- Geracao de codigo C# otimizado (--opt --cs) ---"
CS_OPT="testes/TestesOtimizacaoCSharp.py"
rodar_teste "$CS_OPT" "nao"

opt_cs_stdout=$(mktemp)
opt_cs_file=$(mktemp)
tr -d '\r' < "$CS_OPT" | ./compilador --opt --cs > "$opt_cs_stdout" 2>/dev/null
rc_opt_cs_out=$?
tr -d '\r' < "$CS_OPT" | ./compilador --opt --cs "$opt_cs_file" >/dev/null 2>&1
rc_opt_cs_file=$?

if [ "$rc_opt_cs_out" -eq 0 ] && [ "$rc_opt_cs_file" -eq 0 ] &&
   diff -q "$opt_cs_stdout" "$opt_cs_file" >/dev/null 2>&1 &&
   grep -Fq "int x = 14;" "$opt_cs_stdout" &&
   grep -Fq "double div_real = 2.5;" "$opt_cs_stdout" &&
   grep -Fq 'string msg = "compilador";' "$opt_cs_stdout" &&
   grep -Fq "double g = 2.0;" "$opt_cs_stdout" &&
   grep -Fq "double r_div_i = (double)n / 1;" "$opt_cs_stdout" &&
   grep -Fq "double r_div_f1 = r_div_f;" "$opt_cs_stdout" &&
   ! grep -Fq "morto_apos_return" "$opt_cs_stdout" &&
   ! grep -Fq "morto_while" "$opt_cs_stdout" &&
   ! grep -Fq "morto_if" "$opt_cs_stdout" &&
   ! grep -Fq "TAC" "$opt_cs_stdout"; then
    echo "[OK]    TestesOtimizacaoCSharp.py (--opt --cs) - C# otimizado sem TAC misturado"
    PASSOU=$((PASSOU + 1))
else
    echo "[FALHA] TestesOtimizacaoCSharp.py (--opt --cs) - C# nao otimizado ou TAC misturado"
    FALHOU=$((FALHOU + 1))
fi
rm -f "$opt_cs_stdout" "$opt_cs_file"

cs_sem_opt=$(tr -d '\r' < "$CS_OPT" | ./compilador --cs 2>/dev/null)
if echo "$cs_sem_opt" | grep -Fq "int x = 2 + 3 * 4;"; then
    echo "[OK]    TestesOtimizacaoCSharp.py (--cs isolado) - preserva AST nao otimizada"
    PASSOU=$((PASSOU + 1))
else
    echo "[FALHA] TestesOtimizacaoCSharp.py (--cs isolado) - alterou saida sem --opt"
    FALHOU=$((FALHOU + 1))
fi

tac_opt_only=$(tr -d '\r' < "$CS_OPT" | ./compilador --opt 2>/dev/null)
if ! echo "$tac_opt_only" | grep -Fq "namespace CompiladorPython"; then
    echo "[OK]    TestesOtimizacaoCSharp.py (--opt isolado) - continua emitindo apenas TAC"
    PASSOU=$((PASSOU + 1))
else
    echo "[FALHA] TestesOtimizacaoCSharp.py (--opt isolado) - emitiu C# indevidamente"
    FALHOU=$((FALHOU + 1))
fi

echo ""
echo "========================================="
echo "   Resultado: $PASSOU passaram, $FALHOU falharam"
echo "========================================="

if [ "$FALHOU" -ne 0 ]; then
    exit 1
fi
