// #include "kolejka.h"
// #include <unistd.h>
// #include <sys/sem.h>
// #include "rejestracja.h"
// #include "czas.h"
// #include "semaphore.h"
// #include "pacjent.h"
// #include "procesy.h"
// #include <time.h>
//
//
// int losuj_lekarza() {
//     //return 0; // Zawsze lekarz POZ
//     int los = rand() % 10;
//     if (los < 6) return 0;  // 60% szans na 0
//     return los - 5;         // 10% szans na 1, 2, 3, 4
// }
//
// void pacjent(int id) {
//     log_process("START", "Pacjent", id);  // Logowanie rozpoczęcia procesu pacjenta
//
//     int kolejka_zewnetrzna = msgget(KOLEJKA_ZEWNETRZNA, IPC_CREAT | 0666);
//     if (kolejka_zewnetrzna == -1) {
//         perror("Błąd otwierania kolejki zewnętrznej");
//         exit(1);
//     }
//
//     // Tworzenie danych pacjenta
//     srand(time(NULL)); // Ustawienie ziarna raz na początku programu
//
//     int wiek = rand() % 50 + 10; // Losowy wiek między 10 a 59
//     int priorytet = (rand() % 10 < 2) ? 1 : 0; // 20% szans na VIP
//     int rodzic_obecny = wiek < 18 ? 1 : 0;
//
//     Pacjent pacjent = {id, wiek, priorytet, rodzic_obecny, losuj_lekarza()};
//     Komunikat komunikat = {1, pacjent};
//
//     if (msgsnd(kolejka_zewnetrzna, &komunikat, sizeof(Pacjent), 0) == -1){
//         perror("Błąd wysyłania pacjenta do kolejki zewnętrznej");
//         exit(1);
//     }else {
//         printf("KROK 2 Pacjent ID: %d%s%s %d dołączył do kolejki zewnętrznej.\n",
//                id,
//                priorytet ? " (VIP)" : "",
//                rodzic_obecny ? " (z rodzicem)" : "",
//                wiek);
//         log_process("PRZYJSCIE", "Pacjent", id);
//     }
//     // Czekanie na wejście do rejestracji
//     while (1) {
//         Czas teraz = aktualny_czas();
//
//         if (porownaj_czas(teraz, czas_otwarcia) < 0 || porownaj_czas(teraz, czas_zamkniecia) >= 0) {
//             printf("KROK 3' Pacjent ID: %d nie może wejść - przychodnia zamknięta (czas: %02d:%02d).\n",
//                    id, teraz.godzina, teraz.minuta);
//             log_process("CZEKANIE", "Pacjent", id);
//             sleep(2); // Oczekiwanie przed ponowną próbą
//             continue;
//         }
//         // Próba wejścia do rejestracji
//         int kolejka_rejestracja = msgget(KOLEJKA_REJESTRACJA, IPC_CREAT | 0666);
//         if (kolejka_rejestracja == -1) {
//             perror("Błąd otwierania kolejki rejestracyjnej");
//             exit(1);
//         }
//         else {
//            // printf("[DEBUG] Pacjent %d wszedl do rejestracji\n", id);
//         }
//     }
// }
//
// void zmien_liczba_osob(int zmiana) {
//     zablokuj_semafor();
//     *liczba_osob += zmiana;
//     printf("[Monitorowanie] CALKOWITA Liczba osób w przychodni: %d\n", *liczba_osob);
//     log_process("ZMIANA LICZBY OSOB", "Zmien liczbe", *liczba_osob);
//     odblokuj_semafor();
// }

#include <stdio.h>
#include <stdlib.h>
#include "kolejka.h"
#include "pacjent.h"
#include <pthread.h>
#include <sys/ipc.h>

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
    printf("[PACJENT] ID: %d, Wiek: %d, Priorytet: %d, Rodzic: %d, Lekarz: %d\n",
           pacjent.id, pacjent.wiek, pacjent.priorytet, pacjent.rodzic_obecny, pacjent.lekarz);

    // Symulacja działania pacjenta (np. wejście do kolejki)
    int kolejka_zewnetrzna = msgget(KOLEJKA_ZEWNETRZNA, IPC_CREAT | 0666);
    if (kolejka_zewnetrzna == -1) {
        perror("[PACJENT][ERROR] Nie udało się otworzyć kolejki zewnętrznej");
        return 1;
    }

    Komunikat komunikat;
    komunikat.typ = 1; // Typ wiadomości do kolejki
    komunikat.pacjent = pacjent;

    // Wysłanie komunikatu do kolejki
    if (msgsnd(kolejka_zewnetrzna, &komunikat, sizeof(Pacjent), 0) == -1) {
        perror("[PACJENT][ERROR] Nie udało się wysłać komunikatu");
        return 1;
    }

    printf("[PACJENT] Pacjent ID: %d dołączył do kolejki.\n", pacjent.id);
    return 0;
}
