#include "rejestracja.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "czas.c"
#include "pacjent.h"
#include "kolejka.h"
#include "kolejka.c"
#include "procesy.h"
#include "procesy.c"
#include "czas.h"
#include "lekarz.h"

#include <pthread.h>
#include <signal.h>
#include <sys/errno.h>
#include <sys/shm.h>

pthread_t kolejka_thread;
volatile bool czy_dziala = true;

void* uruchom_zarzadzanie_kolejka(void* arg) {
        zarzadz_kolejka_zewnetrzna();
    return NULL;
}

void zakonczenie_procesow() {
    pid_t pid;
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
        printf("Proces o PID %d został zakończony.\n", pid);
    }
}
void zakoncz_program(int sig) {
    printf("[REJESTRACJA][INFO] Otrzymano sygnał zakończenia (SIG: %d). Kończę proces.\n", sig);

    czy_dziala = false;
    znajdz_i_zakoncz_procesy("rejestracja");
    zakonczenie_procesow();
    // Poczekaj na zakończenie wątku
    pthread_join(kolejka_thread, NULL);
    //wyczysc_kolejki();
    // Zamknij zasoby
    if (shmdt(liczba_osob) == -1) {
        perror("[REJESTRACJA][ERROR] Błąd odłączania pamięci współdzielonej");
    }

    exit(0);
}
int shm_id;       // ID pamięci współdzielonej
int *liczba_osob; // Wskaźnik do pamięci współdzielonej

void uzyskaj_pamiec_wspoldzielona() {
    shm_id = shmget(PAMIEC_WSPOLDZIELONA_KLUCZ, sizeof(int), 0); // Odczyt istniejącej pamięci
    if (shm_id == -1) {
        perror("[REJESTRACJA] Błąd uzyskiwania dostępu do pamięci współdzielonej");
        exit(1);
    }

    liczba_osob = (int *)shmat(shm_id, NULL, 0);
    if (liczba_osob == (void *)-1) {
        perror("[REJESTRACJA] Błąd dołączania do pamięci współdzielonej");
        exit(1);
    }
    printf("[REJESTRACJA] Połączono z pamięcią współdzieloną. liczba_osob = %d\n", *liczba_osob);
}

