#include "kolejka.h"
#include <unistd.h>
#include <sys/sem.h>

void pacjent(int id) {
    int kolejka_rejestracja = msgget(KOLEJKA_REJESTRACJA, IPC_CREAT | 0666);
    if (kolejka_rejestracja == -1) {
        perror("Błąd otwierania kolejki rejestracyjnej");
        exit(1);
    }

    while (1) {
        semafor_op(-1); // Wejście do sekcji krytycznej
        if (*liczba_osob < MAX_OSOB_W_PRZYCHODNI) {
            (*liczba_osob)++; // Zwiększenie liczby osób w przychodni
            wyswietl_liczbe_osob(); // Wyświetl aktualną liczbę osób
            semafor_op(1); // Wyjście z sekcji krytycznej

            // Wysyłanie komunikatu do rejestracji
            Pacjent pacjent = {id, rand() % 50 + 10, 0, rand() % 2}; // Losowe dane pacjenta
            Komunikat komunikat = {1, pacjent};

            msgsnd(kolejka_rejestracja, &komunikat, sizeof(Pacjent), 0);
            printf("Pacjent ID: %d wszedł do przychodni i został zarejestrowany.\n", id);

            break; // Pacjent wszedł do przychodni, opuszcza pętlę
        } else {
            semafor_op(1); // Wyjście z sekcji krytycznej
            printf("Pacjent ID: %d nie może wejść do przychodni - brak miejsca. Próbuje ponownie...\n", id);
            sleep(2); // Pacjent czeka przed ponowną próbą
        }
    }

    exit(0);
}