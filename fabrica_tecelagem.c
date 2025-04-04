#include "tecelao.h"
#include <ncurses.h>

/* Ncurses: Inicializa cores e janelas */
void init_ncurses() {
    initscr();
    start_color();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    init_pair(1, COLOR_GREEN, COLOR_BLACK);  // Rolo sendo processado
    init_pair(2, COLOR_YELLOW, COLOR_BLACK); // Rolo esperando
    init_pair(3, COLOR_RED, COLOR_BLACK);    // Tecelão ocioso
    init_pair(4, COLOR_CYAN, COLOR_BLACK);   // Bobina livre
}

/* Ncurses: Atualiza a tela */
void atualiza_tela() {
    clear();
    printw("=== FÁBRICA DE TECELAGEM ===\n\n");
    
    /* Rolos */
    for (int i = 0; i < N_ROLOS; i++) {
        if (estadoR[i] == E) printw("R%02d↣ ", i);
        else if (estadoR[i] == P) printw("R%02d✓ ", i);
    }
    printw("\n\nBobinas:\n");

    /* Bobinas */
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
    printw("\n\nTear:\n");

    /* Tecelões */
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
    printw("\n");
    refresh();
}

int main() {
    pthread_t thr_rolos[N_ROLOS], thr_teceloes[N_TECELOES];
    int id_rolo[N_ROLOS], id_tecelao[N_TECELOES];

    /* Inicializa ncurses e semáforos */
    init_ncurses();
    init_semaforos();

    /* Cria threads */
    for (int i = 0; i < N_ROLOS; i++) {
        id_rolo[i] = i;
        pthread_create(&thr_rolos[i], NULL, f_rolo, &id_rolo[i]);
    }
    for (int i = 0; i < N_TECELOES; i++) {
        id_tecelao[i] = i;
        pthread_create(&thr_teceloes[i], NULL, f_tecelao, &id_tecelao[i]);
    }

    /* Loop principal (atualiza tela a cada 0.5s) */
    while (1) {
        sem_wait(&sem_estados);
        atualiza_tela();
        sem_post(&sem_estados);
        usleep(500000);  // 0.5s
    }

    endwin();
    return 0;
}