#ifndef TIPOS_H
#define TIPOS_H

typedef enum {
    TIPO_INT,
    TIPO_FLOAT,
    TIPO_STRING,
    TIPO_BOOL,
    TIPO_VOID,
    TIPO_DESCONHECIDO
} TipoDado;

static inline const char *tipo_para_string(TipoDado tipo) {
    switch (tipo) {
        case TIPO_INT:          return "int";
        case TIPO_FLOAT:        return "float";
        case TIPO_STRING:       return "string";
        case TIPO_BOOL:         return "bool";
        case TIPO_VOID:         return "void";
        case TIPO_DESCONHECIDO: return "desconhecido";
        default:                return "?";
    }
}

#endif