void rejestracja(int id) {
    //log_process("START", "Rejestracja", id);  // Logowanie rozpoczęcia rejestracji
    printf("Uruchomiono kolejke rejestracja %d\n", id);

    int kolejka_rejestracja = msgget(KOLEJKA_REJESTRACJA, IPC_CREAT | 0666);
     int kolejka_poz = msgget(KOLEJKA_POZ, IPC_CREAT | 0666);
     int kolejka_kardiolog = msgget(KOLEJKA_KARDIOLOG, IPC_CREAT | 0666);
     int kolejka_okulista = msgget(KOLEJKA_OKULISTA, IPC_CREAT | 0666);
     int kolejka_pediatra = msgget(KOLEJKA_PEDIATRA, IPC_CREAT | 0666);
     int kolejka_medycyna_pracy = msgget(KOLEJKA_MEDYCYNA_PRACY, IPC_CREAT | 0666);
    int kolejki_lekarzy_vip[] = {KOLEJKA_VIP_POZ, KOLEJKA_VIP_KARDIOLOG, KOLEJKA_VIP_OKULISTA, KOLEJKA_VIP_PEDIATRA, KOLEJKA_VIP_MEDYCYNA_PRACY};
    int kolejki_lekarzy[] = {KOLEJKA_POZ, KOLEJKA_KARDIOLOG, KOLEJKA_OKULISTA, KOLEJKA_PEDIATRA, KOLEJKA_MEDYCYNA_PRACY};

     uzyskaj_pamiec_wspoldzielona();

     if (kolejka_rejestracja == -1 || kolejka_poz == -1 || kolejka_kardiolog == -1 ||
         kolejka_okulista == -1 || kolejka_pediatra == -1 || kolejka_medycyna_pracy == -1) {
         perror("Błąd tworzenia kolejek");
         exit(1);
         }

    int semafor_rejestracja = uzyskaj_dostep_do_semafora(klucz_semafora_rejestracja);
     int pacjenci_w_kolejce_POZ = 0;
     int pacjenci_w_kolejce_KARDIOLOG = 0;
     int pacjenci_w_kolejce_OKULISTA = 0;
     int pacjenci_w_kolejce_PEDIATRA = 0;
     int pacjenci_w_kolejce_MEDYCYNA_PRACY = 0;
    printf("[DEBUG] Proces %d używa kolejki: %d, semafora: %d\n", getpid(), kolejka_rejestracja, semafor_rejestracja);
    while (1) {
        Komunikat komunikat;
        zmniejsz_semafor(semafor_rejestracja);
        // Odbiór pacjenta z kolejki rejestracji
        if (msgrcv(kolejka_rejestracja, &komunikat, sizeof(Pacjent), 0, 0) != -1) {
            printf("KROK 4 Rejestracja %d: Odebrano pacjenta ID: %d\n", id, komunikat.pacjent.id);
        }
        else {
            perror("Błąd odbierania pacjenta z kolejki rejestracji");
            continue;
        }

        zwieksz_semafor(semafor_rejestracja);

        if (!czy_przychodnia_otwarta() && sprawdz_kolejke(kolejka_rejestracja)  == 0) {
            int lek_num = komunikat.pacjent.lekarz;
            char lek_nazw[20];
            switch (komunikat.pacjent.lekarz) {
                case 0: // POZ
                    strcpy(lek_nazw, "POZ");
                break;
                case 1: // Kardiolog
                    strcpy(lek_nazw, "KARDIOLOG");
                break;
                case 2: // Okulista
                    strcpy(lek_nazw, "OKULISTA");
                break;
                case 3: // Pediatra
                    strcpy(lek_nazw, "PEDIATRA");
                break;
                case 4: // Medycyna pracy
                    strcpy(lek_nazw, "MEDYCYNA PRACY");
                break;
                default:
                    strcpy(lek_nazw, "NIEZNANY");
                break;
            }
                RaportPacjenta raport = {komunikat.pacjent.id, lek_nazw, "REJESTRACJA"};
                zapisz_do_raportu(raport);
                zakoncz_wizyte(komunikat.pacjent);
                break;
            }
            sleep(2);
            // Logowanie odbioru pacjenta
            //log_process("ODEBRANO", "Rejestracja", komunikat.pacjent.id);
            printf("KROK 5 ");
            // Skierowanie pacjenta do odpowiedniej kolejki
            switch (komunikat.pacjent.lekarz) {
                case 0: // POZ
                {
                    if (pacjenci_w_kolejce_POZ < X1) {
                        pacjenci_w_kolejce_POZ++;
                        int kolejka_docelowa = komunikat.pacjent.priorytet ? KOLEJKA_VIP_POZ : KOLEJKA_POZ;
                        msgsnd(kolejka_docelowa, &komunikat, sizeof(Pacjent), 0);
                        printf("Rejestracja %d: Pacjent ID: %d skierowany do kolejki POZ.\n", id, komunikat.pacjent.id);
                        printf("pacjenci_w_kolejce_POZ: %d\n", pacjenci_w_kolejce_POZ);
                        //log_process("SKIEROWANO", "POZ", komunikat.pacjent.id);

                    } else {
                        printf("Rejestracja %d: Limit pacjentów POZ osiągnięty. Pacjent ID: %d nie może zostać skierowany.\n", id, komunikat.pacjent.id);
                        //log_process("ODMOWA", "POZ", komunikat.pacjent.id);
                        RaportPacjenta raport = {komunikat.pacjent.id, "POZ", "REJESTRACJA"};
                        zapisz_do_raportu(raport);
                        zakoncz_wizyte(komunikat.pacjent);
                    }
                    break;
                }
                case 1: // Kardiolog
                {
                    if (pacjenci_w_kolejce_KARDIOLOG < X2) {
                        pacjenci_w_kolejce_KARDIOLOG++;
                        int kolejka_docelowa = komunikat.pacjent.priorytet ? KOLEJKA_VIP_KARDIOLOG : KOLEJKA_KARDIOLOG;
                        printf("pacjenci_w_kolejce_KARDIOLOG: %d\n", pacjenci_w_kolejce_KARDIOLOG);
                        msgsnd(kolejka_docelowa, &komunikat, sizeof(Pacjent), 0);
                        printf("Rejestracja %d: Pacjent ID: %d skierowany do kolejki KARDIOLOG.\n", id, komunikat.pacjent.id);
                     //   log_process("SKIEROWANO", "Kardiolog", komunikat.pacjent.id);


                    } else {
                        printf("Rejestracja %d: Limit pacjentów KARDIOLOG osiągnięty. Pacjent ID: %d nie może zostać skierowany.\n", id, komunikat.pacjent.id);
                       // log_process("ODMOWA", "Kardiolog", komunikat.pacjent.id);
                        RaportPacjenta raport = {komunikat.pacjent.id, "KARDIOLOG", "REJESTRACJA"};
                        zapisz_do_raportu(raport);
                        zakoncz_wizyte(komunikat.pacjent);
                    }
                    break;
                }
                case 2: // Okulista
                {
                    if (pacjenci_w_kolejce_OKULISTA < X3) {
                        pacjenci_w_kolejce_OKULISTA++;
                        int kolejka_docelowa = komunikat.pacjent.priorytet ? KOLEJKA_VIP_OKULISTA : KOLEJKA_OKULISTA;
                        printf("pacjenci_w_kolejce_OKULISTA: %d\n", pacjenci_w_kolejce_OKULISTA);
                        msgsnd(kolejka_docelowa, &komunikat, sizeof(Pacjent), 0);
                        printf("Rejestracja %d: Pacjent ID: %d skierowany do kolejki OKULISTA.\n", id, komunikat.pacjent.id);
                       // log_process("SKIEROWANO", "Okulista", komunikat.pacjent.id);
                    } else {
                        printf("Rejestracja %d: Limit pacjentów OKULISTA osiągnięty. Pacjent ID: %d nie może zostać skierowany.\n", id, komunikat.pacjent.id);
                        //log_process("ODMOWA", "Okulista", komunikat.pacjent.id);
                        RaportPacjenta raport = {komunikat.pacjent.id, "OKULISTA", "REJESTRACJA"};
                        zapisz_do_raportu(raport);
                        zakoncz_wizyte(komunikat.pacjent);
                    }
                    break;
                }
                case 3: // Pediatra
                {
                    if (pacjenci_w_kolejce_PEDIATRA < X4) {
                        pacjenci_w_kolejce_PEDIATRA++;
                        int kolejka_docelowa = komunikat.pacjent.priorytet ? KOLEJKA_VIP_PEDIATRA : KOLEJKA_PEDIATRA;
                        printf("pacjenci_w_kolejce_PEDIATRA: %d\n", pacjenci_w_kolejce_PEDIATRA);
                        msgsnd(kolejka_docelowa, &komunikat, sizeof(Pacjent), 0);
                        printf("Rejestracja %d: Pacjent ID: %d skierowany do kolejki PEDIATRA.\n", id, komunikat.pacjent.id);
                       // log_process("SKIEROWANO", "Pediatra", komunikat.pacjent.id);
                    } else {
                        printf("Rejestracja %d: Limit pacjentów PEDIATRA osiągnięty. Pacjent ID: %d nie może zostać skierowany.\n", id, komunikat.pacjent.id);
                       // log_process("ODMOWA", "Pediatra", komunikat.pacjent.id);
                        RaportPacjenta raport = {komunikat.pacjent.id, "PEDIATRA", "REJESTRACJA"};
                        zapisz_do_raportu(raport);
                        zakoncz_wizyte(komunikat.pacjent);
                    }
                    break;
                }
                case 4: // Medycyna pracy
                {
                    if (pacjenci_w_kolejce_MEDYCYNA_PRACY < X5) {
                        pacjenci_w_kolejce_MEDYCYNA_PRACY++;
                        int kolejka_docelowa = komunikat.pacjent.priorytet ? KOLEJKA_VIP_MEDYCYNA_PRACY : KOLEJKA_MEDYCYNA_PRACY;
                        printf("pacjenci_w_kolejce_MEDYCYNA_PRACY: %d\n", pacjenci_w_kolejce_MEDYCYNA_PRACY);
                        msgsnd(kolejka_docelowa, &komunikat, sizeof(Pacjent), 0);
                        printf("Rejestracja %d: Pacjent ID: %d skierowany do kolejki MEDYCYNA PRACY.\n", id, komunikat.pacjent.id);
                       // log_process("SKIEROWANO", "Medycyna_Pracy", komunikat.pacjent.id);
                    } else {
                        printf("Rejestracja %d: Limit pacjentów MEDYCYNA PRACY osiągnięty. Pacjent ID: %d nie może zostać skierowany.\n", id, komunikat.pacjent.id);
                        //log_process("ODMOWA", "Medycyna_Pracy", komunikat.pacjent.id);
                        RaportPacjenta raport = {komunikat.pacjent.id, "MEDYCYNA PRACY", "REJESTRACJA"};
                        zapisz_do_raportu(raport);
                        zakoncz_wizyte(komunikat.pacjent);
                    }
                    break;
                }
            }
    }
}


