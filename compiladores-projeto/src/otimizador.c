#include "otimizador.h"
#include <limits.h>
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

static bool e_string_literal(const char *s) {
    if (!s) return false;
    size_t len = strlen(s);
    if (len >= 2 && ((s[0] == '"' && s[len - 1] == '"') || (s[0] == '\'' && s[len - 1] == '\''))) {
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
    if (e_string_literal(val)) return true;
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
    bool modificou_geral = false;
    bool modificou_passo = true;
    int passadas = 0;

    while (modificou_passo && passadas < 10) {
        modificou_passo = false;
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

            // Propagação de constantes previamente conhecidas dentro do bloco básico
            if (inst->operando1 != NULL && !e_constante(inst->operando1)) {
                const char *val = const_tab_obter(&tab, inst->operando1);
                if (val != NULL) {
                    free(inst->operando1);
                    inst->operando1 = strdup(val);
                    modificou_passo = true;
                    modificou_geral = true;
                }
            }

            if (inst->operando2 != NULL && !e_constante(inst->operando2)) {
                const char *val = const_tab_obter(&tab, inst->operando2);
                if (val != NULL) {
                    free(inst->operando2);
                    inst->operando2 = strdup(val);
                    modificou_passo = true;
                    modificou_geral = true;
                }
            }

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
                        modificou_passo = true;
                        modificou_geral = true;
                        const_tab_definir(&tab, inst->resultado, inst->operando1);
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
                        modificou_passo = true;
                        modificou_geral = true;
                        const_tab_definir(&tab, inst->resultado, inst->operando1);
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
                        modificou_passo = true;
                        modificou_geral = true;
                        const_tab_definir(&tab, inst->resultado, inst->operando1);
                        continue;
                    }
                }

                // Dobra de constantes de texto (concatenação e comparação)
                if (e_string_literal(inst->operando1) && e_string_literal(inst->operando2)) {
                    if (strcmp(inst->operador, "+") == 0) {
                        size_t len1 = strlen(inst->operando1);
                        size_t len2 = strlen(inst->operando2);
                        size_t total_len = (len1 - 2) + (len2 - 2) + 3;
                        char *novo = malloc(total_len);
                        novo[0] = '"';
                        memcpy(novo + 1, inst->operando1 + 1, len1 - 2);
                        memcpy(novo + 1 + len1 - 2, inst->operando2 + 1, len2 - 2);
                        novo[total_len - 2] = '"';
                        novo[total_len - 1] = '\0';

                        inst->tipo = TAC_ASSIGN;
                        free(inst->operando1);
                        inst->operando1 = novo;
                        free(inst->operando2);
                        inst->operando2 = NULL;
                        free(inst->operador);
                        inst->operador = NULL;
                        modificou_passo = true;
                        modificou_geral = true;
                        const_tab_definir(&tab, inst->resultado, inst->operando1);
                        continue;
                    }
                    if (strcmp(inst->operador, "==") == 0 || strcmp(inst->operador, "!=") == 0) {
                        size_t len1 = strlen(inst->operando1);
                        size_t len2 = strlen(inst->operando2);
                        bool iguais = (len1 == len2 && memcmp(inst->operando1 + 1, inst->operando2 + 1, len1 - 2) == 0);
                        bool res = (strcmp(inst->operador, "==") == 0) ? iguais : !iguais;

                        inst->tipo = TAC_ASSIGN;
                        free(inst->operando1);
                        inst->operando1 = strdup(res ? "True" : "False");
                        free(inst->operando2);
                        inst->operando2 = NULL;
                        free(inst->operador);
                        inst->operador = NULL;
                        modificou_passo = true;
                        modificou_geral = true;
                        const_tab_definir(&tab, inst->resultado, inst->operando1);
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
                    modificou_passo = true;
                    modificou_geral = true;
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
                    modificou_passo = true;
                    modificou_geral = true;
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
                    modificou_passo = true;
                    modificou_geral = true;
                    const_tab_definir(&tab, inst->resultado, "0");
                    continue;
                }
                if (v1_int && strcmp(inst->operador, "*") == 0 && v1 == 0) {
                    // 0 * x -> 0
                    inst->tipo = TAC_ASSIGN;
                    free(inst->operando2);
                    inst->operando2 = NULL;
                    free(inst->operador);
                    inst->operador = NULL;
                    modificou_passo = true;
                    modificou_geral = true;
                    const_tab_definir(&tab, inst->resultado, "0");
                    continue;
                }
                if (v2_int && strcmp(inst->operador, "+") == 0 && v2 == 0) {
                    // x + 0 -> x
                    inst->tipo = TAC_ASSIGN;
                    free(inst->operando2);
                    inst->operando2 = NULL;
                    free(inst->operador);
                    inst->operador = NULL;
                    modificou_passo = true;
                    modificou_geral = true;
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
                    modificou_passo = true;
                    modificou_geral = true;
                    continue;
                }
                if (v2_int && strcmp(inst->operador, "-") == 0 && v2 == 0) {
                    // x - 0 -> x
                    inst->tipo = TAC_ASSIGN;
                    free(inst->operando2);
                    inst->operando2 = NULL;
                    free(inst->operador);
                    inst->operador = NULL;
                    modificou_passo = true;
                    modificou_geral = true;
                    continue;
                }
                if (v2_int && strcmp(inst->operador, "/") == 0 && v2 == 1) {
                    // x / 1 -> x
                    inst->tipo = TAC_ASSIGN;
                    free(inst->operando2);
                    inst->operando2 = NULL;
                    free(inst->operador);
                    inst->operador = NULL;
                    modificou_passo = true;
                    modificou_geral = true;
                    continue;
                }

                // x - x -> 0
                if (strcmp(inst->operador, "-") == 0 && strcmp(inst->operando1, inst->operando2) == 0) {
                    inst->tipo = TAC_ASSIGN;
                    free(inst->operando1);
                    inst->operando1 = strdup("0");
                    free(inst->operando2);
                    inst->operando2 = NULL;
                    free(inst->operador);
                    inst->operador = NULL;
                    modificou_passo = true;
                    modificou_geral = true;
                    const_tab_definir(&tab, inst->resultado, "0");
                    continue;
                }

                // Simplificações booleanas algébricas
                if (strcmp(inst->operador, "and") == 0) {
                    if (strcmp(inst->operando2, "True") == 0) {
                        // x and True -> x
                        inst->tipo = TAC_ASSIGN;
                        free(inst->operando2);
                        inst->operando2 = NULL;
                        free(inst->operador);
                        inst->operador = NULL;
                        modificou_passo = true;
                        modificou_geral = true;
                        continue;
                    }
                    if (strcmp(inst->operando1, "True") == 0) {
                        // True and x -> x
                        inst->tipo = TAC_ASSIGN;
                        free(inst->operando1);
                        inst->operando1 = inst->operando2;
                        inst->operando2 = NULL;
                        free(inst->operador);
                        inst->operador = NULL;
                        modificou_passo = true;
                        modificou_geral = true;
                        continue;
                    }
                    if (strcmp(inst->operando2, "False") == 0) {
                        // x and False -> False
                        inst->tipo = TAC_ASSIGN;
                        free(inst->operando1);
                        inst->operando1 = strdup("False");
                        free(inst->operando2);
                        inst->operando2 = NULL;
                        free(inst->operador);
                        inst->operador = NULL;
                        modificou_passo = true;
                        modificou_geral = true;
                        const_tab_definir(&tab, inst->resultado, "False");
                        continue;
                    }
                    if (strcmp(inst->operando1, "False") == 0) {
                        // False and x -> False
                        inst->tipo = TAC_ASSIGN;
                        free(inst->operando2);
                        inst->operando2 = NULL;
                        free(inst->operador);
                        inst->operador = NULL;
                        modificou_passo = true;
                        modificou_geral = true;
                        const_tab_definir(&tab, inst->resultado, "False");
                        continue;
                    }
                }

                if (strcmp(inst->operador, "or") == 0) {
                    if (strcmp(inst->operando2, "False") == 0) {
                        // x or False -> x
                        inst->tipo = TAC_ASSIGN;
                        free(inst->operando2);
                        inst->operando2 = NULL;
                        free(inst->operador);
                        inst->operador = NULL;
                        modificou_passo = true;
                        modificou_geral = true;
                        continue;
                    }
                    if (strcmp(inst->operando1, "False") == 0) {
                        // False or x -> x
                        inst->tipo = TAC_ASSIGN;
                        free(inst->operando1);
                        inst->operando1 = inst->operando2;
                        inst->operando2 = NULL;
                        free(inst->operador);
                        inst->operador = NULL;
                        modificou_passo = true;
                        modificou_geral = true;
                        continue;
                    }
                    if (strcmp(inst->operando2, "True") == 0) {
                        // x or True -> True
                        inst->tipo = TAC_ASSIGN;
                        free(inst->operando1);
                        inst->operando1 = strdup("True");
                        free(inst->operando2);
                        inst->operando2 = NULL;
                        free(inst->operador);
                        inst->operador = NULL;
                        modificou_passo = true;
                        modificou_geral = true;
                        const_tab_definir(&tab, inst->resultado, "True");
                        continue;
                    }
                    if (strcmp(inst->operando1, "True") == 0) {
                        // True or x -> True
                        inst->tipo = TAC_ASSIGN;
                        free(inst->operando2);
                        inst->operando2 = NULL;
                        free(inst->operador);
                        inst->operador = NULL;
                        modificou_passo = true;
                        modificou_geral = true;
                        const_tab_definir(&tab, inst->resultado, "True");
                        continue;
                    }
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
                    modificou_passo = true;
                    modificou_geral = true;
                    const_tab_definir(&tab, inst->resultado, inst->operando1);
                    continue;
                }
                bool b;
                if (strcmp(inst->operador, "not") == 0 && e_booleano(inst->operando1, &b)) {
                    inst->tipo = TAC_ASSIGN;
                    free(inst->operando1);
                    inst->operando1 = strdup(b ? "False" : "True");
                    free(inst->operador);
                    inst->operador = NULL;
                    modificou_passo = true;
                    modificou_geral = true;
                    const_tab_definir(&tab, inst->resultado, inst->operando1);
                    continue;
                }
            }

            // Atualização da tabela de constantes para TAC_ASSIGN
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
        passadas++;
    }

    return modificou_geral;
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

