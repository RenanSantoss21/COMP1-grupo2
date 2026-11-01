#include "otimizador.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ── Reconhecimento de tipos e literais ──────────────────────────── */

static bool e_temporario(const char *s) {
    if (!s || s[0] != 't' || s[1] == '\0') return false;
    for (int i = 1; s[i] != '\0'; i++) {
        if (!isdigit((unsigned char)s[i])) return false;
    }
    return true;
}

static bool e_inteiro(const char *s, long *out) {
    if (!s || *s == '\0') return false;
    char *endptr;
    long v = strtol(s, &endptr, 10);
    if (*endptr == '\0') {
        if (out) *out = v;
        return true;
    }
    return false;
}

static bool e_booleano(const char *s, bool *out) {
    if (!s) return false;
    if (strcmp(s, "True") == 0) {
        if (out) *out = true;
        return true;
    }
    if (strcmp(s, "False") == 0) {
        if (out) *out = false;
        return true;
    }
    return false;
}

static bool e_flutuante(const char *s, double *out) {
    if (!s || *s == '\0') return false;
    char *endptr;
    double v = strtod(s, &endptr);
    if (*endptr == '\0') {
        if (out) *out = v;
        return true;
    }
    return false;
}

bool e_constante(const char *val) {
    if (!val || *val == '\0') return false;
    long dummy_int;
    if (e_inteiro(val, &dummy_int)) return true;
    bool dummy_bool;
    if (e_booleano(val, &dummy_bool)) return true;
    size_t len = strlen(val);
    if (len >= 2 && val[0] == '"' && val[len - 1] == '"') return true;
    char *endptr;
    strtod(val, &endptr);
    if (*endptr == '\0') return true;
    return false;
}

/* ── Tabela Local de Constantes ──────────────────────────────────── */

typedef struct {
    char *var;
    char *val;
} EntradaConstante;

typedef struct {
    EntradaConstante *entradas;
    int tamanho;
    int capacidade;
} TabelaConstantes;

static void const_tab_init(TabelaConstantes *tab) {
    tab->capacidade = 16;
    tab->tamanho = 0;
    tab->entradas = malloc(sizeof(EntradaConstante) * tab->capacidade);
}

static void const_tab_limpar(TabelaConstantes *tab) {
    for (int i = 0; i < tab->tamanho; i++) {
        free(tab->entradas[i].var);
        free(tab->entradas[i].val);
    }
    tab->tamanho = 0;
}

static void const_tab_destruir(TabelaConstantes *tab) {
    const_tab_limpar(tab);
    free(tab->entradas);
    tab->entradas = NULL;
    tab->capacidade = 0;
}

static const char* const_tab_obter(TabelaConstantes *tab, const char *var) {
    if (!var) return NULL;
    for (int i = 0; i < tab->tamanho; i++) {
        if (strcmp(tab->entradas[i].var, var) == 0) {
            return tab->entradas[i].val;
        }
    }
    return NULL;
}

static void const_tab_invalidar(TabelaConstantes *tab, const char *var) {
    if (!var) return;
    for (int i = 0; i < tab->tamanho; i++) {
        if (strcmp(tab->entradas[i].var, var) == 0) {
            free(tab->entradas[i].var);
            free(tab->entradas[i].val);
            tab->entradas[i] = tab->entradas[tab->tamanho - 1];
            tab->tamanho--;
            return;
        }
    }
}

static void const_tab_definir(TabelaConstantes *tab, const char *var, const char *val) {
    if (!var || !val) return;
    for (int i = 0; i < tab->tamanho; i++) {
        if (strcmp(tab->entradas[i].var, var) == 0) {
            free(tab->entradas[i].val);
            tab->entradas[i].val = strdup(val);
            return;
        }
    }
    if (tab->tamanho >= tab->capacidade) {
        tab->capacidade *= 2;
        tab->entradas = realloc(tab->entradas, sizeof(EntradaConstante) * tab->capacidade);
    }
    tab->entradas[tab->tamanho].var = strdup(var);
    tab->entradas[tab->tamanho].val = strdup(val);
    tab->tamanho++;
}

/* ── Remoção física de uma instrução do TAC ──────────────────────── */

