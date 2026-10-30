#ifndef CODEGEN_INTERMEDIARIO_H
#define CODEGEN_INTERMEDIARIO_H
typedef enum{
    TAC_ASSIGN,
    TAC_BINOP,
    TAC_UNOP,
    TAC_LABEL,
    TAC_GOTO,
    TAC_IF_FALSE,
    TAC_CALL,
    TAC_RETURN,
    TAC_PARAM,
    TAC_PRINT,
    TAC_FUNC_BEGIN,
    TAC_FUNC_END
}TipoInstrucao;

typedef struct{
    TipoInstrucao tipo;
    char* resultado;
    char* operando1;
    char* operando2;
    char* operador;


}Instrucao;

typedef struct
{
Instrucao *Instrucoes;
int tamanho;
int capacidade;
}TAC;

Instrucao cria_instrucao(TipoInstrucao tipo,const char* resultado, const char* op1, const char* op2,const char* op);
char* novo_temporario(void);
char* novo_rotulo(void);
void emitir(Instrucao inst);
void imprimir_codigo_intermediario(void);
void liberar_tac(void);
TAC* obter_tac(void);

#endif