/* ── Otimização direta sobre a AST (para CodeGen C#) ─────────────── */

static NoAST *criar_literal_int_tipado(int valor, int linha) {
    NoAST *novo = criar_no_int(valor, linha);
    novo->tipo_resolvido = TIPO_INT;
    return novo;
}

static NoAST *criar_literal_float_tipado(double valor, int linha) {
    NoAST *novo = criar_no_float(valor, linha);
    novo->tipo_resolvido = TIPO_FLOAT;
    return novo;
}

static NoAST *criar_literal_bool_tipado(int valor, int linha) {
    NoAST *novo = criar_no_bool(valor ? 1 : 0, linha);
    novo->tipo_resolvido = TIPO_BOOL;
    return novo;
}

static bool ast_e_literal(const NoAST *no) {
    if (no == NULL) return false;
    return no->tipo == NO_NUM_INT || no->tipo == NO_NUM_FLOAT ||
           no->tipo == NO_BOOL || no->tipo == NO_STRING;
}

static char *ast_literal_para_texto_alocado(const NoAST *no) {
    if (no == NULL) return NULL;
    char buf[64];
    switch (no->tipo) {
        case NO_NUM_INT:
            snprintf(buf, sizeof(buf), "%d", no->valor_int);
            return strdup(buf);
        case NO_NUM_FLOAT:
            snprintf(buf, sizeof(buf), "%.17g", no->valor_float);
            if (strpbrk(buf, ".eE") == NULL) strcat(buf, ".0");
            return strdup(buf);
        case NO_BOOL:
            return strdup(no->valor_int ? "True" : "False");
        case NO_STRING:
            return no->valor_string ? strdup(no->valor_string) : NULL;
        default:
            return NULL;
    }
}

