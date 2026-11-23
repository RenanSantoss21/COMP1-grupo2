#!/bin/bash

DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$DIR/.."

COMP="./compilador"
if [ ! -x "$COMP" ] && [ -x ./compilador.exe ]; then
    COMP="./compilador.exe"
fi
if [ ! -x "$COMP" ]; then
    echo "Executavel './compilador' nao encontrado. Rode 'make' antes dos testes E2E."
    exit 1
fi

if ! command -v python3 >/dev/null 2>&1; then
    echo "Erro: 'python3' nao encontrado para validacao comparativa E2E."
    exit 1
fi

CSHARP_MODO=""
CSC_PATH="/mnt/c/Windows/Microsoft.NET/Framework64/v4.0.30319/csc.exe"

if command -v dotnet >/dev/null 2>&1 && [ -n "$(dotnet --list-sdks 2>/dev/null)" ]; then
    CSHARP_MODO="dotnet"
elif command -v mcs >/dev/null 2>&1; then
    CSHARP_MODO="mcs"
elif [ -x "$CSC_PATH" ]; then
    CSHARP_MODO="csc"
else
    echo "Erro: nenhum compilador C# disponivel (dotnet SDK, mcs ou csc.exe)."
    exit 1
fi

BASE_TEMP="/tmp"
if [ "$CSHARP_MODO" = "csc" ]; then
    win_temp="$(cd /mnt/c 2>/dev/null && cmd.exe /c echo %TEMP% 2>/dev/null | tr -d '\r\n')"
    if [ -n "$win_temp" ] && command -v wslpath >/dev/null 2>&1; then
        unix_temp="$(wslpath -u "$win_temp" 2>/dev/null)"
        if [ -n "$unix_temp" ] && [ -d "$unix_temp" ]; then
            BASE_TEMP="$unix_temp"
        fi
    fi
fi

WORK="$(mktemp -d "$BASE_TEMP/comp_e2e_XXXXXX" 2>/dev/null || mktemp -d "/tmp/comp_e2e_XXXXXX")"
trap 'rm -rf "$WORK"' EXIT

PASSOU=0
FALHOU=0

registrar_ok() {
    echo "[OK]    $1"
    PASSOU=$((PASSOU + 1))
}

registrar_falha() {
    echo "[FALHA] $1"
    if [ -n "$2" ]; then
        echo "$2" | sed 's/^/        /'
    fi
    FALHOU=$((FALHOU + 1))
}

compilar_csharp() {
    cs_src="$1"
    exe_dest="$2"
    log_comp="$3"
    nome_base="$(basename "$exe_dest" .exe)"

    case "$CSHARP_MODO" in
        dotnet)
            proj_dir="$WORK/dotnet_$nome_base"
            rm -rf "$proj_dir"
            mkdir -p "$proj_dir"
            versao_net=$(dotnet --version | cut -d. -f1)
            cat > "$proj_dir/App.csproj" <<CSPROJ
<Project Sdk="Microsoft.NET.Sdk">
  <PropertyGroup>
    <OutputType>Exe</OutputType>
    <TargetFramework>net$versao_net.0</TargetFramework>
    <Nullable>disable</Nullable>
  </PropertyGroup>
</Project>
CSPROJ
            cp "$cs_src" "$proj_dir/Program.cs"
            DOTNET_CLI_TELEMETRY_OPTOUT=1 DOTNET_NOLOGO=1 \
                dotnet build "$proj_dir/App.csproj" -c Release >"$log_comp" 2>&1
            ;;
        mcs)
            mcs -out:"$exe_dest" "$cs_src" >"$log_comp" 2>&1
            ;;
        csc)
            "$CSC_PATH" /nologo /utf8output /codepage:65001 /debug+ \
                /out:"$(wslpath -w "$exe_dest")" \
                "$(wslpath -w "$cs_src")" >"$log_comp" 2>&1
            ;;
    esac
}