static void remover_instrucao(TAC *tac, int idx) {
    if (!tac || idx < 0 || idx >= tac->tamanho) return;
    free(tac->Instrucoes[idx].resultado);
    free(tac->Instrucoes[idx].operando1);
    free(tac->Instrucoes[idx].operando2);
    free(tac->Instrucoes[idx].operador);
    for (int i = idx; i < tac->tamanho - 1; i++) {
        tac->Instrucoes[i] = tac->Instrucoes[i + 1];
    }
    tac->tamanho--;
}

/* ── Passada 1: Propagação de Constantes ─────────────────────────── */

bool otimizar_propagacao_constantes(TAC *tac) {
    if (!tac || tac->tamanho == 0) return false;
    bool modificou = false;

    TabelaConstantes tab;
    const_tab_init(&tab);

    for (int i = 0; i < tac->tamanho; i++) {
        Instrucao *inst = &tac->Instrucoes[i];

        // Fronteira de fluxo de controle: limpa as constantes conhecidas
        if (inst->tipo == TAC_LABEL || inst->tipo == TAC_GOTO ||
            inst->tipo == TAC_FUNC_BEGIN || inst->tipo == TAC_FUNC_END) {
            const_tab_limpar(&tab);
            continue;
        }

        // Substituição no operando 1
        if (inst->operando1 != NULL && !e_constante(inst->operando1)) {
            const char *val = const_tab_obter(&tab, inst->operando1);
            if (val != NULL) {
                free(inst->operando1);
                inst->operando1 = strdup(val);
                modificou = true;
            }
        }

        // Substituição no operando 2
        if (inst->operando2 != NULL && !e_constante(inst->operando2)) {
            const char *val = const_tab_obter(&tab, inst->operando2);
            if (val != NULL) {
                free(inst->operando2);
                inst->operando2 = strdup(val);
                modificou = true;
            }
        }

        // Atualização da tabela de constantes
        if (inst->tipo == TAC_ASSIGN && inst->resultado != NULL) {
            if (e_constante(inst->operando1)) {
                const_tab_definir(&tab, inst->resultado, inst->operando1);
            } else {
                const_tab_invalidar(&tab, inst->resultado);
            }
        } else if (inst->tipo != TAC_IF_FALSE && inst->resultado != NULL) {
            const_tab_invalidar(&tab, inst->resultado);
        }
    }

    const_tab_destruir(&tab);
    return modificou;
}

/* ── Passada 2: Constant Folding e Simplificações Algébricas ──────── */