static NoAST *ast_criar_literal_de_texto(const char *val, TipoDado tipo_esperado, int linha) {
    if (val == NULL) return NULL;

    bool b = false;
    if (e_booleano(val, &b)) {
        return criar_literal_bool_tipado(b ? 1 : 0, linha);
    }

    long v_long = 0;
    if (e_inteiro(val, &v_long)) {
        if (tipo_esperado == TIPO_FLOAT) {
            return criar_literal_float_tipado((double)v_long, linha);
        }
        if (v_long >= INT_MIN && v_long <= INT_MAX) {
            return criar_literal_int_tipado((int)v_long, linha);
        }
        return NULL;
    }

    if (e_string_literal(val)) {
        NoAST *n = criar_no_string((char *)val, linha);
        n->tipo_resolvido = TIPO_STRING;
        return n;
    }

    double d = 0.0;
    if (e_flutuante(val, &d)) {
        return criar_literal_float_tipado(d, linha);
    }

    return NULL;
}

static bool ast_expressao_pura(const NoAST *no) {
    if (no == NULL) return true;
    if (no->tipo == NO_FUNCCALL || no->tipo == NO_INPUT || no->tipo == NO_ASSIGN) {
        return false;
    }
    if (!ast_expressao_pura(no->esquerda) ||
        !ast_expressao_pura(no->direita) ||
        !ast_expressao_pura(no->condicao) ||
        !ast_expressao_pura(no->bloco_if) ||
        !ast_expressao_pura(no->bloco_else)) {
        return false;
    }
    for (int i = 0; i < no->num_argumentos; i++) {
        if (!ast_expressao_pura(no->argumentos[i])) return false;
    }
    return true;
}

static bool ast_e_zero_num(const NoAST *no) {
    if (no == NULL) return false;
    if (no->tipo == NO_NUM_INT) return no->valor_int == 0;
    if (no->tipo == NO_NUM_FLOAT) return no->valor_float == 0.0;
    return false;
}

static bool ast_e_um_num(const NoAST *no) {
    if (no == NULL) return false;
    if (no->tipo == NO_NUM_INT) return no->valor_int == 1;
    if (no->tipo == NO_NUM_FLOAT) return no->valor_float == 1.0;
    return false;
}

static NoAST *promover_filho_ast(NoAST *pai, bool manter_esquerda, bool *mudou) {
    NoAST *sobrevivente;
    if (manter_esquerda) {
        sobrevivente = pai->esquerda;
        pai->esquerda = NULL;
    } else {
        sobrevivente = pai->direita;
        pai->direita = NULL;
    }
    liberar_ast(pai);
    *mudou = true;
    return sobrevivente;
}

/*
 * Converte o conteúdo interno de um literal de string Python ('...' ou "...")
 * para uma forma normalizada segura dentro de aspas duplas ("...").
 * Se out == NULL, apenas calcula o tamanho necessário.
 */
