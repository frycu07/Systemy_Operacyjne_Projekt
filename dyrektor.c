#include "dyrektor.h"
#include "kolejka.h"
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
void obsluga_sygnalu_dyrektora(int sig) {
    printf("[DYREKTOR][SYGNAL] Otrzymano sygnał: %d\n", sig);

    if (sig == SIGUSR1) {
        printf("[DYREKTOR] Polecenie: Losowe zakończenie pracy przez lekarza.\n");
        int indeks = rand() % liczba_lekarzy; // Losowy lekarz z tablicy
        pid_t pid_lekarza = pid_lekarzy[indeks];
        if (pid_lekarza > 0) {
            printf("[DYREKTOR] Wysyłam SIGTERM do lekarza o PID: %d\n", pid_lekarza);
            kill(pid_lekarza, SIGTERM);
        } else {
            printf("[DYREKTOR] PID lekarza jest nieprawidłowy.\n");
        }
    } else if (sig == SIGUSR2) {
        printf("[DYREKTOR] Polecenie: Ewakuacja pacjentów.\n");
        ewakuacja_pacjentow();
    }
}

// Funkcja przeprowadzająca ewakuację pacjentów
void ewakuacja_pacjentow() {
    printf("[DYREKTOR] Rozpoczęcie ewakuacji pacjentów...\n");

    int kolejki[] = {KOLEJKA_REJESTRACJA, KOLEJKA_POZ, KOLEJKA_KARDIOLOG, KOLEJKA_OKULISTA, KOLEJKA_PEDIATRA, KOLEJKA_MEDYCYNA_PRACY};
    const char *nazwy_lekarzy[] = {"Rejestracja", "POZ", "Kardiolog", "Okulista", "Pediatra", "Medycyna Pracy"};

    for (int i = 0; i < 6; i++) {
        int kolejka_id = msgget(kolejki[i], IPC_CREAT | 0666);
        if (kolejka_id == -1) {
            perror("[DYREKTOR][EWAKUACJA] Nie udało się otworzyć kolejki");
            continue;
        }

        Komunikat komunikat;
        while (msgrcv(kolejka_id, &komunikat, sizeof(Pacjent), 0, IPC_NOWAIT) != -1) {
            RaportPacjenta raport;
            raport.id = komunikat.pacjent.id;
            snprintf(raport.skierowanie_do, sizeof(raport.skierowanie_do), "%s", nazwy_lekarzy[i]);
            snprintf(raport.wystawil, sizeof(raport.wystawil), "Dyrektor");

            zapisz_do_raportu(raport); // Zapisz pacjenta do raportu dziennego
            printf("[DYREKTOR][EWAKUACJA] Pacjent ID: %d został zapisany do raportu.\n", raport.id);
        }
    }

    printf("[DYREKTOR] Ewakuacja zakończona.\n");
}

// Funkcja główna dyrektora
void dyrektor() {
    printf("[DYREKTOR] Rozpoczęcie pracy. PID: %d\n", getpid());

    // Inicjalizacja generatora losowego
    srand(time(NULL));

    // Rejestracja obsługi sygnałów
    signal(SIGUSR1, obsluga_sygnalu_dyrektora);
    signal(SIGUSR2, obsluga_sygnalu_dyrektora);

    // Pętla główna dyrektora
    while (1) {
        printf("[DYREKTOR] Oczekiwanie na sygnały...\n");
        sleep(100); // Symulacja pracy dyrektora
    }
}

// Funkcja inicjalizująca PID lekarzy
void zarejestruj_pid_lekarzy(pid_t pid_poz1, pid_t pid_poz2, pid_t pid_spec[]) {
    pid_lekarzy[0] = pid_poz1;       // Lekarz POZ 1
    pid_lekarzy[1] = pid_poz2;       // Lekarz POZ 2
    for (int i = 0; i < 4; i++) {
        pid_lekarzy[2 + i] = pid_spec[i]; // Specjaliści
    }
}