executar_csharp() {
    exe_dest="$1"
    out_file="$2"
    err_file="$3"
    nome_base="$(basename "$exe_dest" .exe)"

    case "$CSHARP_MODO" in
        dotnet)
            proj_dir="$WORK/dotnet_$nome_base"
            LANG=pt_BR.UTF-8 DOTNET_CLI_TELEMETRY_OPTOUT=1 DOTNET_NOLOGO=1 \
                dotnet run --project "$proj_dir/App.csproj" -c Release --no-build >"$out_file" 2>"$err_file"
            ;;
        mcs)
            if command -v mono >/dev/null 2>&1; then
                LANG=pt_BR.UTF-8 mono "$exe_dest" >"$out_file" 2>"$err_file"
            else
                LANG=pt_BR.UTF-8 "$exe_dest" >"$out_file" 2>"$err_file"
            fi
            ;;
        csc)
            "$exe_dest" >"$out_file" 2>"$err_file"
            ;;
    esac
}

gerar_e_validar_cs() {
    arquivo="$1"
    prefixo="$2"
    cs_stdout="$WORK/${prefixo}_stdout.cs"
    cs_file="$WORK/${prefixo}_file.cs"
    err_stdout="$WORK/${prefixo}_stdout.err"
    err_file="$WORK/${prefixo}_file.err"
    nome="$(basename "$arquivo")"

    tr -d '\r' < "$arquivo" | "$COMP" --cs > "$cs_stdout" 2>"$err_stdout"
    rc_out=$?
    tr -d '\r' < "$arquivo" | "$COMP" --cs "$cs_file" >/dev/null 2>"$err_file"
    rc_file=$?

    diff_cs=$(diff -u "$cs_stdout" "$cs_file" 2>&1)
    rc_diff=$?

    if [ "$rc_out" -eq 0 ] && [ "$rc_file" -eq 0 ] && [ "$rc_diff" -eq 0 ] &&
       ! grep -Fq "Aviso:" "$err_stdout" && ! grep -Fq "Aviso:" "$err_file" &&
       ! grep -Fq "não traduzido" "$cs_stdout"; then
        registrar_ok "$nome - --cs stdout == arquivo e 0 avisos nao traduzidos"
        return 0
    else
        detalhes="rc_out=$rc_out rc_file=$rc_file rc_diff=$rc_diff"
        if [ -n "$diff_cs" ]; then
            detalhes="$detalhes"$'\n'"$diff_cs"
        fi
        if [ -s "$err_stdout" ]; then
            detalhes="$detalhes"$'\n'"$(cat "$err_stdout")"
        fi
        registrar_falha "$nome - erro na emissao --cs ou avisos nao traduzidos" "$detalhes"
        return 1
    fi
}

echo "========================================="
echo "   Validacao E2E (Mini-Python -> C#)"
echo "   Compilador C#: $CSHARP_MODO"
echo "========================================="

echo ""
echo "--- 1. Pipeline completo em TestesCodegenCSharp4.py ---"
CS4="testes/TestesCodegenCSharp4.py"

ast_out="$WORK/cs4_ast.txt"
ast_err="$WORK/cs4_ast.err"
tr -d '\r' < "$CS4" | "$COMP" --ast > "$ast_out" 2>"$ast_err"
rc_ast=$?
if [ "$rc_ast" -eq 0 ] &&
   grep -Fq "Programa" "$ast_out" &&
   grep -Fq "If" "$ast_out" &&
   grep -Fq "While" "$ast_out" &&
   grep -Fq "For" "$ast_out"; then
    registrar_ok "TestesCodegenCSharp4.py (--ast) - exit 0 com Programa/If/While/For"
else
    registrar_falha "TestesCodegenCSharp4.py (--ast) - rc=$rc_ast" "$(cat "$ast_err")"
fi

