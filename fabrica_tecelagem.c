#include "tecelao.h"
#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>

/* Variáveis globais */
int N_ROLOS, N_TECELOES, N_BOBINAS;
estado_r *estadoR;
estado_t *estadoT;
estado_bobina *estadoBobina;
int *rolosBobina;
int *rolosTear;

/* Semáforos */
sem_t sem_bobinas;
sem_t *sem_tear;
sem_t *sem_tecido_pronto;
sem_t *sem_rolo_no_tear;
sem_t sem_escreve_painel, sem_le_painel;
sem_t sem_estados;
int painel;

/* Variável de controle para finalizar as threads */
volatile int running = 1;

/* Inicialização do ncurses com verificação de erros */
void init_ncurses() {
    if (!initscr()) {
        fprintf(stderr, "Erro ao inicializar ncurses\n");
        exit(EXIT_FAILURE);
    }
    
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);  // Esconde o cursor
    
    /* Configuração de cores */
    init_pair(1, COLOR_GREEN, COLOR_BLACK);    // Tecelagem
    init_pair(2, COLOR_YELLOW, COLOR_BLACK);     // Espera / Bobina em uso
    init_pair(3, COLOR_RED, COLOR_BLACK);        // Tecelão dormindo
    init_pair(4, COLOR_CYAN, COLOR_BLACK);       // Bobina livre
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK);    // Cabeçalho e estatísticas
}

/* Atualização da tela com informações detalhadas */
void atualiza_tela() {
    clear();
    
    /* Cabeçalho e Legenda */
    attron(COLOR_PAIR(5));
    printw("=== FÁBRICA DE TECELAGEM ===\n");
    printw("Tecelões: %d | Bobinas: %d | Rolos: %d (Pressione 'q' para sair)\n", 
           N_TECELOES, N_BOBINAS, N_ROLOS);
    printw("Legenda: > = Rolo entrando | ... = Esperando | T = Tecelagem | X = Pronto | ZZZ = Tecelão dormindo\n\n");
    attroff(COLOR_PAIR(5));
    
    /* Seção de status dos rolos */
    printw("Rolos: ");
    for (int i = 0; i < N_ROLOS; i++) {
        switch(estadoR[i]) {
            case E:
                printw("R%02d> ", i);
                break;
            case A:
                attron(COLOR_PAIR(2));
                printw("R%02d... ", i);
                attroff(COLOR_PAIR(2));
                break;
            case TT:
                attron(COLOR_PAIR(1));
                printw("R%02dT ", i);
                attroff(COLOR_PAIR(1));
                break;
            case P:
                printw("R%02dX ", i);
                break;
            case F:
                printw("      "); // Rolos finalizados não são exibidos
                break;
        }
    }
    printw("\n\n");
    
    /* Status das Bobinas */
    printw("Bobinas:\n");
    for (int i = 0; i < N_BOBINAS; i++) {
        if (estadoBobina[i] == C) {
            attron(COLOR_PAIR(2));
            printw("[R%02d] ", rolosBobina[i]);
            attroff(COLOR_PAIR(2));
        } else {
            attron(COLOR_PAIR(4));
            printw("[____] ");
            attroff(COLOR_PAIR(4));
        }
    }
    printw("\n\n");
    
    /* Status dos Tecelões (Tear) */
    printw("Tear:\n");
    for (int i = 0; i < N_TECELOES; i++) {
        if (estadoT[i] == TO) {
            attron(COLOR_PAIR(1));
            printw("T%02d:R%02d ", i, rolosTear[i]);
            attroff(COLOR_PAIR(1));
        } else {
            attron(COLOR_PAIR(3));
            printw("T%02d:ZZZ ", i);
            attroff(COLOR_PAIR(3));
        }
    }
    
    /* Estatísticas */
    attron(COLOR_PAIR(5));
    printw("\n\n=== ESTATÍSTICAS ===\n");
    int tec_ativos = 0, rolos_prontos = 0;
    for (int i = 0; i < N_TECELOES; i++) {
        if (estadoT[i] == TO) tec_ativos++;
    }
    for (int i = 0; i < N_ROLOS; i++) {
        if (estadoR[i] == P) rolos_prontos++;
    }
    printw("Tecelões ativos: %d/%d | Rolos prontos: %d/%d", tec_ativos, N_TECELOES, rolos_prontos, N_ROLOS);
    attroff(COLOR_PAIR(5));
    
    refresh();
}

/* Liberação dos recursos alocados na memória */
void free_memory() {
    free(estadoR);
    free(estadoT);
    free(estadoBobina);
    free(rolosBobina);
    free(rolosTear);
    free(sem_tear);
    free(sem_tecido_pronto);
    free(sem_rolo_no_tear);
}

