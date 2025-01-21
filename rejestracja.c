
// #include <unistd.h>
// #include <time.h>
// #include "rejestracja.h"
// #include <signal.h>
// #include "czas.h"
// #include "pacjent.h"
// #include "procesy.h"
// #include <stdbool.h>
// #include <sys/sem.h>
// #include "lekarz.h"
// #include <sys/msg.h>
// #include <sys/ipc.h>
// #include "kolejka.h"


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int semafor_rejestracja;
void zakoncz_program(int sig) {
    printf("[REJESTRACJA][INFO] Otrzymano sygnał zakończenia (SIG: %d). Kończę proces.\n", sig);
    exit(0);
}

// void rejestracja(int id, int semafor_rejestracja) {
//     log_process("START", "Rejestracja", id);  // Logowanie rozpoczęcia rejestracji
//
//     int kolejka_rejestracja = msgget(KOLEJKA_REJESTRACJA, IPC_CREAT | 0666);
//     int kolejka_poz = msgget(KOLEJKA_POZ, IPC_CREAT | 0666);
//     int kolejka_kardiolog = msgget(KOLEJKA_KARDIOLOG, IPC_CREAT | 0666);
//     int kolejka_okulista = msgget(KOLEJKA_OKULISTA, IPC_CREAT | 0666);
//     int kolejka_pediatra = msgget(KOLEJKA_PEDIATRA, IPC_CREAT | 0666);
//     int kolejka_medycyna_pracy = msgget(KOLEJKA_MEDYCYNA_PRACY, IPC_CREAT | 0666);
//
//     if (kolejka_rejestracja == -1 || kolejka_poz == -1 || kolejka_kardiolog == -1 ||
//         kolejka_okulista == -1 || kolejka_pediatra == -1 || kolejka_medycyna_pracy == -1) {
//         perror("Błąd tworzenia kolejek");
//         exit(1);
//     }
//
//     int pacjenci_w_kolejce_POZ = 0;
//     int pacjenci_w_kolejce_KARDIOLOG = 0;
//     int pacjenci_w_kolejce_OKULISTA = 0;
//     int pacjenci_w_kolejce_PEDIATRA = 0;
//     int pacjenci_w_kolejce_MEDYCYNA_PRACY = 0;
//
//     while (1) {
//         Komunikat komunikat;
//
//         // Odbiór pacjenta z kolejki rejestracji
//         if (msgrcv(kolejka_rejestracja, &komunikat, sizeof(Pacjent), 0, 0) != -1) {
//             printf("KROK 4 Rejestracja %d: Odebrano pacjenta ID: %d\n", id, komunikat.pacjent.id);
//             zmniejsz_semafor(semafor_rejestracja);
//             if (!czy_przychodnia_otwarta() && sprawdz_kolejke(kolejka_rejestracja) == 0) {
//                 RaportPacjenta raport = {komunikat.pacjent.id, "KARDIOLOG", "REJESTRACJA"};
//                 zapisz_do_raportu(raport);
//                 zakoncz_wizyte(komunikat.pacjent);
//                 break;
//             }
//             sleep(2);
//             // Logowanie odbioru pacjenta
//             log_process("ODEBRANO", "Rejestracja", komunikat.pacjent.id);
//             printf("KROK 5 ");
//             // Skierowanie pacjenta do odpowiedniej kolejki
//             switch (komunikat.pacjent.lekarz) {
//                 case 0: // POZ
//                 {
//                     if (pacjenci_w_kolejce_POZ < X1) {
//                         pacjenci_w_kolejce_POZ++;
//                         msgsnd(kolejka_poz, &komunikat, sizeof(Pacjent), 0);
//                         printf("Rejestracja %d: Pacjent ID: %d skierowany do kolejki POZ.\n", id, komunikat.pacjent.id);
//                         printf("pacjenci_w_kolejce_POZ: %d\n", pacjenci_w_kolejce_POZ);
//                         log_process("SKIEROWANO", "POZ", komunikat.pacjent.id);
//
//                     } else {
//                         printf("Rejestracja %d: Limit pacjentów POZ osiągnięty. Pacjent ID: %d nie może zostać skierowany.\n", id, komunikat.pacjent.id);
//                         log_process("ODMOWA", "POZ", komunikat.pacjent.id);
//                         RaportPacjenta raport = {komunikat.pacjent.id, "POZ", "REJESTRACJA"};
//                         zapisz_do_raportu(raport);
//                         zakoncz_wizyte(komunikat.pacjent);
//                     }
//                     break;
//                 }
//                 case 1: // Kardiolog
//                 {
//                     if (pacjenci_w_kolejce_KARDIOLOG < X2) {
//                         pacjenci_w_kolejce_KARDIOLOG++;
//                         printf("pacjenci_w_kolejce_KARDIOLOG: %d\n", pacjenci_w_kolejce_KARDIOLOG);
//                         msgsnd(kolejka_kardiolog, &komunikat, sizeof(Pacjent), 0);
//                         printf("Rejestracja %d: Pacjent ID: %d skierowany do kolejki KARDIOLOG.\n", id, komunikat.pacjent.id);
//                         log_process("SKIEROWANO", "Kardiolog", komunikat.pacjent.id);
//
//
//                     } else {
//                         printf("Rejestracja %d: Limit pacjentów KARDIOLOG osiągnięty. Pacjent ID: %d nie może zostać skierowany.\n", id, komunikat.pacjent.id);
//                         log_process("ODMOWA", "Kardiolog", komunikat.pacjent.id);
//                         RaportPacjenta raport = {komunikat.pacjent.id, "KARDIOLOG", "REJESTRACJA"};
//                         zapisz_do_raportu(raport);
//                         zakoncz_wizyte(komunikat.pacjent);
//                     }
//                     break;
//                 }
//                 case 2: // Okulista
//                 {
//                     if (pacjenci_w_kolejce_OKULISTA < X3) {
//                         pacjenci_w_kolejce_OKULISTA++;
//                         printf("pacjenci_w_kolejce_OKULISTA: %d\n", pacjenci_w_kolejce_OKULISTA);
//                         msgsnd(kolejka_okulista, &komunikat, sizeof(Pacjent), 0);
//                         printf("Rejestracja %d: Pacjent ID: %d skierowany do kolejki OKULISTA.\n", id, komunikat.pacjent.id);
//                         log_process("SKIEROWANO", "Okulista", komunikat.pacjent.id);
//                     } else {
//                         printf("Rejestracja %d: Limit pacjentów OKULISTA osiągnięty. Pacjent ID: %d nie może zostać skierowany.\n", id, komunikat.pacjent.id);
//                         log_process("ODMOWA", "Okulista", komunikat.pacjent.id);
//                         RaportPacjenta raport = {komunikat.pacjent.id, "OKULISTA", "REJESTRACJA"};
//                         zapisz_do_raportu(raport);
//                         zakoncz_wizyte(komunikat.pacjent);
//                     }
//                     break;
//                 }
//                 case 3: // Pediatra
//                 {
//                     if (pacjenci_w_kolejce_PEDIATRA < X4) {
//                         pacjenci_w_kolejce_PEDIATRA++;
//                         printf("pacjenci_w_kolejce_PEDIATRA: %d\n", pacjenci_w_kolejce_PEDIATRA);
//                         msgsnd(kolejka_pediatra, &komunikat, sizeof(Pacjent), 0);
//                         printf("Rejestracja %d: Pacjent ID: %d skierowany do kolejki PEDIATRA.\n", id, komunikat.pacjent.id);
//                         log_process("SKIEROWANO", "Pediatra", komunikat.pacjent.id);
//                     } else {
//                         printf("Rejestracja %d: Limit pacjentów PEDIATRA osiągnięty. Pacjent ID: %d nie może zostać skierowany.\n", id, komunikat.pacjent.id);
//                         log_process("ODMOWA", "Pediatra", komunikat.pacjent.id);
//                         RaportPacjenta raport = {komunikat.pacjent.id, "PEDIATRA", "REJESTRACJA"};
//                         zapisz_do_raportu(raport);
//                         zakoncz_wizyte(komunikat.pacjent);
//                     }
//                     break;
//                 }
//                 case 4: // Medycyna pracy
//                 {
//                     if (pacjenci_w_kolejce_MEDYCYNA_PRACY < X5) {
//                         pacjenci_w_kolejce_MEDYCYNA_PRACY++;
//                         printf("pacjenci_w_kolejce_MEDYCYNA_PRACY: %d\n", pacjenci_w_kolejce_MEDYCYNA_PRACY);
//                         msgsnd(kolejka_medycyna_pracy, &komunikat, sizeof(Pacjent), 0);
//                         printf("Rejestracja %d: Pacjent ID: %d skierowany do kolejki MEDYCYNA PRACY.\n", id, komunikat.pacjent.id);
//                         log_process("SKIEROWANO", "Medycyna_Pracy", komunikat.pacjent.id);
//                     } else {
//                         printf("Rejestracja %d: Limit pacjentów MEDYCYNA PRACY osiągnięty. Pacjent ID: %d nie może zostać skierowany.\n", id, komunikat.pacjent.id);
//                         log_process("ODMOWA", "Medycyna_Pracy", komunikat.pacjent.id);
//                         RaportPacjenta raport = {komunikat.pacjent.id, "MEDYCYNA PRACY", "REJESTRACJA"};
//                         zapisz_do_raportu(raport);
//                         zakoncz_wizyte(komunikat.pacjent);
//                     }
//                     break;
//                 }
//             }
//         zwieksz_semafor(semafor_rejestracja);
//         }
//                     else {
//             sleep(1); // Czekanie na kolejnego pacjenta
//         }
//     }
// }
//
// void zakoncz_wizyte(Pacjent pacjent) {
//
//     // Zmniejsz `liczba_osob` w zależności od obecności rodzica.
//     if (pacjent.rodzic_obecny) {
//         zmien_liczba_osob(-2);
//         printf("KROK 8 Pacjent ID: %d (z rodzicem) opuścił przychodnię. Liczba osób w przychodni: %d\n",
//                pacjent.id, *liczba_osob);
//         log_process("WYJSCIE", "PACJENT Z RODZICEM", *liczba_osob);
//     } else {
//         zmien_liczba_osob(-1);
//         printf("KROK 8 Pacjent ID: %d opuścił przychodnię. Liczba osób w przychodni: %d\n",
//                pacjent.id, *liczba_osob);
//         log_process("WYJSCIE", "PACJENT", *liczba_osob);
//     }
//
// }
//
// int aktualna_godzina() {
//     time_t teraz = time(NULL);
//     struct tm *czas = localtime(&teraz);
//     return czas->tm_hour; // Zwraca aktualną godzinę
// }
//
// void zarzadz_kolejka_zewnetrzna() {
//     int kolejka_zewnetrzna = msgget(KOLEJKA_ZEWNETRZNA, IPC_CREAT | 0666);
//     int kolejka_rejestracja = msgget(KOLEJKA_REJESTRACJA, IPC_CREAT | 0666);
//
//     if (kolejka_zewnetrzna == -1 || kolejka_rejestracja == -1) {
//         perror("Błąd otwierania kolejek");
//         exit(1);
//     }
//
//     while (1) {
//
//         Komunikat komunikat;
//
//         if (msgrcv(kolejka_zewnetrzna, &komunikat, sizeof(Pacjent), 0, IPC_NOWAIT) != -1) {
//             printf("KROK 3 Odebrano pacjenta ID: %d z kolejki KOLEJKA_ZEWNETRZNA\n", komunikat.pacjent.id);
//             int wymagane_miejsce = komunikat.pacjent.rodzic_obecny ? 2 : 1;
//             if (*liczba_osob + wymagane_miejsce <= MAX_OSOB_W_PRZYCHODNI) {
//                 // Tworzenie procesu rodzica, jeśli obecny
//                 if (komunikat.pacjent.rodzic_obecny) {
//                     if (fork() == 0) {
//                         printf("Rodzic OK\n");
//                         log_process("START", "Rodzic", komunikat.pacjent.id);
//                         printf("Rodzic pacjenta ID: %d czeka z dzieckiem.\n", komunikat.pacjent.id);
//                         pause(); // Rodzic czeka z dzieckiem
//                         printf("Po pauzie ok OK\n");
//                         log_process("END", "Rodzic", komunikat.pacjent.id);
//                         exit(0);
//                     }
//                 }
//                 // Wysyłanie pacjenta do kolejki rejestracyjnej
//                 komunikat.typ = 1;
//                 msgsnd(kolejka_rejestracja, &komunikat, sizeof(Pacjent), 0);
//                 printf("KROK 4 Pacjent ID: %d%s%s został wpuszczony do przychodni z kolejki zewnętrznej.\n",
//                        komunikat.pacjent.id,
//                        komunikat.pacjent.priorytet ? " (VIP)" : "",
//                        komunikat.pacjent.rodzic_obecny ? " (z rodzicem)" : "");
//
//                 zmien_liczba_osob(wymagane_miejsce); // Zmiana liczby osób w przychodni
//
//             } else {
//                 // Kolejka zewnętrzna czeka, jeśli brak miejsca w przychodni
//                 msgsnd(kolejka_zewnetrzna, &komunikat, sizeof(Pacjent), 0);
//                 printf("KROK 4' Pacjent ID: %d%s%s musi poczekać na wejście do przychodni.\n",
//                        komunikat.pacjent.id,
//                        komunikat.pacjent.priorytet ? " (VIP)" : "",
//                        komunikat.pacjent.rodzic_obecny ? " (z rodzicem)" : "");
//             }
//         }
//
//         // Krótka przerwa przed następną iteracją
//         sleep(1);
//     }
// }
//
// void zarzadz_i_monitoruj_rejestracje(void *argumenty) {
//     ArgumentyRejestracja *args = (ArgumentyRejestracja *)argumenty;
//     int kolejka_rejestracja = args->kolejka_rejestracja;
//     int semafor_rejestracja = args->semafor_rejestracja;
//     int max_osob_w_przychodni = args->max_osob_w_przychodni; {
//         struct msqid_ds statystyki;
//
//         // Flaga określająca, czy drugie okienko jest aktywne
//         bool drugie_okienko_aktywne = false;
//
//         // Zmienna przechowująca PID drugiego okienka
//         pid_t drugie_okienko_pid = -1;
//
//         while (1) {
//             if (kolejka_rejestracja == -1) {
//                 perror("Błąd przy tworzeniu kolejki rejestracji");
//                 exit(2);
//             }
//
//             // Pobierz dane o kolejce rejestracji
//             if (msgctl(kolejka_rejestracja, IPC_STAT, &statystyki) == -1) {
//                 perror("Błąd pobierania danych o kolejce rejestracji");
//                 exit(3);
//             }
//
//
//             // Liczba pacjentów w kolejce
//             long liczba_pacjentow = statystyki.msg_qnum;
//
//             // Debug: Wyświetl aktualną liczbę pacjentów
//             printf("[Monitorowanie] Liczba pacjentów w kolejce do rejestracji: %ld\n", liczba_pacjentow);
//
//             // Jeżeli liczba pacjentów przekracza MAX_OSOB_W_PRZYCHODNI / 2, otwórz drugie okienko
//             if (liczba_pacjentow > max_osob_w_przychodni / 2 && !drugie_okienko_aktywne) {
//
//                 drugie_okienko_pid = fork();
//                 if (drugie_okienko_pid == 0) {
//                     // Proces dziecka uruchamia drugie okienko rejestracji
//                     log_process("START", "Rejestracja", 1);
//                     zwieksz_semafor(semafor_rejestracja); // Odblokowanie dostępu
//                     rejestracja(1, semafor_rejestracja); // Funkcja obsługi rejestracji
//                     log_process("END", "Rejestracja", 1);
//                     exit(0);
//                 } else if (drugie_okienko_pid > 0) {
//                     // Proces rodzica ustawia flagę
//                     drugie_okienko_aktywne = true;
//                     printf("Otwarto drugie okienko rejestracji. PID: %d\n", drugie_okienko_pid);
//                 } else {
//                     perror("Błąd podczas tworzenia procesu dla drugiego okienka");
//                 }
//             }
//
//             // Jeżeli liczba pacjentów spadnie poniżej MAX_OSOB_W_PRZYCHODNI / 3, zamknij drugie okienko
//             if (liczba_pacjentow < max_osob_w_przychodni / 3 && drugie_okienko_aktywne) {
//                 zmniejsz_semafor(semafor_rejestracja); // Blokowanie dostępu
//
//                 if (drugie_okienko_pid > 0) {
//                     // Wysyłamy sygnał SIGTERM do procesu drugiego okienka
//                     if (kill(drugie_okienko_pid, SIGTERM) == -1) {
//                         perror("Błąd wysyłania sygnału SIGTERM do procesu drugiego okienka");
//                     } else {
//                         // Oczekiwanie na zakończenie procesu
//                         if (waitpid(drugie_okienko_pid, NULL, 0) == -1) {
//                             perror("Błąd oczekiwania na zakończenie procesu drugiego okienka");
//                         } else {
//                             printf("Drugie okienko rejestracji (PID: %d) zostało zamknięte.\n", drugie_okienko_pid);
//                             drugie_okienko_aktywne = false;
//                         }
//                     }
//                 } else {
//                     printf("Nie znaleziono aktywnego procesu dla drugiego okienka rejestracji.\n");
//                 }
//             }
//
//             sleep(1); // Odczekaj przed kolejną kontrolą
//         }
//     }
// }
//
// void zapisz_do_raportu(RaportPacjenta pacjent) {
//     FILE *plik = fopen("raport_dzienny.txt", "a"); // Otwórz plik w trybie dopisywania
//     if (plik == NULL) {
//         perror("Nie można otworzyć pliku raportu dziennego");
//         exit(1);
//     }
//
//     // Zapisz informacje do pliku
//     fprintf(plik, "ID Pacjenta: %d\n", pacjent.id);
//     fprintf(plik, "Skierowanie do: %s\n", pacjent.skierowanie_do);
//     fprintf(plik, "Wystawił: %s\n", pacjent.wystawil);
//     fprintf(plik, "------------------------\n");
//
//     fclose(plik); // Zamknij plik
// }

int main() {
    printf("[REJESTRACJA][START] Proces rejestracja uruchomiony. PID: %d\n", getpid());

    // // Rejestracja obsługi sygnałów
    // signal(SIGTERM, zakoncz_program);
    // signal(SIGINT, zakoncz_program);
    //
    // // Pobranie dostępu do istniejącego semafora
    // key_t klucz_semafora_rejestracja = 1233;
    // semafor_rejestracja = semget(klucz_semafora_rejestracja, 1, 0);
    // if (semafor_rejestracja == -1) {
    //     perror("[REJESTRACJA][ERROR] Nie udało się uzyskać dostępu do semafora rejestracji");
    //     exit(1);
    // }
    //
    // int kolejka_rejestracja = msgget(KOLEJKA_REJESTRACJA, IPC_CREAT | 0666);
    // if (kolejka_rejestracja == -1) {
    //     perror("[REJESTRACJA][ERROR] Nie udało się otworzyć kolejki rejestracji");
    //     exit(1);
    // }

    // Wywołanie funkcji rejestracja
    //rejestracja(0, semafor_rejestracja);

    return 0;
}