tab_out="$WORK/cs4_tabela.txt"
tab_err="$WORK/cs4_tabela.err"
tr -d '\r' < "$CS4" | "$COMP" --tabela > "$tab_out" 2>"$tab_err"
rc_tab=$?
if [ "$rc_tab" -eq 0 ] &&
   grep -Fq "Nome: const_base" "$tab_out" &&
   grep -Fq "Nome: a_fib" "$tab_out" &&
   grep -Fq "Nome: val_prom" "$tab_out" &&
   grep -Fq "Nome: double" "$tab_out" &&
   grep -Fq "Nome: struct" "$tab_out" &&
   grep -Fq "Nome: namespace" "$tab_out"; then
    registrar_ok "TestesCodegenCSharp4.py (--tabela) - exit 0 com simbolos esperados"
else
    registrar_falha "TestesCodegenCSharp4.py (--tabela) - rc=$rc_tab" "$(cat "$tab_err")"
fi

tac_out="$WORK/cs4_tac.txt"
tac_err="$WORK/cs4_tac.err"
opt_out="$WORK/cs4_opt.txt"
opt_err="$WORK/cs4_opt.err"
tr -d '\r' < "$CS4" | "$COMP" --tac > "$tac_out" 2>"$tac_err"
rc_tac=$?
tr -d '\r' < "$CS4" | "$COMP" --opt > "$opt_out" 2>"$opt_err"
rc_opt=$?
linhas_tac=$(grep -c '[^[:space:]]' "$tac_out" || true)
linhas_opt=$(grep -c '[^[:space:]]' "$opt_out" || true)
if [ "$rc_tac" -eq 0 ] && [ "$rc_opt" -eq 0 ] && [ "$linhas_opt" -lt "$linhas_tac" ]; then
    registrar_ok "TestesCodegenCSharp4.py (--tac $linhas_tac vs --opt $linhas_opt linhas)"
else
    registrar_falha "TestesCodegenCSharp4.py (--tac vs --opt)" "rc_tac=$rc_tac rc_opt=$rc_opt tac=$linhas_tac opt=$linhas_opt"
fi

echo ""
echo "--- 2. Bloqueio de --cs em erro semantico ---"
SEM_ERR="testes/TestesSemanticaErros5.py"
sem_stdout="$WORK/sem_err_stdout.cs"
sem_file="$WORK/sem_err_file.cs"
sem_errlog="$WORK/sem_err.log"

tr -d '\r' < "$SEM_ERR" | "$COMP" --cs > "$sem_stdout" 2>"$sem_errlog"
rc_sem_out=$?
tr -d '\r' < "$SEM_ERR" | "$COMP" --cs "$sem_file" >/dev/null 2>>"$sem_errlog"
rc_sem_file=$?

if [ "$rc_sem_out" -ne 0 ] && [ "$rc_sem_file" -ne 0 ] &&
   grep -Fq "Erro semântico" "$sem_errlog" &&
   ! grep -Fq "namespace CompiladorPython" "$sem_stdout" &&
   [ ! -f "$sem_file" ]; then
    registrar_ok "TestesSemanticaErros5.py (--cs bloqueado com Erro semantico e sem gerar C#)"
else
    registrar_falha "TestesSemanticaErros5.py (--cs nao bloqueou corretamente)" "$(cat "$sem_errlog")"
fi

echo ""
echo "--- 3. Emissao, compilacao C# e execucao comparada ---"

