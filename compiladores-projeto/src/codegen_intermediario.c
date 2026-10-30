#include <stdio.h>
#include<stdlib.h>
#include <string.h>
#include "codegen_intermediario.h"
static TAC lista_tac = {NULL, 0,0};
static int contador_temp =0;
static int contador_rotulo = 0;
TAC* obter_tac(void){
    return &lista_tac;
}
 char* novo_temporario(){
    char* buffer = malloc(16);
    snprintf(buffer, 16, "t%d", contador_temp++);
    return buffer;
}

char* novo_rotulo(){
    char* buffer = malloc(16);
    snprintf(buffer, 16, "L%d", contador_rotulo++);
    return buffer;
}

 Instrucao cria_instrucao(TipoInstrucao tipo,const char* resultado, const char* op1, const char* op2,const char* op){
    Instrucao inst;
    inst.tipo=tipo;
    if(resultado!= NULL){
        inst.resultado = strdup(resultado);
    }else{
        inst.resultado = NULL;
    }
        if(op1!= NULL){
        inst.operando1 = strdup(op1);
    }else{
        inst.operando1 = NULL;
    }
        if(op2!= NULL){
        inst.operando2 = strdup(op2);
    }else{
        inst.operando2 = NULL;
    }
        if(op!= NULL){
        inst.operador = strdup(op);
    }else{
        inst.operador = NULL;
    }
    return inst;

}

void emitir(Instrucao inst){
    if(lista_tac.tamanho>=lista_tac.capacidade && lista_tac.capacidade==0){
        lista_tac.capacidade=10;
        lista_tac.Instrucoes = realloc(lista_tac.Instrucoes, sizeof(Instrucao) * lista_tac.capacidade);
    }
    if(lista_tac.tamanho>=lista_tac.capacidade && lista_tac.capacidade!=0){
        lista_tac.capacidade*=2;
        lista_tac.Instrucoes = realloc(lista_tac.Instrucoes, sizeof(Instrucao) * lista_tac.capacidade);
    }
    lista_tac.Instrucoes[lista_tac.tamanho]=inst;
    lista_tac.tamanho++;
}

void liberar_tac(void){
    for (int i=0;i<lista_tac.tamanho;i++){
        free(lista_tac.Instrucoes[i].operador);
        free(lista_tac.Instrucoes[i].operando1);
        free(lista_tac.Instrucoes[i].operando2);
        free(lista_tac.Instrucoes[i].resultado);
    }
    free(lista_tac.Instrucoes);
    lista_tac.Instrucoes = NULL;                                                                                                                       
    lista_tac.tamanho = 0;                                                                                                                             
    lista_tac.capacidade = 0;                                                                                                                          
    contador_temp = 0;                                                                                                                                 
    contador_rotulo = 0; 
}

void imprimir_codigo_intermediario(void){
    for(int i=0;i<lista_tac.tamanho;i++){
        Instrucao inst = lista_tac.Instrucoes[i];
        switch (inst.tipo)
        {
        case TAC_ASSIGN:
            printf("%s = %s\n", inst.resultado, inst.operando1);
            break;
        case TAC_BINOP:
            printf("%s = %s %s %s\n", inst.resultado, inst.operando1, inst.operador, inst.operando2);
            break;
        case TAC_UNOP:
            printf("%s = %s%s\n", inst.resultado, inst.operador, inst.operando1);
            break;
        case TAC_LABEL:
            printf("%s:\n", inst.resultado);
            break;
        case TAC_GOTO:
            printf("GOTO %s\n", inst.resultado);
            break;
        case TAC_IF_FALSE:
            printf("IF_FALSE %s GOTO %s\n", inst.operando1, inst.resultado);
            break;
        case TAC_PARAM:
            printf("PARAM %s\n", inst.operando1 ? inst.operando1 : inst.resultado);
            break;
        case TAC_CALL:
            if (inst.resultado != NULL) {
                printf("%s = CALL %s, %s\n", inst.resultado, inst.operando1, inst.operando2);
            } else {
                printf("CALL %s, %s\n", inst.operando1, inst.operando2);
            }
            break;
        case TAC_RETURN:
            if (inst.operando1 != NULL) {
                printf("RETURN %s\n", inst.operando1);
            } else if (inst.resultado != NULL) {
                printf("RETURN %s\n", inst.resultado);
            } else {
                printf("RETURN\n");
            }
            break;
        case TAC_PRINT:
            printf("PRINT %s\n", inst.operando1 ? inst.operando1 : inst.resultado);
            break;
        case TAC_FUNC_BEGIN:
            printf("FUNC_BEGIN %s\n", inst.resultado);
            break;
        case TAC_FUNC_END:
            printf("FUNC_END %s\n", inst.resultado);
            break;
        default:
            break;
        }
    }
}