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

void init_semaforos(int N_TECELOES, int N_BOBINAS) {
    sem_init(&sem_bobinas, 0, N_BOBINAS);
    sem_init(&sem_escreve_painel, 0, 1);
    sem_init(&sem_le_painel, 0, 0);
    sem_init(&sem_estados, 0, 1);
    for (int i = 0; i < N_TECELOES; i++) {
        sem_init(&sem_tear[i], 0, 1);
        sem_init(&sem_tecido_pronto[i], 0, 0);
        sem_init(&sem_rolo_no_tear[i], 0, 0);
    }
}

void* f_tecelao(void *v) {
    ThreadArgs *args = (ThreadArgs*)v;
    int id = args->id;

    while(1) {
        sem_wait(&sem_escreve_painel);
        painel = id;
        sem_post(&sem_le_painel);

        sem_wait(&sem_rolo_no_tear[id]);

        sleep(1 + rand() % 3); // Tempo de tecelagem

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

    if (sem_trywait(&sem_bobinas) == 0) {
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
    } else {
        sem_wait(&sem_estados);
        estadoR[id] = F;
        sem_post(&sem_estados);
    }
    return NULL;
}