/* Liberação dos recursos (ncurses e semáforos) */
void cleanup() {
    endwin();
    free_memory();
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Uso: %s <num_teceloes> <num_bobinas> <num_rolos>\n", argv[0]);
        return 1;
    }
    
    N_TECELOES = atoi(argv[1]);
    N_BOBINAS = atoi(argv[2]);
    N_ROLOS = atoi(argv[3]);
    if (N_TECELOES <= 0 || N_BOBINAS <= 0 || N_ROLOS <= 0) {
        printf("Todos os valores devem ser positivos!\n");
        return 1;
    }
    
    srand(time(NULL));
    init_ncurses();
    
    /* Alocação dos estados e recursos */
    estadoR = (estado_r*)calloc(N_ROLOS, sizeof(estado_r));
    estadoT = (estado_t*)calloc(N_TECELOES, sizeof(estado_t));
    estadoBobina = (estado_bobina*)calloc(N_BOBINAS, sizeof(estado_bobina));
    rolosBobina = (int*)calloc(N_BOBINAS, sizeof(int));
    rolosTear = (int*)calloc(N_TECELOES, sizeof(int));
    
    sem_tear = (sem_t*)malloc(N_TECELOES * sizeof(sem_t));
    sem_tecido_pronto = (sem_t*)malloc(N_TECELOES * sizeof(sem_t));
    sem_rolo_no_tear = (sem_t*)malloc(N_TECELOES * sizeof(sem_t));
    
    /* Inicialização dos semáforos */
    sem_init(&sem_bobinas, 0, N_BOBINAS);
    sem_init(&sem_escreve_painel, 0, 1);
    sem_init(&sem_le_painel, 0, 0);
    sem_init(&sem_estados, 0, 1);
    for (int i = 0; i < N_TECELOES; i++) {
        sem_init(&sem_tear[i], 0, 1);
        sem_init(&sem_tecido_pronto[i], 0, 0);
        sem_init(&sem_rolo_no_tear[i], 0, 0);
    }
    
    pthread_t *thr_rolos = (pthread_t*)malloc(N_ROLOS * sizeof(pthread_t));
    pthread_t *thr_teceloes = (pthread_t*)malloc(N_TECELOES * sizeof(pthread_t));
    ThreadArgs *args_rolos = (ThreadArgs*)malloc(N_ROLOS * sizeof(ThreadArgs));
    ThreadArgs *args_teceloes = (ThreadArgs*)malloc(N_TECELOES * sizeof(ThreadArgs));
    
    for (int i = 0; i < N_TECELOES; i++) {
        args_teceloes[i].id = i;
        args_teceloes[i].N_ROLOS = &N_ROLOS;
        args_teceloes[i].N_TECELOES = &N_TECELOES;
        args_teceloes[i].N_BOBINAS = &N_BOBINAS;
        estadoT[i] = O;
    }
    
    for (int i = 0; i < N_ROLOS; i++) {
        args_rolos[i].id = i;
        args_rolos[i].N_ROLOS = &N_ROLOS;
        args_rolos[i].N_TECELOES = &N_TECELOES;
        args_rolos[i].N_BOBINAS = &N_BOBINAS;
        estadoR[i] = F;
    }
    
    for (int i = 0; i < N_BOBINAS; i++) {
        estadoBobina[i] = V;
    }
    
    /* Criação das threads dos tecelões */
    for (int i = 0; i < N_TECELOES; i++) {
        if (pthread_create(&thr_teceloes[i], NULL, f_tecelao, &args_teceloes[i])) {
            endwin();
            fprintf(stderr, "Erro ao criar thread do tecelão %d\n", i);
            cleanup();
            return 1;
        }
    }
    
    /* Criação das threads dos rolos */
    for (int i = 0; i < N_ROLOS; i++) {
        if (pthread_create(&thr_rolos[i], NULL, f_rolo, &args_rolos[i])) {
            endwin();
            fprintf(stderr, "Erro ao criar thread do rolo %d\n", i);
            cleanup();
            return 1;
        }
    }
    
    int ch;
    nodelay(stdscr, TRUE);
    while ((ch = getch()) != 'q') {
        sem_wait(&sem_estados);
        atualiza_tela();
        sem_post(&sem_estados);
        napms(100);
    }
    
    /* Sinaliza término para as threads */
    running = 0;
    /* Libera as threads bloqueadas nos semáforos dos tecelões */
    for (int i = 0; i < N_TECELOES; i++) {
        sem_post(&sem_rolo_no_tear[i]);
        sem_post(&sem_tear[i]);
    }
    /* Libera as threads de rolos que possam estar bloqueadas em sem_bobinas */
    for (int i = 0; i < N_ROLOS; i++) {
        sem_post(&sem_bobinas);
    }
    
    /* Aguarda o término das threads */
    for (int i = 0; i < N_TECELOES; i++) {
        pthread_join(thr_teceloes[i], NULL);
    }
    for (int i = 0; i < N_ROLOS; i++) {
        pthread_join(thr_rolos[i], NULL);
    }
    
    cleanup();
    
    /* Destruição dos semáforos */
    sem_destroy(&sem_bobinas);
    sem_destroy(&sem_escreve_painel);
    sem_destroy(&sem_le_painel);
    sem_destroy(&sem_estados);
    for (int i = 0; i < N_TECELOES; i++) {
        sem_destroy(&sem_tear[i]);
        sem_destroy(&sem_tecido_pronto[i]);
        sem_destroy(&sem_rolo_no_tear[i]);
    }
    
    free(thr_rolos);
    free(thr_teceloes);
    free(args_rolos);
    free(args_teceloes);
    
    printf("\nSimulação finalizada!\n");
    return 0;
}