static size_t normalizar_conteudo_string(const char *lit, char *out) {
    if (!lit) return 0;
    size_t len = strlen(lit);
    if (len < 2) return 0;

    char aspa = lit[0];
    size_t pos = 0;
    for (size_t i = 1; i + 1 < len; i++) {
        char c = lit[i];
        if (c == '\\' && i + 2 < len) {
            char e = lit[++i];
            if (aspa == '\'' && e == '\'') {
                if (out) out[pos] = '\'';
                pos++;
            } else {
                if (out) {
                    out[pos] = '\\';
                    out[pos + 1] = e;
                }
                pos += 2;
            }
        } else if (c == '"') {
            if (out) {
                out[pos] = '\\';
                out[pos + 1] = '"';
            }
            pos += 2;
        } else {
            if (out) out[pos] = c;
            pos++;
        }
    }
    if (out) out[pos] = '\0';
    return pos;
}

static char *normalizar_string_alocada(const char *lit) {
    size_t tam = normalizar_conteudo_string(lit, NULL);
    char *buf = malloc(tam + 1);
    if (!buf) {
        fprintf(stderr, "Erro: falha ao alocar memória no otimizador.\n");
        exit(1);
    }
    normalizar_conteudo_string(lit, buf);
    return buf;
}

static NoAST *otimizar_folding_numerico_e_unario_ast(NoAST *no, bool *mudou) {
    if (no->tipo == NO_UNARYOP && no->esquerda != NULL) {
        NoAST *op = no->esquerda;
        if (no->operador == OP_NEG && op->tipo == NO_NUM_INT && op->valor_int != INT_MIN) {
            NoAST *novo = criar_literal_int_tipado(-op->valor_int, no->linha);
            liberar_ast(no);
            *mudou = true;
            return novo;
        }
        if (no->operador == OP_NEG && op->tipo == NO_NUM_FLOAT) {
            NoAST *novo = criar_literal_float_tipado(-op->valor_float, no->linha);
            liberar_ast(no);
            *mudou = true;
            return novo;
        }
        if (no->operador == OP_NOT && op->tipo == NO_BOOL) {
            NoAST *novo = criar_literal_bool_tipado(!op->valor_int, no->linha);
            liberar_ast(no);
            *mudou = true;
            return novo;
        }
        return no;
    }

    if (no->tipo != NO_BINOP || no->esquerda == NULL || no->direita == NULL) {
        return no;
    }

    NoAST *esq = no->esquerda;
    NoAST *dir = no->direita;

    if (esq->tipo == NO_NUM_INT && dir->tipo == NO_NUM_INT) {
        long long v1 = esq->valor_int;
        long long v2 = dir->valor_int;
        if (no->operador == OP_SOMA || no->operador == OP_SUB || no->operador == OP_MULT) {
            long long res = 0;
            if (no->operador == OP_SOMA) res = v1 + v2;
            else if (no->operador == OP_SUB) res = v1 - v2;
            else res = v1 * v2;

            if (res >= INT_MIN && res <= INT_MAX) {
                NoAST *novo = criar_literal_int_tipado((int)res, no->linha);
                liberar_ast(no);
                *mudou = true;
                return novo;
            }
        } else if (no->operador == OP_DIV && v2 != 0) {
            NoAST *novo = criar_literal_float_tipado((double)v1 / (double)v2, no->linha);
            liberar_ast(no);
            *mudou = true;
            return novo;
        } else if (no->operador == OP_EQ || no->operador == OP_NEQ ||
                   no->operador == OP_LT || no->operador == OP_LTE ||
                   no->operador == OP_GT || no->operador == OP_GTE) {
            int res = 0;
            switch (no->operador) {
                case OP_EQ:  res = (v1 == v2); break;
                case OP_NEQ: res = (v1 != v2); break;
                case OP_LT:  res = (v1 < v2);  break;
                case OP_LTE: res = (v1 <= v2); break;
                case OP_GT:  res = (v1 > v2);  break;
                case OP_GTE: res = (v1 >= v2); break;
            }
            NoAST *novo = criar_literal_bool_tipado(res, no->linha);
            liberar_ast(no);
            *mudou = true;
            return novo;
        }
    }

    bool esq_num = (esq->tipo == NO_NUM_INT || esq->tipo == NO_NUM_FLOAT);
    bool dir_num = (dir->tipo == NO_NUM_INT || dir->tipo == NO_NUM_FLOAT);
    if (esq_num && dir_num && (esq->tipo == NO_NUM_FLOAT || dir->tipo == NO_NUM_FLOAT)) {
        double f1 = (esq->tipo == NO_NUM_FLOAT) ? esq->valor_float : (double)esq->valor_int;
        double f2 = (dir->tipo == NO_NUM_FLOAT) ? dir->valor_float : (double)dir->valor_int;
        if (no->operador == OP_SOMA || no->operador == OP_SUB ||
            no->operador == OP_MULT || (no->operador == OP_DIV && f2 != 0.0)) {
            double res = 0.0;
            switch (no->operador) {
                case OP_SOMA: res = f1 + f2; break;
                case OP_SUB:  res = f1 - f2; break;
                case OP_MULT: res = f1 * f2; break;
                case OP_DIV:  res = f1 / f2; break;
            }
            NoAST *novo = criar_literal_float_tipado(res, no->linha);
            liberar_ast(no);
            *mudou = true;
            return novo;
        } else if (no->operador == OP_EQ || no->operador == OP_NEQ ||
                   no->operador == OP_LT || no->operador == OP_LTE ||
                   no->operador == OP_GT || no->operador == OP_GTE) {
            int res = 0;
            switch (no->operador) {
                case OP_EQ:  res = (f1 == f2); break;
                case OP_NEQ: res = (f1 != f2); break;
                case OP_LT:  res = (f1 < f2);  break;
                case OP_LTE: res = (f1 <= f2); break;
                case OP_GT:  res = (f1 > f2);  break;
                case OP_GTE: res = (f1 >= f2); break;
            }
            NoAST *novo = criar_literal_bool_tipado(res, no->linha);
            liberar_ast(no);
            *mudou = true;
            return novo;
        }
    }

    if (esq->tipo == NO_BOOL && dir->tipo == NO_BOOL) {
        int b1 = esq->valor_int ? 1 : 0;
        int b2 = dir->valor_int ? 1 : 0;
        int res = 0;
        bool calculado = true;
        switch (no->operador) {
            case OP_AND: res = (b1 && b2); break;
            case OP_OR:  res = (b1 || b2); break;
            case OP_EQ:  res = (b1 == b2); break;
            case OP_NEQ: res = (b1 != b2); break;
            case OP_LT:  res = (b1 < b2);  break;
            case OP_LTE: res = (b1 <= b2); break;
            case OP_GT:  res = (b1 > b2);  break;
            case OP_GTE: res = (b1 >= b2); break;
            default:     calculado = false; break;
        }
        if (calculado) {
            NoAST *novo = criar_literal_bool_tipado(res, no->linha);
            liberar_ast(no);
            *mudou = true;
            return novo;
        }
    }

    if (esq->tipo == NO_STRING && dir->tipo == NO_STRING &&
        esq->valor_string != NULL && dir->valor_string != NULL) {
        if (no->operador == OP_SOMA) {
            char *n1 = normalizar_string_alocada(esq->valor_string);
            char *n2 = normalizar_string_alocada(dir->valor_string);
            size_t len1 = strlen(n1);
            size_t len2 = strlen(n2);
            char *concat = malloc(len1 + len2 + 3);
            if (!concat) {
                fprintf(stderr, "Erro: falha ao alocar memória no otimizador.\n");
                exit(1);
            }
            concat[0] = '"';
            memcpy(concat + 1, n1, len1);
            memcpy(concat + 1 + len1, n2, len2);
            concat[1 + len1 + len2] = '"';
            concat[2 + len1 + len2] = '\0';

            NoAST *novo = criar_no_string(concat, no->linha);
            novo->tipo_resolvido = TIPO_STRING;
            free(n1);
            free(n2);
            free(concat);
            liberar_ast(no);
            *mudou = true;
            return novo;
        }
        if (no->operador == OP_EQ || no->operador == OP_NEQ) {
            char *n1 = normalizar_string_alocada(esq->valor_string);
            char *n2 = normalizar_string_alocada(dir->valor_string);
            bool iguais = (strcmp(n1, n2) == 0);
            int res = (no->operador == OP_EQ) ? iguais : !iguais;
            free(n1);
            free(n2);

            NoAST *novo = criar_literal_bool_tipado(res, no->linha);
            liberar_ast(no);
            *mudou = true;
            return novo;
        }
    }

    return no;
}