for idx in 1 2 3 4 5; do
    arquivo="testes/TestesCodegenCSharp${idx}.py"
    nome="$(basename "$arquivo")"
    prefixo="cs${idx}"

    if ! gerar_e_validar_cs "$arquivo" "$prefixo"; then
        continue
    fi

    cs_src="$WORK/${prefixo}_file.cs"
    exe_dest="$WORK/${prefixo}.exe"
    log_comp="$WORK/${prefixo}_comp.log"

    if compilar_csharp "$cs_src" "$exe_dest" "$log_comp"; then
        registrar_ok "$nome - compilacao C# ($CSHARP_MODO) com 0 erros"
    else
        registrar_falha "$nome - erro ao compilar C#" "$(cat "$log_comp")"
        continue
    fi

    cs_out="$WORK/${prefixo}_cs.out"
    cs_err="$WORK/${prefixo}_cs.err"
    executar_csharp "$exe_dest" "$cs_out" "$cs_err"
    rc_cs=$?

    if [ "$idx" -eq 1 ]; then
        if [ "$rc_cs" -eq 0 ]; then
            registrar_ok "$nome - execucao C# (exit 0)"
        else
            registrar_falha "$nome - execucao C# falhou (rc=$rc_cs)" "$(cat "$cs_err")"
        fi
    else
        py_out="$WORK/${prefixo}_py.out"
        py_err="$WORK/${prefixo}_py.err"
        python3 "$arquivo" >"$py_out" 2>"$py_err"
        rc_py=$?

        tr -d '\r' < "$cs_out" > "$WORK/${prefixo}_cs_norm.out"
        tr -d '\r' < "$py_out" > "$WORK/${prefixo}_py_norm.out"
        diff_exec=$(diff -u "$WORK/${prefixo}_py_norm.out" "$WORK/${prefixo}_cs_norm.out" 2>&1)
        rc_diff=$?

        if [ "$rc_cs" -eq 0 ] && [ "$rc_py" -eq 0 ] && [ "$rc_diff" -eq 0 ]; then
            registrar_ok "$nome - execucao C# identica ao python3 (exit 0)"
        else
            registrar_falha "$nome - divergencia ou erro (rc_cs=$rc_cs rc_py=$rc_py)" "$diff_exec"$'\n'"$(cat "$cs_err")$(cat "$py_err")"
        fi
    fi
done

echo ""
echo "--- 4. Guarda de passo zero em runtime ---"
CS_ZERO="testes/TestesCodegenCSharpErroPassoZero.py"
if gerar_e_validar_cs "$CS_ZERO" "cs_zero"; then
    cs_zero_src="$WORK/cs_zero_file.cs"
    exe_zero="$WORK/cs_zero.exe"
    log_zero="$WORK/cs_zero_comp.log"

    if compilar_csharp "$cs_zero_src" "$exe_zero" "$log_zero"; then
        registrar_ok "TestesCodegenCSharpErroPassoZero.py - compilacao C# com 0 erros"

        py_zero_out="$WORK/cs_zero_py.out"
        python3 "$CS_ZERO" >"$py_zero_out" 2>&1
        rc_py_zero=$?

        cs_zero_out="$WORK/cs_zero_cs.out"
        executar_csharp "$exe_zero" "$cs_zero_out" "$WORK/cs_zero_cs.err"
        rc_cs_zero=$?
        cat "$WORK/cs_zero_cs.err" >> "$cs_zero_out"

        if [ "$rc_py_zero" -ne 0 ] && grep -Fq "ValueError" "$py_zero_out"; then
            registrar_ok "TestesCodegenCSharpErroPassoZero.py - python3 falhou com ValueError (rc=$rc_py_zero)"
        else
            registrar_falha "TestesCodegenCSharpErroPassoZero.py - python3 deveria falhar com ValueError (rc=$rc_py_zero)" "$(cat "$py_zero_out")"
        fi

        if [ "$rc_cs_zero" -ne 0 ] && grep -Fq "range() arg 3 must not be zero" "$cs_zero_out"; then
            registrar_ok "TestesCodegenCSharpErroPassoZero.py - C# falhou com ArgumentException (rc=$rc_cs_zero)"
        else
            registrar_falha "TestesCodegenCSharpErroPassoZero.py - C# deveria falhar com range() arg 3 must not be zero (rc=$rc_cs_zero)" "$(cat "$cs_zero_out")"
        fi
    else
        registrar_falha "TestesCodegenCSharpErroPassoZero.py - erro ao compilar C#" "$(cat "$log_zero")"
    fi
fi

echo ""
echo "========================================="
echo "   Resultado E2E: $PASSOU passaram, $FALHOU falharam"
echo "========================================="

if [ "$FALHOU" -ne 0 ]; then
    exit 1
fi
