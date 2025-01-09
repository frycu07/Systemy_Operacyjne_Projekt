#include "kolejka.h"
#include <unistd.h>
#include <time.h>
#include "rejestracja.h"
#include "czas.h"

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

int aktualna_godzina() {
    time_t teraz = time(NULL);
    struct tm *czas = localtime(&teraz);
    return czas->tm_hour; // Zwraca aktualną godzinę
}

void zarzadz_kolejka_zewnetrzna() {
    int kolejka_zewnetrzna = msgget(KOLEJKA_ZEWNETRZNA, IPC_CREAT | 0666);
    int kolejka_rejestracja = msgget(KOLEJKA_REJESTRACJA, IPC_CREAT | 0666);

    if (kolejka_zewnetrzna == -1 || kolejka_rejestracja == -1) {
        perror("Błąd otwierania kolejek");
        exit(1);
    }

    while (1) {
        Komunikat komunikat;

        if (msgrcv(kolejka_zewnetrzna, &komunikat, sizeof(Pacjent), 0, 0) != -1) {
            Czas teraz = aktualny_czas();

            semafor_op(-1);
            if (porownaj_czas(teraz, czas_otwarcia) >= 0 && porownaj_czas(teraz, czas_zamkniecia) < 0 &&
                *liczba_osob < MAX_OSOB_W_PRZYCHODNI) {
                (*liczba_osob)++;
                wyswietl_liczbe_osob();
                semafor_op(1);

                komunikat.typ = komunikat.pacjent.id + 1;
                msgsnd(kolejka_rejestracja, &komunikat, sizeof(Pacjent), 0);
                printf("Pacjent ID: %d został wpuszczony do przychodni z kolejki zewnętrznej.\n",
                       komunikat.pacjent.id);
                } else {
                    semafor_op(1);
                    msgsnd(kolejka_zewnetrzna, &komunikat, sizeof(Pacjent), 0);
                    sleep(1);
                }
        }
    }
}