static NoAST *otimizar_simplificacao_algebrica_ast(NoAST *no, bool *mudou) {
    if (no == NULL || no->tipo != NO_BINOP || no->esquerda == NULL || no->direita == NULL) {
        return no;
    }

    NoAST *esq = no->esquerda;
    NoAST *dir = no->direita;

    /* Identidades aditivas e multiplicativas (preservando promocao para float) */
    if (no->operador == OP_SOMA) {
        if (ast_e_zero_num(dir) && (dir->tipo == NO_NUM_INT || esq->tipo_resolvido == TIPO_FLOAT)) {
            return promover_filho_ast(no, true, mudou);
        }
        if (ast_e_zero_num(esq) && (esq->tipo == NO_NUM_INT || dir->tipo_resolvido == TIPO_FLOAT)) {
            return promover_filho_ast(no, false, mudou);
        }
    } else if (no->operador == OP_SUB) {
        if (ast_e_zero_num(dir) && (dir->tipo == NO_NUM_INT || esq->tipo_resolvido == TIPO_FLOAT)) {
            return promover_filho_ast(no, true, mudou);
        }
        if (esq->tipo == NO_ID && dir->tipo == NO_ID &&
            esq->nome != NULL && dir->nome != NULL && strcmp(esq->nome, dir->nome) == 0) {
            NoAST *novo = (esq->tipo_resolvido == TIPO_FLOAT)
                          ? criar_literal_float_tipado(0.0, no->linha)
                          : criar_literal_int_tipado(0, no->linha);
            liberar_ast(no);
            *mudou = true;
            return novo;
        }
    } else if (no->operador == OP_MULT) {
        if (ast_e_um_num(dir) && (dir->tipo == NO_NUM_INT || esq->tipo_resolvido == TIPO_FLOAT)) {
            return promover_filho_ast(no, true, mudou);
        }
        if (ast_e_um_num(esq) && (esq->tipo == NO_NUM_INT || dir->tipo_resolvido == TIPO_FLOAT)) {
            return promover_filho_ast(no, false, mudou);
        }
        if (ast_e_zero_num(dir) && esq->tipo_resolvido != TIPO_STRING && ast_expressao_pura(esq)) {
            bool e_float = (dir->tipo == NO_NUM_FLOAT || esq->tipo_resolvido == TIPO_FLOAT);
            NoAST *novo = e_float ? criar_literal_float_tipado(0.0, no->linha)
                                  : criar_literal_int_tipado(0, no->linha);
            liberar_ast(no);
            *mudou = true;
            return novo;
        }
        if (ast_e_zero_num(esq) && dir->tipo_resolvido != TIPO_STRING && ast_expressao_pura(dir)) {
            bool e_float = (esq->tipo == NO_NUM_FLOAT || dir->tipo_resolvido == TIPO_FLOAT);
            NoAST *novo = e_float ? criar_literal_float_tipado(0.0, no->linha)
                                  : criar_literal_int_tipado(0, no->linha);
            liberar_ast(no);
            *mudou = true;
            return novo;
        }
    } else if (no->operador == OP_DIV) {
        /* x / 1 so simplifica para x quando x ja e float (Python 3: int / int -> float) */
        if (ast_e_um_num(dir) && esq->tipo_resolvido == TIPO_FLOAT) {
            return promover_filho_ast(no, true, mudou);
        }
    } else if (no->operador == OP_AND) {
        if (dir->tipo == NO_BOOL && dir->valor_int) {
            return promover_filho_ast(no, true, mudou);
        }
        if (esq->tipo == NO_BOOL && esq->valor_int) {
            return promover_filho_ast(no, false, mudou);
        }
        if (esq->tipo == NO_BOOL && !esq->valor_int) {
            NoAST *novo = criar_literal_bool_tipado(0, no->linha);
            liberar_ast(no);
            *mudou = true;
            return novo;
        }
        if (dir->tipo == NO_BOOL && !dir->valor_int && ast_expressao_pura(esq)) {
            NoAST *novo = criar_literal_bool_tipado(0, no->linha);
            liberar_ast(no);
            *mudou = true;
            return novo;
        }
    } else if (no->operador == OP_OR) {
        if (dir->tipo == NO_BOOL && !dir->valor_int) {
            return promover_filho_ast(no, true, mudou);
        }
        if (esq->tipo == NO_BOOL && !esq->valor_int) {
            return promover_filho_ast(no, false, mudou);
        }
        if (esq->tipo == NO_BOOL && esq->valor_int) {
            NoAST *novo = criar_literal_bool_tipado(1, no->linha);
            liberar_ast(no);
            *mudou = true;
            return novo;
        }
        if (dir->tipo == NO_BOOL && dir->valor_int && ast_expressao_pura(esq)) {
            NoAST *novo = criar_literal_bool_tipado(1, no->linha);
            liberar_ast(no);
            *mudou = true;
            return novo;
        }
    }

    return no;
}