//
// int aktualna_godzina() {
//     time_t teraz = time(NULL);
//     struct tm *czas = localtime(&teraz);
//     return czas->tm_hour; // Zwraca aktualną godzinę
// }
//
void zarzadz_kolejka_zewnetrzna() {
    Komunikat komunikat;
    int kolejka_zewnetrzna = msgget(KOLEJKA_ZEWNETRZNA, IPC_CREAT | 0666);
    int kolejka_rejestracja = msgget(KOLEJKA_REJESTRACJA, IPC_CREAT | 0666);
    printf("uruchomiono kolejke zewnetrzna\n");
    if (kolejka_zewnetrzna == -1 || kolejka_rejestracja == -1) {
        perror("Błąd otwierania kolejek");
        exit(1);
    }
    uzyskaj_dostep_do_semafora(klucz_liczba_osob);
    uzyskaj_pamiec_wspoldzielona();

    while (czy_dziala) {
        komunikat.typ = 1;


        if (msgrcv(kolejka_zewnetrzna, &komunikat, sizeof(Komunikat) - sizeof(long), 1, 0) != -1) {
            //printf("KROK 3 Odebrano wiadomość w kolejce zewnetrznej. Pacjent ID: %d\n", komunikat.pacjent.id);
        } else {
            printf("[DEBUG] msgrcv zwrócił błąd\n");
            if (errno == ENOMSG) {
                printf("[INFO] Brak wiadomości w kolejce\n");
            } else {
                perror("[ERROR] msgrcv niepowodzenie");
                printf("[DEBUG] Wartość errno: %d\n", errno);
            }
        }

     int wymagane_miejsce = komunikat.pacjent.rodzic_obecny ? 2 : 1;
     //printf("[DEBUG] liczba_osob:%d , wymagane_miejsce: %d, MAX_OSOB_W_PRZYCHODNI: %d\n",
     //*liczba_osob, wymagane_miejsce, MAX_OSOB_W_PRZYCHODNI);
     if (*liczba_osob + wymagane_miejsce <= MAX_OSOB_W_PRZYCHODNI) {
         // Tworzenie procesu rodzica, jeśli obecny
         if (komunikat.pacjent.rodzic_obecny) {
             if (fork() == 0) {
                 printf("Rodzic OK\n");
                 //log_process("START", "Rodzic", komunikat.pacjent.id);
                 printf("Rodzic pacjenta ID: %d czeka z dzieckiem.\n", komunikat.pacjent.id);
                 pause(); // Rodzic czeka z dzieckiem
                 printf("Po pauzie ok OK\n");
                 //log_process("END", "Rodzic", komunikat.pacjent.id);
                 exit(0);
             }
         }

         // Wysyłanie pacjenta do kolejki rejestracyjnej

         komunikat.typ = 1;

         if (msgsnd(kolejka_rejestracja, &komunikat, sizeof(Pacjent), 0 == -1)) {
             printf("Błąd wysyłania wiadomości\n");
         }

         printf("KROK 4 Pacjent ID: %d%s%s został wpuszczony do przychodni z kolejki zewnętrznej.\n",
                komunikat.pacjent.id,
                komunikat.pacjent.priorytet ? " (VIP)" : "",
                komunikat.pacjent.rodzic_obecny ? " (z rodzicem)" : "");
                 zmien_liczba_osob(wymagane_miejsce); // Zmiana liczby osób w przychodni

             } else {
                 // Kolejka zewnętrzna czeka, jeśli brak miejsca w przychodni
                 msgsnd(kolejka_zewnetrzna, &komunikat, sizeof(Pacjent), 0);
                 // printf("KROK 4' Pacjent ID: %d%s%s musi poczekać na wejście do przychodni.\n",
                 //        komunikat.pacjent.id,
                 //        komunikat.pacjent.priorytet ? " (VIP)" : "",
                 //        komunikat.pacjent.rodzic_obecny ? " (z rodzicem)" : "");
             }
         }

         // Krótka przerwa przed następną iteracją
         sleep(1);
        }


