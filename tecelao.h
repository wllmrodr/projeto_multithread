#ifndef TECELAO_H
#define TECELAO_H

#include <pthread.h>
#include <semaphore.h>

#define N_ROLOS 10
#define N_TECELOES 2
#define N_BOBINAS 5

/* Estados */
typedef enum {E, TT, P, F, A} estado_r;  // Rolo: Entrou, Tecendo, Pronto, Fora, Aguardando
typedef enum {TO, O} estado_t;           // Tecelão: Tecendo, Ocioso
typedef enum {C, V} estado_bobina;       // Bobina: Cheia, Vazia

/* Variáveis globais (externas) */
extern estado_r estadoR[N_ROLOS];
extern estado_t estadoT[N_TECELOES];
extern estado_bobina estadoBobina[N_BOBINAS];
extern int rolosBobina[N_BOBINAS];
extern int rolosTear[N_TECELOES];

/* Semáforos (externos) */
extern sem_t sem_bobinas;
extern sem_t sem_tear[N_TECELOES];
extern sem_t sem_tecido_pronto[N_TECELOES];
extern sem_t sem_rolo_no_tear[N_TECELOES];
extern sem_t sem_escreve_painel, sem_le_painel;
extern sem_t sem_estados;
extern int painel;

/* Protótipos das funções */
void init_semaforos();
void* f_tecelao(void *v);
void* f_rolo(void *v);

#endif