static NoAST *otimizar_expr_ast(NoAST *no, TabelaConstantes *tab, bool *mudou) {
    if (no == NULL) return NULL;

    if (no->tipo == NO_ID) {
        if (tab != NULL && no->nome != NULL) {
            const char *val = const_tab_obter(tab, no->nome);
            if (val != NULL) {
                NoAST *lit = ast_criar_literal_de_texto(val, no->tipo_resolvido, no->linha);
                if (lit != NULL) {
                    liberar_ast(no);
                    *mudou = true;
                    return lit;
                }
            }
        }
        return no;
    }

    if (no->esquerda != NULL) no->esquerda = otimizar_expr_ast(no->esquerda, tab, mudou);
    if (no->direita != NULL)  no->direita = otimizar_expr_ast(no->direita, tab, mudou);

    if (no->tipo == NO_FUNCCALL || no->tipo == NO_BLOCO) {
        for (int i = 0; i < no->num_argumentos; i++) {
            no->argumentos[i] = otimizar_expr_ast(no->argumentos[i], tab, mudou);
        }
    }

    if (no->tipo == NO_BINOP || no->tipo == NO_UNARYOP) {
        no = otimizar_folding_numerico_e_unario_ast(no, mudou);
        if (no->tipo == NO_BINOP) {
            no = otimizar_simplificacao_algebrica_ast(no, mudou);
        }
        return no;
    }

    return no;
}

static void otimizar_comando_ast(NoAST *no, TabelaConstantes *tab, bool *mudou);

