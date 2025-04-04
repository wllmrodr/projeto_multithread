#include "tecelao.h"
#include <stdlib.h>
#include <unistd.h>

/* Implementação dos semáforos */
void init_semaforos() {
    sem_init(&sem_bobinas, 0, N_BOBINAS);
    sem_init(&sem_escreve_painel, 0, 1);
    sem_init(&sem_le_painel, 0, 0);
    sem_init(&sem_estados, 0, 1);

    for (int i = 0; i < N_TECELOES; i++) {
        sem_init(&sem_tear[i], 0, 1);
        sem_init(&sem_rolo_no_tear[i], 0, 0);
        sem_init(&sem_tecido_pronto[i], 0, 0);
        estadoT[i] = O;
    }

    for (int i = 0; i < N_ROLOS; i++) estadoR[i] = F;
    for (int i = 0; i < N_BOBINAS; i++) estadoBobina[i] = V;
}

/* Thread do tecelão */
void* f_tecelao(void *v) {
    int id = *(int*)v;
    while (1) {
        sem_wait(&sem_escreve_painel);
        painel = id;
        sem_post(&sem_le_painel);
        sem_wait(&sem_rolo_no_tear[id]);
        sleep(1 + rand() % 3);  // Tempo de tecelagem
        sem_post(&sem_tecido_pronto[id]);
    }
    return NULL;
}

/* Thread do rolo */
void* f_rolo(void *v) {
    int id = *(int*)v;
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

        sem_post(&sem_rolo_no_tear[meu_tear]);
        sem_wait(&sem_tecido_pronto[meu_tear]);

        sem_wait(&sem_estados);
        estadoR[id] = P;
        estadoT[meu_tear] = O;
        sem_post(&sem_tear[meu_tear]);
    } else {
        sem_wait(&sem_estados);
        estadoR[id] = F;
        sem_post(&sem_estados);
    }
    return NULL;
}