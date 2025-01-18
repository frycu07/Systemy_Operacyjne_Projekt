#include "kolejka.h"
#include <unistd.h>
#include <time.h>
#include "rejestracja.h"

#include <signal.h>

#include "czas.h"
#include "pacjent.h"
#include "procesy.h"
#include <stdbool.h>


void rejestracja(int id) {
    log_process("START", "Rejestracja", id);  // Logowanie rozpoczęcia rejestracji

    int kolejka_rejestracja = msgget(KOLEJKA_REJESTRACJA, IPC_CREAT | 0666);
    int kolejka_poz = msgget(KOLEJKA_POZ, IPC_CREAT | 0666);
    int kolejka_kardiolog = msgget(KOLEJKA_KARDIOLOG, IPC_CREAT | 0666);
    int kolejka_okulista = msgget(KOLEJKA_OKULISTA, IPC_CREAT | 0666);
    int kolejka_pediatra = msgget(KOLEJKA_PEDIATRA, IPC_CREAT | 0666);
    int kolejka_medycyna_pracy = msgget(KOLEJKA_MEDYCYNA_PRACY, IPC_CREAT | 0666);

    if (kolejka_rejestracja == -1 || kolejka_poz == -1 || kolejka_kardiolog == -1 ||
        kolejka_okulista == -1 || kolejka_pediatra == -1 || kolejka_medycyna_pracy == -1) {
        perror("Błąd tworzenia kolejek");
        exit(1);
    }

    while (1) {
        Komunikat komunikat;

        // Odbiór pacjenta z kolejki rejestracji
        if (msgrcv(kolejka_rejestracja, &komunikat, sizeof(Pacjent), 0, IPC_NOWAIT) != -1) {
            printf("KROK 4 Rejestracja %d: Odebrano pacjenta ID: %d\n", id, komunikat.pacjent.id);

            // Logowanie odbioru pacjenta
            log_process("ODEBRANO", "Rejestracja", komunikat.pacjent.id);
            printf("KROK 5");
            // Skierowanie pacjenta do odpowiedniej kolejki
            switch (komunikat.pacjent.lekarz) {
                case 0: // POZ
                    msgsnd(kolejka_poz, &komunikat, sizeof(Pacjent), 0);
                    printf("Rejestracja %d: Pacjent ID: %d skierowany do kolejki POZ.\n", id, komunikat.pacjent.id);
                    log_process("SKIEROWANO", "POZ", komunikat.pacjent.id);
                    break;
                case 1: // Kardiolog
                    msgsnd(kolejka_kardiolog, &komunikat, sizeof(Pacjent), 0);
                    printf("Rejestracja %d: Pacjent ID: %d skierowany do kardiologa.\n", id, komunikat.pacjent.id);
                    log_process("SKIEROWANO", "Kardiolog", komunikat.pacjent.id);
                    break;
                case 2: // Okulista
                    msgsnd(kolejka_okulista, &komunikat, sizeof(Pacjent), 0);
                    printf("Rejestracja %d: Pacjent ID: %d skierowany do okulisty.\n", id, komunikat.pacjent.id);
                    log_process("SKIEROWANO", "Okulista", komunikat.pacjent.id);
                    break;
                case 3: // Pediatra
                    msgsnd(kolejka_pediatra, &komunikat, sizeof(Pacjent), 0);
                    printf("Rejestracja %d: Pacjent ID: %d skierowany do pediatry.\n", id, komunikat.pacjent.id);
                    log_process("SKIEROWANO", "Pediatra", komunikat.pacjent.id);
                    break;
                case 4: // Medycyna pracy
                    msgsnd(kolejka_medycyna_pracy, &komunikat, sizeof(Pacjent), 0);
                    printf("Rejestracja %d: Pacjent ID: %d skierowany do lekarza medycyny pracy.\n", id, komunikat.pacjent.id);
                    log_process("SKIEROWANO", "Medycyna_pracy", komunikat.pacjent.id);
                    break;
            }
        } else {
            sleep(1); // Czekanie na kolejnego pacjenta
        }
    }
}