static void remover_comando_bloco(NoAST *bloco, int idx) {
    if (bloco == NULL || idx < 0 || idx >= bloco->num_argumentos) return;
    liberar_ast(bloco->argumentos[idx]);
    for (int j = idx; j < bloco->num_argumentos - 1; j++) {
        bloco->argumentos[j] = bloco->argumentos[j + 1];
    }
    bloco->num_argumentos--;
}

static void substituir_comando_por_bloco(NoAST *container, int idx, NoAST *bloco_origem) {
    NoAST *antigo = container->argumentos[idx];

    if (bloco_origem == NULL) {
        remover_comando_bloco(container, idx);
        return;
    }

    if (bloco_origem->tipo != NO_BLOCO) {
        container->argumentos[idx] = bloco_origem;
        liberar_ast(antigo);
        return;
    }

    int k = bloco_origem->num_argumentos;
    if (k <= 0) {
        remover_comando_bloco(container, idx);
        return;
    }

    int n_antigo = container->num_argumentos;
    int n_novo = n_antigo - 1 + k;
    if (k > 1) {
        NoAST **novo_arr = realloc(container->argumentos, sizeof(NoAST *) * n_novo);
        if (!novo_arr) {
            fprintf(stderr, "Erro: falha ao alocar memória no otimizador.\n");
            exit(1);
        }
        container->argumentos = novo_arr;
    }
    if (n_antigo - 1 - idx > 0) {
        memmove(&container->argumentos[idx + k],
                &container->argumentos[idx + 1],
                sizeof(NoAST *) * (n_antigo - 1 - idx));
    }
    memcpy(&container->argumentos[idx], bloco_origem->argumentos, sizeof(NoAST *) * k);
    container->num_argumentos = n_novo;

    free(bloco_origem->argumentos);
    bloco_origem->argumentos = NULL;
    bloco_origem->num_argumentos = 0;

    liberar_ast(antigo);
}

static void otimizar_bloco_ast(NoAST *bloco, TabelaConstantes *tab, bool *mudou) {
    if (bloco == NULL) return;

    for (int i = 0; i < bloco->num_argumentos; i++) {
        NoAST *cmd = bloco->argumentos[i];
        if (cmd == NULL) {
            remover_comando_bloco(bloco, i);
            i--;
            continue;
        }

        if (cmd->tipo == NO_WHILE) {
            if (tab) const_tab_limpar(tab);
            cmd->condicao = otimizar_expr_ast(cmd->condicao, tab, mudou);
            if (tab) const_tab_limpar(tab);

            if (cmd->condicao != NULL && cmd->condicao->tipo == NO_BOOL && !cmd->condicao->valor_int) {
                remover_comando_bloco(bloco, i);
                *mudou = true;
                i--;
                continue;
            }

            otimizar_comando_ast(cmd->bloco_if, tab, mudou);
            if (tab) const_tab_limpar(tab);
            continue;
        }

        if (cmd->tipo == NO_IF) {
            cmd->condicao = otimizar_expr_ast(cmd->condicao, tab, mudou);

            if (cmd->condicao != NULL && cmd->condicao->tipo == NO_BOOL && cmd->condicao->valor_int) {
                NoAST *bloco_vivo = cmd->bloco_if;
                if (bloco_vivo != NULL && bloco_vivo->tipo != NO_BLOCO) {
                    cmd->bloco_if = NULL;
                }
                substituir_comando_por_bloco(bloco, i, bloco_vivo);
                *mudou = true;
                i--;
                continue;
            }

            if (cmd->condicao != NULL && cmd->condicao->tipo == NO_BOOL && !cmd->condicao->valor_int) {
                if (cmd->num_argumentos > 0) {
                    NoAST *primeiro_elif = cmd->argumentos[0];
                    liberar_ast(cmd->condicao);
                    liberar_ast(cmd->bloco_if);
                    cmd->condicao = primeiro_elif->condicao;
                    cmd->bloco_if = primeiro_elif->bloco_if;
                    cmd->linha = primeiro_elif->linha;
                    primeiro_elif->condicao = NULL;
                    primeiro_elif->bloco_if = NULL;
                    liberar_ast(primeiro_elif);

                    for (int j = 0; j < cmd->num_argumentos - 1; j++) {
                        cmd->argumentos[j] = cmd->argumentos[j + 1];
                    }
                    cmd->num_argumentos--;
                    if (cmd->num_argumentos == 0) {
                        free(cmd->argumentos);
                        cmd->argumentos = NULL;
                    }
                    *mudou = true;
                    i--;
                    continue;
                }

                if (cmd->bloco_else != NULL) {
                    NoAST *bloco_vivo = cmd->bloco_else;
                    if (bloco_vivo->tipo != NO_BLOCO) {
                        cmd->bloco_else = NULL;
                    }
                    substituir_comando_por_bloco(bloco, i, bloco_vivo);
                    *mudou = true;
                    i--;
                    continue;
                }

                remover_comando_bloco(bloco, i);
                *mudou = true;
                i--;
                continue;
            }

            if (tab) const_tab_limpar(tab);
            otimizar_comando_ast(cmd->bloco_if, tab, mudou);
            if (tab) const_tab_limpar(tab);

            for (int e = 0; e < cmd->num_argumentos; e++) {
                NoAST *elif_no = cmd->argumentos[e];
                if (elif_no == NULL) continue;
                elif_no->condicao = otimizar_expr_ast(elif_no->condicao, tab, mudou);
                if (tab) const_tab_limpar(tab);

                if (elif_no->condicao != NULL && elif_no->condicao->tipo == NO_BOOL && !elif_no->condicao->valor_int) {
                    liberar_ast(elif_no);
                    for (int j = e; j < cmd->num_argumentos - 1; j++) {
                        cmd->argumentos[j] = cmd->argumentos[j + 1];
                    }
                    cmd->num_argumentos--;
                    if (cmd->num_argumentos == 0) {
                        free(cmd->argumentos);
                        cmd->argumentos = NULL;
                    }
                    *mudou = true;
                    e--;
                    continue;
                }

                otimizar_comando_ast(elif_no->bloco_if, tab, mudou);
                if (tab) const_tab_limpar(tab);
            }

            otimizar_comando_ast(cmd->bloco_else, tab, mudou);
            if (tab) const_tab_limpar(tab);
            continue;
        }

        if (cmd->tipo == NO_RETURN) {
            cmd->esquerda = otimizar_expr_ast(cmd->esquerda, tab, mudou);
            if (i + 1 < bloco->num_argumentos) {
                for (int j = i + 1; j < bloco->num_argumentos; j++) {
                    liberar_ast(bloco->argumentos[j]);
                }
                bloco->num_argumentos = i + 1;
                *mudou = true;
            }
            break;
        }

        otimizar_comando_ast(cmd, tab, mudou);
    }
}

