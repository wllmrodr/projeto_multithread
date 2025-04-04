#include "tecelao.h"
#include <ncurses.h>
#include <stdlib.h>

/* Variáveis globais (agora alocadas dinamicamente) */
int N_ROLOS, N_TECELOES, N_BOBINAS;
estado_r *estadoR;
estado_t *estadoT;
estado_bobina *estadoBobina;
int *rolosBobina;
int *rolosTear;
sem_t sem_bobinas;
sem_t *sem_tear;
sem_t *sem_tecido_pronto;
sem_t *sem_rolo_no_tear;
sem_t sem_escreve_painel, sem_le_painel;
sem_t sem_estados;
int painel;

void init_ncurses() {
    initscr();
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);
    init_pair(3, COLOR_RED, COLOR_BLACK);
    init_pair(4, COLOR_CYAN, COLOR_BLACK);
}

void atualiza_tela() {
    clear();
    printw("=== FÁBRICA DE TECELAGEM (T: %d | B: %d | R: %d) ===\n\n", 
           N_TECELOES, N_BOBINAS, N_ROLOS);
    
    /* Restante da função permanece igual */
    /* ... */
}

void cleanup() {
    free(estadoR);
    free(estadoT);
    free(estadoBobina);
    free(rolosBobina);
    free(rolosTear);
    free(sem_tear);
    free(sem_tecido_pronto);
    free(sem_rolo_no_tear);
}

int main() {
    /* Obter entradas do usuário */
    printw("Digite o número de tecelões: ");
    scanw("%d", &N_TECELOES);
    printw("Digite o número de bobinas: ");
    scanw("%d", &N_BOBINAS);
    printw("Digite o número de rolos: ");
    scanw("%d", &N_ROLOS);

    /* Alocação dinâmica */
    estadoR = (estado_r*)malloc(N_ROLOS * sizeof(estado_r));
    estadoT = (estado_t*)malloc(N_TECELOES * sizeof(estado_t));
    estadoBobina = (estado_bobina*)malloc(N_BOBINAS * sizeof(estado_bobina));
    rolosBobina = (int*)malloc(N_BOBINAS * sizeof(int));
    rolosTear = (int*)malloc(N_TECELOES * sizeof(int));
    
    sem_tear = (sem_t*)malloc(N_TECELOES * sizeof(sem_t));
    sem_tecido_pronto = (sem_t*)malloc(N_TECELOES * sizeof(sem_t));
    sem_rolo_no_tear = (sem_t*)malloc(N_TECELOES * sizeof(sem_t));

    /* Inicialização */
    init_ncurses();
    init_semaforos(N_TECELOES, N_BOBINAS);

    pthread_t *thr_rolos = (pthread_t*)malloc(N_ROLOS * sizeof(pthread_t));
    pthread_t *thr_teceloes = (pthread_t*)malloc(N_TECELOES * sizeof(pthread_t));
    ThreadArgs *args_rolos = (ThreadArgs*)malloc(N_ROLOS * sizeof(ThreadArgs));
    ThreadArgs *args_teceloes = (ThreadArgs*)malloc(N_TECELOES * sizeof(ThreadArgs));

    /* Cria threads */
    for (int i = 0; i < N_TECELOES; i++) {
        args_teceloes[i].id = i;
        args_teceloes[i].N_ROLOS = &N_ROLOS;
        args_teceloes[i].N_TECELOES = &N_TECELOES;
        args_teceloes[i].N_BOBINAS = &N_BOBINAS;
        pthread_create(&thr_teceloes[i], NULL, f_tecelao, &args_teceloes[i]);
        estadoT[i] = O;
    }

    for (int i = 0; i < N_ROLOS; i++) {
        args_rolos[i].id = i;
        args_rolos[i].N_ROLOS = &N_ROLOS;
        args_rolos[i].N_TECELOES = &N_TECELOES;
        args_rolos[i].N_BOBINAS = &N_BOBINAS;
        pthread_create(&thr_rolos[i], NULL, f_rolo, &args_rolos[i]);
        estadoR[i] = F;
    }

    for (int i = 0; i < N_BOBINAS; i++) {
        estadoBobina[i] = V;
    }

    /* Loop principal */
    while(1) {
        sem_wait(&sem_estados);
        atualiza_tela();
        sem_post(&sem_estados);
        napms(500); // 0.5s
    }

    /* Limpeza (nunca alcançado neste exemplo) */
    cleanup();
    endwin();
    return 0;
}