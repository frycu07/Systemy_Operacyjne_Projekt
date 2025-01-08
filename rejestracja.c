#include "kolejka.h"
#include <unistd.h>

void rejestracja() {
    int kolejka_rejestracja = msgget(KOLEJKA_REJESTRACJA, IPC_CREAT | 0666);
    int kolejka_poz = msgget(KOLEJKA_POZ, IPC_CREAT | 0666);
    int kolejka_specjalista = msgget(KOLEJKA_SPECJALISTA, IPC_CREAT | 0666);

    if (kolejka_rejestracja == -1 || kolejka_poz == -1 || kolejka_specjalista == -1) {
        perror("Błąd tworzenia kolejek");
        exit(1);
    }

    while (1) {
        Komunikat komunikat;
        if (msgrcv(kolejka_rejestracja, &komunikat, sizeof(Pacjent), 0, 0) == -1) {
            perror("Błąd odbioru komunikatu w rejestracji");
            continue;
        }

        printf("Rejestracja: Odebrano pacjenta ID: %d\n", komunikat.pacjent.id);

        // Kierowanie do odpowiedniej kolejki
        if (komunikat.pacjent.lekarz == 0) {
            komunikat.typ = 1; // Typ dla lekarza POZ
            msgsnd(kolejka_poz, &komunikat, sizeof(Pacjent), 0);
        } else {
            komunikat.typ = 2; // Typ dla specjalisty
            msgsnd(kolejka_specjalista, &komunikat, sizeof(Pacjent), 0);
        }

    }
}

void zakoncz_wizyte(int id) {
    semafor_op(-1); // Wejście do sekcji krytycznej
    (*liczba_osob)--; // Zmniejszenie liczby osób w przychodni
    wyswietl_liczbe_osob(); // Wyświetl aktualną liczbę osób
    semafor_op(1); // Wyjście z sekcji krytycznej
    printf("Pacjent ID: %d opuścił przychodnię.\n", id);
}