static void otimizar_comando_ast(NoAST *no, TabelaConstantes *tab, bool *mudou) {
    if (no == NULL) return;

    switch (no->tipo) {
        case NO_PROGRAMA:
        case NO_BLOCO:
            otimizar_bloco_ast(no, tab, mudou);
            break;

        case NO_ASSIGN:
            no->esquerda = otimizar_expr_ast(no->esquerda, tab, mudou);
            if (tab != NULL && no->nome != NULL) {
                if (ast_e_literal(no->esquerda)) {
                    char *val = ast_literal_para_texto_alocado(no->esquerda);
                    if (val != NULL) {
                        const_tab_definir(tab, no->nome, val);
                        free(val);
                    }
                } else {
                    const_tab_invalidar(tab, no->nome);
                }
            }
            break;

        case NO_RETURN:
        case NO_INPUT:
            no->esquerda = otimizar_expr_ast(no->esquerda, tab, mudou);
            break;

        case NO_PRINT:
        case NO_FUNCCALL:
            for (int i = 0; i < no->num_argumentos; i++) {
                no->argumentos[i] = otimizar_expr_ast(no->argumentos[i], tab, mudou);
            }
            break;

        case NO_IF:
            no->condicao = otimizar_expr_ast(no->condicao, tab, mudou);
            if (tab) const_tab_limpar(tab);
            otimizar_comando_ast(no->bloco_if, tab, mudou);
            if (tab) const_tab_limpar(tab);
            for (int i = 0; i < no->num_argumentos; i++) {
                NoAST *elif_no = no->argumentos[i];
                if (elif_no != NULL) {
                    elif_no->condicao = otimizar_expr_ast(elif_no->condicao, tab, mudou);
                    if (tab) const_tab_limpar(tab);
                    otimizar_comando_ast(elif_no->bloco_if, tab, mudou);
                    if (tab) const_tab_limpar(tab);
                }
            }
            otimizar_comando_ast(no->bloco_else, tab, mudou);
            if (tab) const_tab_limpar(tab);
            break;

        case NO_WHILE:
            if (tab) const_tab_limpar(tab);
            no->condicao = otimizar_expr_ast(no->condicao, tab, mudou);
            if (tab) const_tab_limpar(tab);
            otimizar_comando_ast(no->bloco_if, tab, mudou);
            if (tab) const_tab_limpar(tab);
            break;

        case NO_FOR:
            no->esquerda = otimizar_expr_ast(no->esquerda, tab, mudou);
            if (tab) const_tab_limpar(tab);
            otimizar_comando_ast(no->bloco_if, tab, mudou);
            if (tab) const_tab_limpar(tab);
            break;

        case NO_FUNCDEF: {
            TabelaConstantes fn_tab;
            const_tab_init(&fn_tab);
            otimizar_comando_ast(no->bloco_if, &fn_tab, mudou);
            const_tab_destruir(&fn_tab);
            break;
        }

        default:
            break;
    }
}

int otimizar_ast(NoAST *raiz) {
    if (raiz == NULL) return 0;

    int iteracoes = 0;
    while (iteracoes < 20) {
        bool modificou = false;
        TabelaConstantes tab;
        const_tab_init(&tab);
        otimizar_comando_ast(raiz, &tab, &modificou);
        const_tab_destruir(&tab);
        if (!modificou) break;
        iteracoes++;
    }

    return iteracoes;
}
