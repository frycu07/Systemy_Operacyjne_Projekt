#include "kolejka.h"
#include <unistd.h>
#include <sys/sem.h>
#include "rejestracja.h"
#include "czas.h"
#include "semaphore.h"
#include "pacjent.h"

void pacjent(int id) {
    int kolejka_zewnetrzna = msgget(KOLEJKA_ZEWNETRZNA, IPC_CREAT | 0666);
    if (kolejka_zewnetrzna == -1) {
        perror("Błąd otwierania kolejki zewnętrznej");
        exit(1);
    }

    // Dodanie pacjenta do kolejki zewnętrznej
    Pacjent pacjent = {id, rand() % 50 + 10, 0, rand() % 2};
    Komunikat komunikat = {1, pacjent};

    msgsnd(kolejka_zewnetrzna, &komunikat, sizeof(Pacjent), 0);
    printf("Pacjent ID: %d dołączył do kolejki zewnętrznej.\n", id);

    // Pacjent czeka na pozwolenie na wejście
    while (1) {
        Czas teraz = aktualny_czas();

        if (porownaj_czas(teraz, czas_otwarcia) < 0 || porownaj_czas(teraz, czas_zamkniecia) >= 0) {
            printf("Pacjent ID: %d nie może wejść - przychodnia zamknięta (czas: %02d:%02d).\n",
                   id, teraz.godzina, teraz.minuta);
            sleep(2); // Oczekiwanie przed ponowną próbą
            continue;
        }

        // Próba wejścia do kolejki rejestracyjnej
        int kolejka_rejestracja = msgget(KOLEJKA_REJESTRACJA, IPC_CREAT | 0666);
        if (kolejka_rejestracja == -1) {
            perror("Błąd otwierania kolejki rejestracyjnej");
            exit(1);
        }

        if (msgrcv(kolejka_rejestracja, &komunikat, sizeof(Pacjent), id + 1, IPC_NOWAIT) != -1) {
            printf("Pacjent ID: %d wszedł do przychodni i został zarejestrowany.\n", id);
            break;
        } else {
            sleep(1); // Czekanie na pozwolenie
        }
    }

    exit(0);
}
void wejdz_do_przychodni() {
    zablokuj_semafor();  // Zablokowanie semafora przed modyfikacją liczba_osob

    if (liczba_osob < MAX_OSOB_W_PRZYCHODNI) {
        liczba_osob++;
        printf("Liczba osób w przychodni: %d\n", liczba_osob);  // Logowanie
    } else {
        printf("Przychodnia pełna. Pacjent musi czekać.\n");
    }

    odblokuj_semafor();  // Odblokowanie semafora po zakończeniu operacji
}