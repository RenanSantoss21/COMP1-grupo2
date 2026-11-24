#!/bin/bash

# Wrapper de compatibilidade: delega para a suite E2E principal (Issue #18).
DIR="$(cd "$(dirname "$0")" && pwd)"
exec "$DIR/rodar_testes_e2e.sh" "$@"