bool otimizar_constant_folding(TAC *tac) {
    if (!tac || tac->tamanho == 0) return false;
    bool modificou = false;

    for (int i = 0; i < tac->tamanho; i++) {
        Instrucao *inst = &tac->Instrucoes[i];

        if (inst->tipo == TAC_BINOP && inst->operando1 && inst->operando2 && inst->operador) {
            long v1, v2;
            bool v1_int = e_inteiro(inst->operando1, &v1);
            bool v2_int = e_inteiro(inst->operando2, &v2);

            // Dobra de constantes inteiras
            if (v1_int && v2_int) {
                char buffer[64];
                bool calculado = false;

                if (strcmp(inst->operador, "+") == 0) {
                    snprintf(buffer, sizeof(buffer), "%ld", v1 + v2);
                    calculado = true;
                } else if (strcmp(inst->operador, "-") == 0) {
                    snprintf(buffer, sizeof(buffer), "%ld", v1 - v2);
                    calculado = true;
                } else if (strcmp(inst->operador, "*") == 0) {
                    snprintf(buffer, sizeof(buffer), "%ld", v1 * v2);
                    calculado = true;
                } else if (strcmp(inst->operador, "/") == 0 && v2 != 0) {
                    snprintf(buffer, sizeof(buffer), "%ld", v1 / v2);
                    calculado = true;
                } else if (strcmp(inst->operador, "==") == 0) {
                    snprintf(buffer, sizeof(buffer), "%s", (v1 == v2) ? "True" : "False");
                    calculado = true;
                } else if (strcmp(inst->operador, "!=") == 0) {
                    snprintf(buffer, sizeof(buffer), "%s", (v1 != v2) ? "True" : "False");
                    calculado = true;
                } else if (strcmp(inst->operador, "<") == 0) {
                    snprintf(buffer, sizeof(buffer), "%s", (v1 < v2) ? "True" : "False");
                    calculado = true;
                } else if (strcmp(inst->operador, "<=") == 0) {
                    snprintf(buffer, sizeof(buffer), "%s", (v1 <= v2) ? "True" : "False");
                    calculado = true;
                } else if (strcmp(inst->operador, ">") == 0) {
                    snprintf(buffer, sizeof(buffer), "%s", (v1 > v2) ? "True" : "False");
                    calculado = true;
                } else if (strcmp(inst->operador, ">=") == 0) {
                    snprintf(buffer, sizeof(buffer), "%s", (v1 >= v2) ? "True" : "False");
                    calculado = true;
                }

                if (calculado) {
                    inst->tipo = TAC_ASSIGN;
                    free(inst->operando1);
                    inst->operando1 = strdup(buffer);
                    free(inst->operando2);
                    inst->operando2 = NULL;
                    free(inst->operador);
                    inst->operador = NULL;
                    modificou = true;
                    continue;
                }
            }

            // Dobra de constantes em ponto flutuante (quando ao menos um operando é float)
            double f1, f2;
            if ((!v1_int || !v2_int) && e_flutuante(inst->operando1, &f1) && e_flutuante(inst->operando2, &f2)) {
                char buffer[64];
                bool calculado = false;

                if (strcmp(inst->operador, "+") == 0) {
                    snprintf(buffer, sizeof(buffer), "%f", f1 + f2);
                    calculado = true;
                } else if (strcmp(inst->operador, "-") == 0) {
                    snprintf(buffer, sizeof(buffer), "%f", f1 - f2);
                    calculado = true;
                } else if (strcmp(inst->operador, "*") == 0) {
                    snprintf(buffer, sizeof(buffer), "%f", f1 * f2);
                    calculado = true;
                } else if (strcmp(inst->operador, "/") == 0 && f2 != 0.0) {
                    snprintf(buffer, sizeof(buffer), "%f", f1 / f2);
                    calculado = true;
                } else if (strcmp(inst->operador, "==") == 0) {
                    snprintf(buffer, sizeof(buffer), "%s", (f1 == f2) ? "True" : "False");
                    calculado = true;
                } else if (strcmp(inst->operador, "!=") == 0) {
                    snprintf(buffer, sizeof(buffer), "%s", (f1 != f2) ? "True" : "False");
                    calculado = true;
                } else if (strcmp(inst->operador, "<") == 0) {
                    snprintf(buffer, sizeof(buffer), "%s", (f1 < f2) ? "True" : "False");
                    calculado = true;
                } else if (strcmp(inst->operador, "<=") == 0) {
                    snprintf(buffer, sizeof(buffer), "%s", (f1 <= f2) ? "True" : "False");
                    calculado = true;
                } else if (strcmp(inst->operador, ">") == 0) {
                    snprintf(buffer, sizeof(buffer), "%s", (f1 > f2) ? "True" : "False");
                    calculado = true;
                } else if (strcmp(inst->operador, ">=") == 0) {
                    snprintf(buffer, sizeof(buffer), "%s", (f1 >= f2) ? "True" : "False");
                    calculado = true;
                }

                if (calculado) {
                    inst->tipo = TAC_ASSIGN;
                    free(inst->operando1);
                    inst->operando1 = strdup(buffer);
                    free(inst->operando2);
                    inst->operando2 = NULL;
                    free(inst->operador);
                    inst->operador = NULL;
                    modificou = true;
                    continue;
                }
            }

            // Dobra de constantes booleanas (and / or)
            bool b1, b2;
            if (e_booleano(inst->operando1, &b1) && e_booleano(inst->operando2, &b2)) {
                bool calculado = false;
                bool res = false;
                if (strcmp(inst->operador, "and") == 0) {
                    res = b1 && b2;
                    calculado = true;
                } else if (strcmp(inst->operador, "or") == 0) {
                    res = b1 || b2;
                    calculado = true;
                }
                if (calculado) {
                    inst->tipo = TAC_ASSIGN;
                    free(inst->operando1);
                    inst->operando1 = strdup(res ? "True" : "False");
                    free(inst->operando2);
                    inst->operando2 = NULL;
                    free(inst->operador);
                    inst->operador = NULL;
                    modificou = true;
                    continue;
                }
            }

            // Simplificações algébricas com 0 e 1
            if (v2_int && strcmp(inst->operador, "*") == 0 && v2 == 1) {
                // x * 1 -> x
                inst->tipo = TAC_ASSIGN;
                free(inst->operando2);
                inst->operando2 = NULL;
                free(inst->operador);
                inst->operador = NULL;
                modificou = true;
                continue;
            }
            if (v1_int && strcmp(inst->operador, "*") == 0 && v1 == 1) {
                // 1 * x -> x
                inst->tipo = TAC_ASSIGN;
                free(inst->operando1);
                inst->operando1 = inst->operando2;
                inst->operando2 = NULL;
                free(inst->operador);
                inst->operador = NULL;
                modificou = true;
                continue;
            }
            if (v2_int && strcmp(inst->operador, "*") == 0 && v2 == 0) {
                // x * 0 -> 0
                inst->tipo = TAC_ASSIGN;
                free(inst->operando1);
                inst->operando1 = strdup("0");
                free(inst->operando2);
                inst->operando2 = NULL;
                free(inst->operador);
                inst->operador = NULL;
                modificou = true;
                continue;
            }
            if (v1_int && strcmp(inst->operador, "*") == 0 && v1 == 0) {
                // 0 * x -> 0
                inst->tipo = TAC_ASSIGN;
                free(inst->operando2);
                inst->operando2 = NULL;
                free(inst->operador);
                inst->operador = NULL;
                modificou = true;
                continue;
            }
            if (v2_int && strcmp(inst->operador, "+") == 0 && v2 == 0) {
                // x + 0 -> x
                inst->tipo = TAC_ASSIGN;
                free(inst->operando2);
                inst->operando2 = NULL;
                free(inst->operador);
                inst->operador = NULL;
                modificou = true;
                continue;
            }
            if (v1_int && strcmp(inst->operador, "+") == 0 && v1 == 0) {
                // 0 + x -> x
                inst->tipo = TAC_ASSIGN;
                free(inst->operando1);
                inst->operando1 = inst->operando2;
                inst->operando2 = NULL;
                free(inst->operador);
                inst->operador = NULL;
                modificou = true;
                continue;
            }
            if (v2_int && strcmp(inst->operador, "-") == 0 && v2 == 0) {
                // x - 0 -> x
                inst->tipo = TAC_ASSIGN;
                free(inst->operando2);
                inst->operando2 = NULL;
                free(inst->operador);
                inst->operador = NULL;
                modificou = true;
                continue;
            }
            if (v2_int && strcmp(inst->operador, "/") == 0 && v2 == 1) {
                // x / 1 -> x
                inst->tipo = TAC_ASSIGN;
                free(inst->operando2);
                inst->operando2 = NULL;
                free(inst->operador);
                inst->operador = NULL;
                modificou = true;
                continue;
            }
        }

        // Operador unário sobre constante
        if (inst->tipo == TAC_UNOP && inst->operando1 && inst->operador) {
            long v;
            if (strcmp(inst->operador, "-") == 0 && e_inteiro(inst->operando1, &v)) {
                char buffer[64];
                snprintf(buffer, sizeof(buffer), "%ld", -v);
                inst->tipo = TAC_ASSIGN;
                free(inst->operando1);
                inst->operando1 = strdup(buffer);
                free(inst->operador);
                inst->operador = NULL;
                modificou = true;
                continue;
            }
            bool b;
            if (strcmp(inst->operador, "not") == 0 && e_booleano(inst->operando1, &b)) {
                inst->tipo = TAC_ASSIGN;
                free(inst->operando1);
                inst->operando1 = strdup(b ? "False" : "True");
                free(inst->operador);
                inst->operador = NULL;
                modificou = true;
                continue;
            }
        }
    }

    return modificou;
}

