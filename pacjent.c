#include "kolejka.h"
#include <unistd.h>
#include <sys/sem.h>
#include "rejestracja.h"
#include "czas.h"
#include "semaphore.h"
#include "pacjent.h"
#include "procesy.h"

int losuj_lekarza() {
    int los = rand() % 10;
    if (los < 6) return 0;  // 60% szans na 0
    return los - 5;         // 10% szans na 1, 2, 3, 4
}

void pacjent(int id) {
    log_process("START", "Pacjent", id);  // Logowanie rozpoczęcia procesu pacjenta

    int kolejka_zewnetrzna = msgget(KOLEJKA_ZEWNETRZNA, IPC_CREAT | 0666);
    if (kolejka_zewnetrzna == -1) {
        perror("Błąd otwierania kolejki zewnętrznej");
        exit(1);
    }

    // Dodanie pacjenta do kolejki zewnętrznej
    Pacjent pacjent = {id, rand() % 50 + 10, 0, losuj_lekarza()};
    Komunikat komunikat = {1, pacjent};

    msgsnd(kolejka_zewnetrzna, &komunikat, sizeof(Pacjent), 0);
    printf("Pacjent ID: %d dołączył do kolejki zewnętrznej.\n", id);
    log_process("PRZYJSCIE", "Pacjent", id );  // Log staniecia w kolejce
    // Pacjent czeka na pozwolenie na wejście
    while (1) {
        Czas teraz = aktualny_czas();

        if (porownaj_czas(teraz, czas_otwarcia) < 0 || porownaj_czas(teraz, czas_zamkniecia) >= 0) {
            printf("Pacjent ID: %d nie może wejść - przychodnia zamknięta (czas: %02d:%02d).\n",
                   id, teraz.godzina, teraz.minuta);
            log_process("CZEKANIE", "Pacjent", id );  // Log staniecia w kolejce
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
    log_process("END", "Pacjent", id);  // Logowanie zakończenia procesu pacjenta

    exit(0);
}
void wejdz_do_przychodni() {
    zablokuj_semafor();  // Zablokowanie semafora przed modyfikacją liczba_osob

    if (*liczba_osob < MAX_OSOB_W_PRZYCHODNI) {
        (*liczba_osob)++;
        printf("Liczba osób w przychodni: %d\n", *liczba_osob);  // Logowanie
    } else {
        printf("Przychodnia pełna. Pacjent musi czekać.\n");
    }

    odblokuj_semafor();  // Odblokowanie semafora po zakończeniu operacji
}