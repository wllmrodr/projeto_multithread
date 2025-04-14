#include "tecelao.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <semaphore.h>

/* Variáveis globais (definidas em fabrica_tecelagem.c) */
extern estado_r *estadoR;
extern estado_t *estadoT;
extern estado_bobina *estadoBobina;
extern int *rolosBobina;
extern int *rolosTear;
extern sem_t sem_bobinas;
extern sem_t *sem_tear;
extern sem_t *sem_tecido_pronto;
extern sem_t *sem_rolo_no_tear;
extern sem_t sem_escreve_painel, sem_le_painel;
extern sem_t sem_estados;
extern int painel;

/* Variável de controle definida em fabrica_tecelagem.c */
extern volatile int running;

void* f_tecelao(void *v) {
    ThreadArgs *args = (ThreadArgs*)v;
    int id = args->id;

    while (running) {
        sem_wait(&sem_escreve_painel);
        painel = id;
        sem_post(&sem_le_painel);
        
        sem_wait(&sem_rolo_no_tear[id]);
        /* Se o sinal de término foi dado, encerra a thread */
        if (!running) break;
        
        sleep(1 + rand() % 3); // Simula o tempo de tecelagem
        
        sem_post(&sem_tecido_pronto[id]);
    }
    return NULL;
}

void* f_rolo(void *v) {
    ThreadArgs *args = (ThreadArgs*)v;
    int id = args->id;
    int N_BOBINAS = *(args->N_BOBINAS);
    
    sleep(rand() % 3);

    sem_wait(&sem_estados);
    estadoR[id] = E;
    sem_post(&sem_estados);
    
    /* Aguarda até que uma bobina esteja disponível */
    sem_wait(&sem_bobinas);
    /* Verifica se a thread foi liberada para encerrar */
    if (!running) {
        sem_post(&sem_bobinas);
        return NULL;
    }
    
    sem_wait(&sem_estados);
    estadoR[id] = A;
    for (int i = 0; i < N_BOBINAS; i++) {
        if (estadoBobina[i] == V) {
            estadoBobina[i] = C;
            rolosBobina[i] = id;
            break;
        }
    }
    sem_post(&sem_estados);
    
    sem_wait(&sem_le_painel);
    int meu_tear = painel;
    sem_post(&sem_escreve_painel);
    sem_wait(&sem_tear[meu_tear]);
    
    sem_wait(&sem_estados);
    estadoT[meu_tear] = TO;
    rolosTear[meu_tear] = id;
    estadoR[id] = TT;
    sem_post(&sem_estados);
    
    sem_post(&sem_rolo_no_tear[meu_tear]);
    sem_wait(&sem_tecido_pronto[meu_tear]);
    
    sem_wait(&sem_estados);
    estadoR[id] = P;
    estadoT[meu_tear] = O;
    sem_post(&sem_estados);
    sem_post(&sem_tear[meu_tear]);
    
    /* Libera a bobina: atualiza o estado e post no semáforo */
    sem_wait(&sem_estados);
    for (int i = 0; i < N_BOBINAS; i++) {
        if (rolosBobina[i] == id) {
            estadoBobina[i] = V;
            break;
        }
    }
    sem_post(&sem_estados);
    sem_post(&sem_bobinas);
    
    return NULL;
}
