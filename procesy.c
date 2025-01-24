#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "procesy.h"
#include <stdbool.h>
#include <sys/sem.h>
#include <signal.h>

#include "kolejka.h"

// Funkcja do uzyskania aktualnego czasu w formacie tekstowym
const char* current_time_string() {
    static char buffer[64];
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    strftime(buffer, 64, "%Y-%m-%d %H:%M:%S", tm_info);
    return buffer;
}
//
// // Funkcja logująca informacje o procesach
// void log_process(const char* status, const char* type, int id) {
//     FILE* log = fopen("procesy.log", "a");
//     if (!log) {
//         perror("Błąd otwierania pliku logów");
//         return;
//     }
//     fprintf(log, "[%s] PID:%d Type:%s ID:%d Time:%s\n", status, getpid(), type, id, current_time_string());
//     fclose(log);
// }
void log_process(const char* status, const char* type, int id) {
    FILE *log_file = fopen("procesy.log", "a");
    if (log_file != NULL) {
        fprintf(log_file, "[%s] Type:%s ID:%d Time:%s Called from PID:%d\n",
                status, type, id, current_time_string(), getpid());
        fclose(log_file);
    }
}


void zakonczenie_poprzednich_procesow() {
    char command[100];
    sprintf(command, "pkill -f %s", "Projekt_SO"); // Zamienia "Projekt_SO" na nazwę programu
    system(command); // Wymusza zakończenie procesów
}

void znajdz_i_zakoncz_procesy(const char *nazwa_procesu) {
    char komenda[256];
    snprintf(komenda, sizeof(komenda), "pgrep %s", nazwa_procesu);

    FILE *fp = popen(komenda, "r");
    if (fp == NULL) {
        perror("[CZYSZCZENIE][ERROR] Nie udało się otworzyć pgrep");
        return;
    }

    char linia[128];
    while (fgets(linia, sizeof(linia), fp) != NULL) {
        pid_t pid = atoi(linia); // Odczytaj PID z wyjścia pgrep
        if (pid > 0) {
            // Wysyłanie SIGTERM do procesu
            if (kill(pid, SIGTERM) == 0) {
                printf("[CZYSZCZENIE][INFO] Wysłano SIGTERM do procesu PID: %d (%s)\n", pid, nazwa_procesu);
            } else {
                perror("[CZYSZCZENIE][ERROR] Nie udało się zakończyć procesu");
            }
        }
    }

    pclose(fp);
}

// Funkcja do czyszczenia procesów przy wyjściu
void wyczysc_procesy() {
    const char *procesy[] = {"lekarz", "pacjent", "rejestracja"};
    int liczba_procesow = sizeof(procesy) / sizeof(procesy[0]);

    printf("[CZYSZCZENIE][INFO] Rozpoczynam czyszczenie procesów.\n");

    for (int i = 0; i < liczba_procesow; i++) {
        printf("[CZYSZCZENIE][INFO] Szukam i kończę procesy o nazwie: %s\n", procesy[i]);
        znajdz_i_zakoncz_procesy(procesy[i]);
    }

    printf("[CZYSZCZENIE][INFO] Zakończono czyszczenie procesów.\n");
}