void zakoncz_wizyte(Pacjent pacjent) {

    // Zmniejsz `liczba_osob` w zależności od obecności rodzica.
    if (pacjent.rodzic_obecny) {
        zmien_liczba_osob(-2);
        printf("Pacjent ID: %d (z rodzicem) opuścił przychodnię. Liczba osób w przychodni: %d\n",
               pacjent.id, *liczba_osob);
        log_process("WYJSCIE", "PACJENT Z RODZICEM", *liczba_osob);
    } else {
        zmien_liczba_osob(-1);
        printf("Pacjent ID: %d opuścił przychodnię. Liczba osób w przychodni: %d\n",
               pacjent.id, *liczba_osob);
        log_process("WYJSCIE", "PACJENT", *liczba_osob);
    }

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

            int wymagane_miejsce = komunikat.pacjent.rodzic_obecny ? 2 : 1;

            if (*liczba_osob + wymagane_miejsce <= MAX_OSOB_W_PRZYCHODNI) {

                // Tworzenie procesu rodzica, jeśli obecny
                if (komunikat.pacjent.rodzic_obecny) {
                    if (fork() == 0) {
                        log_process("START", "Rodzic", komunikat.pacjent.id);
                        printf("Rodzic pacjenta ID: %d czeka z dzieckiem.\n", komunikat.pacjent.id);
                        pause(); // Rodzic czeka z dzieckiem
                        log_process("END", "Rodzic", komunikat.pacjent.id);
                        exit(0);
                    }
                }

                // Wysyłanie pacjenta do kolejki rejestracyjnej
                komunikat.typ = 1;
                msgsnd(kolejka_rejestracja, &komunikat, sizeof(Pacjent), 0);
                printf("KROK 3 Pacjent ID: %d%s%s został wpuszczony do przychodni z kolejki zewnętrznej.\n",
                       komunikat.pacjent.id,
                       komunikat.pacjent.priorytet ? " (VIP)" : "",
                       komunikat.pacjent.rodzic_obecny ? " (z rodzicem)" : "");
                zmien_liczba_osob(wymagane_miejsce); // Zmiana liczby osób w przychodni

            } else {
                // Kolejka zewnętrzna czeka, jeśli brak miejsca w przychodni
                msgsnd(kolejka_zewnetrzna, &komunikat, sizeof(Pacjent), 0);
                printf("Pacjent ID: %d%s%s musi poczekać na wejście do przychodni.\n",
                       komunikat.pacjent.id,
                       komunikat.pacjent.priorytet ? " (VIP)" : "",
                       komunikat.pacjent.rodzic_obecny ? " (z rodzicem)" : "");
            }
        }

        // Krótka przerwa przed następną iteracją
        sleep(1);
    }
}

void zarzadz_okienkami(void *argumenty) {
    struct ZarzadzanieArgumenty *args = (struct ZarzadzanieArgumenty *)argumenty;
    int max_osob_w_przychodni = args->max_osob_w_przychodni;
    int kolejka_rejestracja = args->kolejka_rejestracja;

    struct msqid_ds statystyki;
    bool drugie_okienko_aktywne = false;
    pid_t drugie_okienko_pid = -1;

    while (1) {
        if (msgctl(kolejka_rejestracja, IPC_STAT, &statystyki) == -1) {
            perror("Błąd pobierania danych o kolejce rejestracji");
            exit(3);
        }

        int liczba_pacjentow = statystyki.msg_qnum;

        if (liczba_pacjentow > max_osob_w_przychodni / 2 && !drugie_okienko_aktywne) {
            drugie_okienko_pid = fork();
            if (drugie_okienko_pid == 0) {
                rejestracja(1);
                exit(0);
            }
            drugie_okienko_aktywne = true;
            printf("Otwarto drugie okienko rejestracji. PID: %d\n", drugie_okienko_pid);
        }

        if (liczba_pacjentow < max_osob_w_przychodni / 3 && drugie_okienko_aktywne) {
            kill(drugie_okienko_pid, SIGTERM);
            waitpid(drugie_okienko_pid, NULL, 0);
            drugie_okienko_aktywne = false;
            printf("Zamknięto drugie okienko rejestracji. PID: %d\n", drugie_okienko_pid);
        }

        sleep(1);
    }
}
void monitoruj_kolejke_rejestracja(int max_osob_w_przychodni, int kolejka_rejestracja) {
    struct msqid_ds statystyki;

    // Flaga określająca, czy drugie okienko jest aktywne
    bool drugie_okienko_aktywne = false;

    while (1) {

        //printf("Debug: ID kolejki rejestracja = %d\n", kolejka_rejestracja);
        if (kolejka_rejestracja == -1) {
            perror("Błąd przy tworzeniu kolejki rejestracji");
            exit(2);
        }
        // Pobierz dane o kolejce rejestracji
        if (msgctl(kolejka_rejestracja, IPC_STAT, &statystyki) == -1) {
            perror("Błąd pobierania danych o kolejce rejestracji");
            exit(3);
        }

        // Liczba pacjentów w kolejce
        int liczba_pacjentow = statystyki.msg_qnum;

        // Debug: Wyświetl aktualną liczbę pacjentów
        //printf("[Monitorowanie] Liczba pacjentów w kolejce: %d\n", liczba_pacjentow);

        // Jeżeli liczba pacjentów wynosi 0, pomiń iterację
        if (liczba_pacjentow == 0) {
            sleep(1);
            continue;
        }

        // Jeżeli liczba pacjentów przekracza MAX_OSOB_W_PRZYCHODNI / 2, otwórz drugie okienko
        if (liczba_pacjentow > max_osob_w_przychodni / 2 && !drugie_okienko_aktywne) {
            if (fork() == 0) {
                log_process("START", "Rejestracja", 1); // Uruchomienie drugiego okienka
                rejestracja(1);                            // Funkcja obsługi rejestracji
                log_process("END", "Rejestracja", 1);
                exit(0);
            }
            drugie_okienko_aktywne = true;
            printf("Otwarto drugie okienko rejestracji.\n");
        }

        // Jeżeli liczba pacjentów spadnie poniżej MAX_OSOB_W_PRZYCHODNI / 3, zamknij drugie okienko
        if (liczba_pacjentow < max_osob_w_przychodni / 3 && drugie_okienko_aktywne) {
            zakonczenie_poprzednich_procesow(); // Zamknięcie drugiego okienka
            drugie_okienko_aktywne = false;
            printf("Zamknięto drugie okienko rejestracji.\n");
        }

        sleep(1); // Odczekaj przed kolejną kontrolą
    }
}
