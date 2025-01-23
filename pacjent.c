#include <unistd.h>
#include <sys/sem.h>
#include "czas.h"
#include "pacjent.h"
#include "procesy.h"
#include <time.h>
#include "czas.c"
#include "rejestracja.h"
#include <stdio.h>
#include <stdlib.h>
#include "kolejka.h"
#include <pthread.h>
#include <sys/ipc.h>

//

void pacjent_zarzadzanie(Pacjent pacjent) {

    int kolejka_zewnetrzna = msgget(KOLEJKA_ZEWNETRZNA, IPC_CREAT | 0666);
    if (kolejka_zewnetrzna == -1) {
        perror("Błąd otwierania kolejki zewnętrznej");
        exit(1);
    }

    Komunikat komunikat = {1, pacjent};
    if (msgsnd(kolejka_zewnetrzna, &komunikat, sizeof(Komunikat) - sizeof(long), 0) == -1) {
        perror("Błąd wysyłania pacjenta do kolejki zewnętrznej");
        exit(1);
    }else {
        printf("KROK 2 Pacjent ID: %d%s%s %d dołączył do kolejki zewnętrznej.\n",
               pacjent.id,
               pacjent.priorytet ? " (VIP)" : "",
               pacjent.rodzic_obecny ? " (z rodzicem)" : "",
               pacjent.wiek);
    }

    // Czekanie na wejście do rejestracji
    while (1) {

        if (czy_przychodnia_otwarta() == false) {
            printf("[DEBUG] Wszedłem do bloku: przychodnia zamknięta.\n");
            printf("KROK 3' Pacjent ID: %d nie może wejść - przychodnia zamknięta.\n", pacjent.id);
            sleep(2); // Oczekiwanie przed ponowną próbą
            continue;
        }
        // Próba wejścia do rejestracji
        int kolejka_rejestracja = msgget(KOLEJKA_REJESTRACJA, IPC_CREAT | 0666);
        if (kolejka_rejestracja == -1) {
            perror("Błąd otwierania kolejki rejestracyjnej");
            exit(1);
        }
        else {
           // printf("[DEBUG] Pacjent %d wszedl do rejestracji\n", id);
        }
    }
}



int main(int argc, char *argv[]) {
    if (argc != 6) {
        fprintf(stderr, "[PACJENT][ERROR] Nieprawidłowa liczba argumentów. Oczekiwano 5.\n");
        return 1;
    }

    // Interpretacja argumentów jako struktura Pacjent
    Pacjent pacjent;
    pacjent.id = atoi(argv[1]);
    pacjent.wiek = atoi(argv[2]);
    pacjent.priorytet = atoi(argv[3]);
    pacjent.rodzic_obecny = atoi(argv[4]);
    pacjent.lekarz = atoi(argv[5]);

    // Debug: Wyświetlenie danych pacjenta
    printf("KROK 1 [PACJENT] ID: %d, Wiek: %d, Priorytet: %d, Rodzic: %d, Lekarz: %d PID: %d\n",
           pacjent.id, pacjent.wiek, pacjent.priorytet, pacjent.rodzic_obecny, pacjent.lekarz, getpid());


    Komunikat komunikat;
    komunikat.typ = 1; // Typ wiadomości do kolejki
    komunikat.pacjent = pacjent;
// funkcja pacjent
    pacjent_zarzadzanie(pacjent);

    return 0;
}