/* ── Passada 3: Remoção de Código Morto ───────────────────────────── */

static int contar_usos(TAC *tac, const char *nome, int a_partir_de_idx) {
    if (!tac || !nome) return 0;
    int usos = 0;
    for (int i = a_partir_de_idx; i < tac->tamanho; i++) {
        Instrucao *inst = &tac->Instrucoes[i];
        if (inst->operando1 && strcmp(inst->operando1, nome) == 0) usos++;
        if (inst->operando2 && strcmp(inst->operando2, nome) == 0) usos++;
        // Se a variável for completamente sobrescrita sem ter sido lida antes, os usos futuros são de outro valor
        if (inst->resultado && strcmp(inst->resultado, nome) == 0 &&
            inst->tipo != TAC_PRINT && inst->tipo != TAC_PARAM) {
            // Se for temporário, temporários não são reatribuídos
            if (e_temporario(nome)) {
                // continua
            }
        }
    }
    return usos;
}

bool otimizar_remocao_codigo_morto(TAC *tac) {
    if (!tac || tac->tamanho == 0) return false;
    bool modificou = false;

    // 1. Eliminação de temporários intermediários de uso único:
    // t1 = c * d
    // x = t1  -->  x = c * d (e t1 é eliminado)
    for (int i = 0; i < tac->tamanho - 1; i++) {
        Instrucao *inst1 = &tac->Instrucoes[i];
        Instrucao *inst2 = &tac->Instrucoes[i + 1];

        if (inst1->resultado && e_temporario(inst1->resultado) &&
            inst2->tipo == TAC_ASSIGN && inst2->operando1 &&
            strcmp(inst1->resultado, inst2->operando1) == 0) {

            // Verifica se inst1->resultado é usado somente em inst2 em todo o TAC
            int total_usos = contar_usos(tac, inst1->resultado, 0);
            if (total_usos == 1) {
                // Redireciona o resultado de inst1 diretamente para inst2->resultado
                free(inst1->resultado);
                inst1->resultado = strdup(inst2->resultado);
                remover_instrucao(tac, i + 1);
                modificou = true;
                i--;
                continue;
            }
        }
    }

    // 2. Remoção de instruções cujos temporários nunca são lidos em todo o TAC
    // t0 = a + b (com t0 sem nenhum uso) -> removido
    for (int i = 0; i < tac->tamanho; i++) {
        Instrucao *inst = &tac->Instrucoes[i];

        if ((inst->tipo == TAC_ASSIGN || inst->tipo == TAC_BINOP || inst->tipo == TAC_UNOP) &&
            inst->resultado && e_temporario(inst->resultado)) {

            int usos = contar_usos(tac, inst->resultado, 0);
            if (usos == 0) {
                remover_instrucao(tac, i);
                modificou = true;
                i--;
                continue;
            }
        }
    }

    // 3. Remoção de código inalcançável imediatamente após RETURN ou GOTO
    // até o próximo LABEL, FUNC_END ou FUNC_BEGIN
    for (int i = 0; i < tac->tamanho - 1; i++) {
        Instrucao *inst = &tac->Instrucoes[i];
        if (inst->tipo == TAC_RETURN || inst->tipo == TAC_GOTO) {
            Instrucao *prox = &tac->Instrucoes[i + 1];
            if (prox->tipo != TAC_LABEL &&
                prox->tipo != TAC_FUNC_END &&
                prox->tipo != TAC_FUNC_BEGIN) {
                remover_instrucao(tac, i + 1);
                modificou = true;
                i--;
            }
        }
    }

    return modificou;
}

/* ── Loop de Ponto Fixo da Pipeline ──────────────────────────────── */

int otimizar_tac(TAC *tac) {
    if (!tac || tac->tamanho == 0) return 0;
    int iteracoes = 0;
    bool modificou = true;

    while (modificou && iteracoes < 20) {
        modificou = false;
        if (otimizar_propagacao_constantes(tac)) modificou = true;
        if (otimizar_constant_folding(tac))      modificou = true;
        if (otimizar_remocao_codigo_morto(tac))  modificou = true;
        iteracoes++;
    }

    return iteracoes;
}
