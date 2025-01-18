#include "kolejka.h"
#include <unistd.h>
#include <sys/sem.h>
#include "rejestracja.h"
#include "czas.h"
#include "semaphore.h"
#include "pacjent.h"
#include "procesy.h"
#include <time.h>


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

    // Tworzenie danych pacjenta
    srand(time(NULL)); // Ustawienie ziarna raz na początku programu

    int wiek = rand() % 50 + 10; // Losowy wiek między 10 a 59
    int priorytet = (rand() % 10 < 2) ? 1 : 0; // 20% szans na VIP
    int rodzic_obecny = wiek < 18 ? 1 : 0;

    Pacjent pacjent = {id, wiek, priorytet, rodzic_obecny, losuj_lekarza()};
    Komunikat komunikat = {1, pacjent};

    msgsnd(kolejka_zewnetrzna, &komunikat, sizeof(Pacjent), 0);
    printf("KROK 2 Pacjent ID: %d%s%s %d dołączył do kolejki zewnętrznej.\n",
           id,
           priorytet ? " (VIP)" : "",
           rodzic_obecny ? " (z rodzicem)" : "",
           wiek);
    log_process("PRZYJSCIE", "Pacjent", id);

    // Czekanie na wejście do rejestracji
    while (1) {
        Czas teraz = aktualny_czas();

        if (porownaj_czas(teraz, czas_otwarcia) < 0 || porownaj_czas(teraz, czas_zamkniecia) >= 0) {
            printf("Pacjent ID: %d nie może wejść - przychodnia zamknięta (czas: %02d:%02d).\n",
                   id, teraz.godzina, teraz.minuta);
            log_process("CZEKANIE", "Pacjent", id);
            sleep(2); // Oczekiwanie przed ponowną próbą
            continue;
        }

        // Próba wejścia do rejestracji
        int kolejka_rejestracja = msgget(KOLEJKA_REJESTRACJA, IPC_CREAT | 0666);
        if (kolejka_rejestracja == -1) {
            perror("Błąd otwierania kolejki rejestracyjnej");
            exit(1);
        }
    }
}

void zmien_liczba_osob(int zmiana) {
    zablokuj_semafor();
    *liczba_osob += zmiana;
    printf("[DEBUG] Liczba osób w przychodni: %d\n", *liczba_osob);
    log_process("ZMIANA LICZBY OSOB", "Zmien liczbe", *liczba_osob);
    odblokuj_semafor();
}