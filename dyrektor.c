#include "dyrektor.h"
#include "procesy.h"
#include "kolejka.h"
#include "procesy.c"
#include "rejestracja.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

// Tablica PID lekarzy
pid_t pid_lekarzy[6]; // 2 lekarzy POZ + 4 specjaliści
int liczba_lekarzy = 6;

// Funkcja obsługi sygnałów dyrektora



void wyczysc_procesy_pacjentow() {
    const char *procesy[] = {"pacjent"};
    int liczba_procesow = sizeof(procesy) / sizeof(procesy[0]);

    printf("[CZYSZCZENIE][INFO] Rozpoczynam czyszczenie procesów.\n");

    for (int i = 0; i < liczba_procesow; i++) {
        printf("[CZYSZCZENIE][INFO] Szukam i kończę procesy o nazwie: %s\n", procesy[i]);
        znajdz_i_zakoncz_procesy(procesy[i]);
    }

    printf("[CZYSZCZENIE][INFO] Zakończono czyszczenie procesów.\n");
}
// Funkcja przeprowadzająca ewakuację pacjentów
void ewakuacja_pacjentow() {
    printf("[DYREKTOR] Rozpoczęcie ewakuacji pacjentów...\n");
    wyczysc_procesy_pacjentow();
    printf("[DYREKTOR] Ewakuacja zakończona.\n");
}
void zamknij_lekarza(int lek, int numer) {
    char komenda[256];
    snprintf(komenda, sizeof(komenda), "pgrep -f 'lekarz %d %d'", lek, numer);

    FILE *fp = popen(komenda, "r");
    if (!fp) {
        perror("[CZYSZCZENIE][ERROR] Nie udało się znaleźć procesu lekarza");
        return;
    }

    char linia[128];
    while (fgets(linia, sizeof(linia), fp) != NULL) {
        pid_t pid = atoi(linia);
        if (pid > 0) {
            printf("[CZYSZCZENIE][INFO] Wysyłam SIGTERM do procesu PID: %d (lekarz %d %d)\n", pid, lek, numer);
            kill(pid, SIGTERM);
        }
    }
    pclose(fp);
}

void handle_signal(int signal){
    if (signal == SIGUSR2) {ewakuacja_pacjentow();}
    else if (signal == SIGALRM) {
        int typ;
        int los_lekarz = rand() % 100;
        if (los_lekarz < 20) {
            los_lekarz = 0;
            int x = rand() % 100;
            typ = (x < 50) ? 1 : 2;
        } else if (los_lekarz < 40) {
            los_lekarz = 1;
            typ = 0;
        } else if (los_lekarz < 60) {
            los_lekarz = 2;
            typ = 0;
        } else if (los_lekarz < 80) {
            los_lekarz = 3;
            typ = 0;
        } else {
            los_lekarz = 4;
            typ = 0;
        }   zamknij_lekarza(los_lekarz, typ);
    }
    else  printf("Unknown signal");
}

int main() {
    while (1){
    signal(SIGALRM, handle_signal);
    signal(SIGUSR2, handle_signal);
        sleep(1);
}
}