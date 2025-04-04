#ifndef TECELAO_H
#define TECELAO_H

#include <pthread.h>
#include <semaphore.h>

/* Variáveis globais (agora definidas em tempo de execução) */
extern int N_ROLOS;
extern int N_TECELOES;
extern int N_BOBINAS;

/* Estados */
typedef enum {E, TT, P, F, A} estado_r;
typedef enum {TO, O} estado_t;
typedef enum {C, V} estado_bobina;

/* Estrutura para passar argumentos para as threads */
typedef struct {
    int id;
    int* N_ROLOS;
    int* N_TECELOES;
    int* N_BOBINAS;
} ThreadArgs;

/* Declarações de funções */
void init_semaforos(int N_TECELOES, int N_BOBINAS);
void* f_tecelao(void *v);
void* f_rolo(void *v);

#endif