void zarzadz_i_monitoruj_rejestracje() {
    printf("Monitorowanie sie zaczelo\n");
    // Uruchomienie procesu rejestracji (okienko 0)
    printf("[DEBUG] Proces główny PID: %d\n", getpid());
    signal(SIGTERM, zakoncz_program);
    signal(SIGINT, zakoncz_program);

    pid_t pid = fork();
    if (pid == 0){
        printf("DZIECKO\n");
        rejestracja(0);
    } else{
        printf ("rodzic\n");
    }
        struct msqid_ds statystyki;

        // Flaga określająca, czy drugie okienko jest aktywne
        bool drugie_okienko_aktywne = false;

        // Zmienna przechowująca PID drugiego okienka
        pid_t drugie_okienko_pid = -1;
        int semafor_rejestracja = uzyskaj_dostep_do_semafora(klucz_semafora_rejestracja);
        int kolejka_rejestracja = msgget(KOLEJKA_REJESTRACJA, IPC_CREAT | 0666);

        if (kolejka_rejestracja == -1) {
            perror("[Monitorowanie] Błąd dostępu do kolejki rejestracji");
            exit(1);
        }
        while (1) {
            // Pobierz aktualne statystyki kolejki rejestracji
            if (msgctl(kolejka_rejestracja, IPC_STAT, &statystyki) == -1) {
                perror("[Monitorowanie] Błąd pobierania statystyk kolejki");
                continue; // Pomijaj iterację, jeśli nie można pobrać statystyk
            }
            // Liczba pacjentów w kolejce
            long liczba_pacjentow = statystyki.msg_qnum;

            // Debug: Wyświetl aktualną liczbę pacjentów
            printf("[Monitorowanie] Liczba pacjentów w kolejce do rejestracji: %ld\n", liczba_pacjentow);

            // Jeżeli liczba pacjentów przekracza MAX_OSOB_W_PRZYCHODNI / 2, otwórz drugie okienko
            if (liczba_pacjentow > MAX_OSOB_W_PRZYCHODNI / 2 && !drugie_okienko_aktywne) {

                drugie_okienko_pid = fork();
                if (drugie_okienko_pid == 0) {
                    // Proces dziecka uruchamia drugie okienko rejestracji
                    //log_process("START", "Rejestracja", 1);
                    rejestracja(1); // Funkcja obsługi rejestracji
                    //log_process("END", "Rejestracja", 1);
                    exit(0);
                } else if (drugie_okienko_pid > 0) {
                    // Proces rodzica ustawia flagę
                    drugie_okienko_aktywne = true;
                    printf("Otwarto drugie okienko rejestracji. PID: %d\n", drugie_okienko_pid);
                } else {
                    perror("Błąd podczas tworzenia procesu dla drugiego okienka");
                }
            }

            // Jeżeli liczba pacjentów spadnie poniżej MAX_OSOB_W_PRZYCHODNI / 3, zamknij drugie okienko
            if (liczba_pacjentow < MAX_OSOB_W_PRZYCHODNI / 3 && drugie_okienko_aktywne) {
                zmniejsz_semafor(semafor_rejestracja); // Blokowanie dostępu

                if (drugie_okienko_pid > 0) {
                    // Wysyłamy sygnał SIGTERM do procesu drugiego okienka
                    if (kill(drugie_okienko_pid, SIGTERM) == -1) {
                        perror("Błąd wysyłania sygnału SIGTERM do procesu drugiego okienka");
                    } else {

                        // Oczekiwanie na zakończenie procesu
                        if (waitpid(drugie_okienko_pid, NULL, 0) == -1) {
                            perror("Błąd oczekiwania na zakończenie procesu drugiego okienka");
                        } else {
                            printf("Drugie okienko rejestracji (PID: %d) zostało zamknięte.\n", drugie_okienko_pid);
                            drugie_okienko_aktywne = false;
                            zwieksz_semafor(semafor_rejestracja);
                        }
                    }
                } else {
                    printf("Nie znaleziono aktywnego procesu dla drugiego okienka rejestracji.\n");
                }
            }

            sleep(1); // Odczekaj przed kolejną kontrolą
        }
  }
//
void zapisz_do_raportu(RaportPacjenta pacjent) {
    FILE *plik = fopen("rapodrt_zienny.txt", "a"); // Otwórz plik w trybie dopisywania
    if (plik == NULL) {
        perror("Nie można otworzyć pliku raportu dziennego");
        exit(1);
    }

    // Zapisz informacje do pliku
    fprintf(plik, "ID Pacjenta: %d\n", pacjent.id);
    fprintf(plik, "Skierowanie do: %s\n", pacjent.skierowanie_do);
    fprintf(plik, "Wystawił: %s\n", pacjent.wystawil);
    fprintf(plik, "------------------------\n");

    fclose(plik); // Zamknij plik
}

int main() {
    printf("[REJESTRACJA][START] PROGRAM rejestracja uruchomiony. PID: %d\n", getpid());

    //Rejestracja obsługi sygnałów
    signal(SIGTERM, zakoncz_program);
    signal(SIGINT, zakoncz_program);

    int semafor_rejestracja = uzyskaj_dostep_do_semafora(klucz_semafora_rejestracja);
    int semafor_liczba_osob; uzyskaj_dostep_do_semafora(klucz_liczba_osob);

    // Utworzenie wątku dla zarzadz_kolejka_zewnetrzna
    if (pthread_create(&kolejka_thread, NULL, uruchom_zarzadzanie_kolejka, NULL) != 0) {
        perror("[REJESTRACJA][ERROR] Nie udało się utworzyć wątku dla zarzadz_kolejka_zewnetrzna");
        exit(1);
    }

    // Uruchomienie rejestracja() w głównym wątku
    zarzadz_i_monitoruj_rejestracje();

    //Oczekiwanie na zakończenie wątku zarzadz_kolejka_zewnetrzna
    //pthread_cancel(kolejka_thread);
    pthread_join(kolejka_thread, NULL);

    return 0;
}