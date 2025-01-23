#include "kolejka.c"
#include <unistd.h>
#include "pacjent.h"
#include "rejestracja.h"
#include "lekarz.h"
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdbool.h>
#include "procesy.h"
#include <signal.h>
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include "kolejka.h"
#include "dyrektor.h"

// Klucz dla pamięci współdzielonej


// Wskaźnik do pamięci współdzielonej
int shm_id;
int *liczba_osob;
int semafor_rejestracja;
int semafor_liczba_osob;
pid_t my_pid;
bool zasoby_wyczyszczone = false;
static bool pamiec_usunieta = false;

Pacjent losuj_pacjenta(int id) {
    Pacjent pacjent; // Tworzenie zmiennej typu Pacjent
    pacjent.id = id; // Inicjalizacja ID pacjenta
    pacjent.wiek = rand() % 50 + 10;
    pacjent.priorytet = (rand() % 10 < 2) ? 1 : 0;
    pacjent.rodzic_obecny = (pacjent.wiek < 18) ? 1 : 0;
    int los = rand() % 10;
    if (los < 6) {
        pacjent.lekarz = 0;
    } else {
        pacjent.lekarz = los - 5;
    }
    return pacjent; // Funkcja zwraca strukturę Pacjent
}
void zakonczenie_procesow() {
    pid_t pid;
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
        printf("Proces o PID %d został zakończony.\n", pid);
    }
}

// Cleanup i signal handler
void cleanup_on_exit() {
    int a = 1; //DEBUG
    if (a) printf("[CLEANUP_ON_EXIT] zasoby_wyczyszczone = %d\n", zasoby_wyczyszczone);
    if (zasoby_wyczyszczone) {
        printf("[CLEANUP_ON_EXIT][DEBUG] Zasoby już zostały wyczyszczone. Pomijam.\n");
        return;
        }

    zakonczenie_procesow();
    wyczysc_kolejki();
    while (wait(NULL) > 0);
    usun_semafor(semafor_liczba_osob);
    usun_semafor(semafor_rejestracja);

    if (!pamiec_usunieta) {
        if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
            perror("[CLEANUP_ON_EXIT][ERROR] Nie udało się usunąć pamięci współdzielonej");
        } else {
            if (a)printf("[CLEANUP_ON_EXIT]ID pamięci współdzielonej: %d\n", shm_id);
            if (a)printf("[CLEANUP_ON_EXIT][DEBUG] Pamięć współdzielona została usunięta\n");
            pamiec_usunieta = true;
        }
    }
        if (a) printf("[CLEANUP_ON_EXIT] Wszystkie zasoby zostały wyczyszczone.\n");
        zasoby_wyczyszczone = true; // Oznaczenie, że zasoby zostały wyczyszczone
        exit(0);
    }


    void signal_handler(int sig) {
        int a = 0;
        if (getpid() == my_pid) {
            printf("[SIGNAL_HANDLER] Otrzymano sygnał: %d w procesie głównym (PID: %d)\n", sig, my_pid);
            cleanup_on_exit();
        } else {
            if (a)printf("[SIGNAL_HANDLER] Otrzymano sygnał: %d w procesie PID: %d (Parent PID: %d)\n", sig, getpid(), getppid());
            if (a)printf("[SIGNAL_HANDLER] Proces potomny PID: %d kończy się bez cleanupu.\n", getpid());
            _exit(0); // Proces potomny kończy się bez cleanupu
        }
    }

    // // Funkcja czyszcząca zakończone procesy
    void* process_cleaner(void* arg) {
        int status;
        pid_t pid;

        while (1) {
            pid = waitpid(-1, &status, WNOHANG); // Non-blocking wait
            if (pid > 0) {
                printf("Proces o PID: %d zakończył się.\n", pid);
            } else if (pid == 0) {
                sleep(1); // No terminated processes, wait a bit
            } else if (pid == -1 && errno == ECHILD) {
                // No child processes left
                break;
            } else {
                perror("Błąd w waitpid");
            }
        }
        return NULL;
    }

    void pamiec_wspoldzielona() {
        int a = 1;
        // Tworzenie segmentu pamięci współdzielonej
        shm_id = shmget(PAMIEC_WSPOLDZIELONA_KLUCZ, sizeof(int), IPC_CREAT | 0666);
        if(a) printf("[PAMIEC_WSPOLDZIELONA][DEBUG] shm_id = %d\n", shm_id);
        if (shm_id == -1) {
            perror("Błąd tworzenia pamięci współdzielonej");
            exit(1);
        }

        // Dołączanie segmentu pamięci współdzielonej
        liczba_osob = (int *)shmat(shm_id, NULL, 0);
        if (a) printf("[PAMIEC_WSPOLDZIELONA][DEBUG] liczba_osob = %d\n", *liczba_osob);
        if (liczba_osob == (void *)-1) {
            perror("Błąd dołączania pamięci współdzielonej");
            exit(1);
        }

        // Inicjalizacja wartości w pamięci współdzielonej
        *liczba_osob = 0;
        if(a) printf("[DEBUG] Pamięć współdzielona utworzona i zainicjalizowana. Wartość początkowa: %d\n", *liczba_osob);
    }


    int main() {
        int a = 1;
        my_pid = getpid(); // Ustaw PID procesu głównego
        printf("[MAIN] Proces główny uruchomiony z PID: %d\n", my_pid);
        //  int a=0;
        pthread_t cleaner_thread, monitor_thread;
        // zakonczenie_poprzednich_procesow();
        signal(SIGINT, signal_handler);
        signal(SIGTERM, signal_handler);

        srand(time(NULL));  // Inicjalizacja generatora liczb losowych

        // Utworzenie segmentu pamięci współdzielonej
        pamiec_wspoldzielona();

        semafor_liczba_osob = stworz_semafor(klucz_liczba_osob);
        zwieksz_semafor(semafor_liczba_osob);
        printf("WARTOSC SEMAFORA LICZBA OSOB: %d\n", pobierz_wartosc_semafora(semafor_liczba_osob));
        // Inicjalizacja semafora rejestracji na wartość 1 (odblokowany)

        semafor_rejestracja = stworz_semafor(klucz_semafora_rejestracja);

         zwieksz_semafor(semafor_rejestracja); // Odblokowanie semafora
         if (a) printf("[MAIN][DEBUG] Semafor rejestracji został utworzony (wartosc = %d)\n", semafor_rejestracja);


    pid_t pid_rejestracja = fork();
    if (pid_rejestracja == 0) {
        // Proces potomny - rejestracja
        if (execl("./rejestracja", "rejestracja", NULL) == -1) {
            perror("[MAIN][ERROR] Nie udało się uruchomić procesu rejestracja");
            exit(1);
        }
    } else if (pid_rejestracja > 0) {
        // Proces rodzica - nie czeka na zakończenie rejestracji
        printf("[MAIN][DEBUG] Utworzono proces rejestracji z PID: %d\n", pid_rejestracja);
    } else {
        perror("[MAIN][ERROR] Fork dla rejestracji nie powiódł się");
        exit(1);
    }

    /// Tworzenie procesów pacjentów
    for (int i = 0; i < 2; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            // Proces potomny - pacjent
            Pacjent pacjent = losuj_pacjenta(i);

            // Konwersja pól struktury na ciągi znaków
            char id_str[10], wiek_str[10], priorytet_str[10], rodzic_str[10], lekarz_str[10];
            sprintf(id_str, "%d", pacjent.id);
            sprintf(wiek_str, "%d", pacjent.wiek);
            sprintf(priorytet_str, "%d", pacjent.priorytet);
            sprintf(rodzic_str, "%d", pacjent.rodzic_obecny);
            sprintf(lekarz_str, "%d", pacjent.lekarz);

            // Uruchomienie procesu pacjent
            if (execl("./pacjent", "pacjent", id_str, wiek_str, priorytet_str, rodzic_str, lekarz_str, NULL) == -1) {
                perror("[MAIN][ERROR] Nie udało się uruchomić procesu pacjent");
                exit(1);
            }
        } else if (pid < 0) {
            perror("[MAIN][ERROR] Fork dla pacjenta nie powiódł się");
            exit(1);
        }
        // Proces rodzica - kontynuuje tworzenie kolejnych pacjentów
        printf("[MAIN][DEBUG] Utworzono proces pacjenta z PID: %d\n", pid);
        //sleep(1); // Symulacja przybywania pacjentów
    }

    // Tworzenie procesu rejestracji

        //
        //     // Tworzenie kolejki rejestracji
        //     int kolejka_rejestracja = msgget(KOLEJKA_REJESTRACJA, IPC_CREAT | 0666);
        //     if (kolejka_rejestracja == -1) {
        //         perror("Błąd tworzenia kolejki rejestracji");
        //         exit(1);
        //     }
        //
        //     ArgumentyRejestracja args = {kolejka_rejestracja, semafor_rejestracja, MAX_OSOB_W_PRZYCHODNI};
        //     if (pthread_create(&monitor_thread, NULL, (void *)zarzadz_i_monitoruj_rejestracje, (void *)&args) != 0) {
        //         perror("Błąd tworzenia wątku monitorującego");
        //         exit(1);
        //     }
        //
        //     int b = 1; // TWORZENIE LEKARZY
        //     pid_t pid_poz1, pid_poz2, pid_spec[4];
        //     if ((pid_poz1 = fork() == 0)) {
        //         lekarz_poz(1, X1);
        //         if (b)printf("[MAIN][DEBUG] Uruchomiono proces: PID = %d, typ = LEKARZ POZ1\n", getpid());
        //         log_process("END", "Lekarz_POZ", 1);
        //         exit(0);
        //     }
        //
        //     if ((pid_poz2 = fork() == 0)) {
        //         sleep(1);
        //         lekarz_poz(2, X1);
        //         if (b)printf("[MAIN][DEBUG] Uruchomiono proces: PID = %d, typ = LEKARZ POZ2\n", getpid());
        //         log_process("END", "Lekarz_POZ", 2);
        //         exit(0);
        //     }
        //
        //     int kolejki_specjalistow[] = {KOLEJKA_KARDIOLOG, KOLEJKA_OKULISTA, KOLEJKA_PEDIATRA, KOLEJKA_MEDYCYNA_PRACY};
        //     int limity_specjalistow[] = {X2, X3, X4, X5};
        //
        //     for (int i = 0; i < 4; i++) {
        //         if ((pid_spec[i] = fork()) == 0) {
        //             sleep(1);
        //             lekarz_specjalista(kolejki_specjalistow[i], limity_specjalistow[i]);
        //             if (b)printf("[DEBUG] Uruchomiono proces: PID = %d, typ = %x \n", getpid(), kolejki_specjalistow[i]);
        //             log_process("END", "Lekarz_Specjalista", kolejki_specjalistow[i]);
        //             exit(0);
        //         }
        //     }
        //     // Inicjalizacja procesu dyrektora
        //     // if (fork() == 0) {
        //     //     dyrektor();
        //     //     exit(0);
        //     // }
        //     // zarejestruj_pid_lekarzy(pid_poz1, pid_poz2, pid_spec);
        //     // Tworzenie wątku do oczyszczania zakończonych procesów

        if (pthread_create(&cleaner_thread, NULL, process_cleaner, NULL) != 0) {
            perror("Błąd tworzenia wątku czyszczącego");
            exit(1);
        }


        // Oczekiwanie na zakończenie wątków
        pthread_join(monitor_thread, NULL);
        pthread_join(cleaner_thread, NULL);

        // Oczekiwanie na zakończenie procesów
        while (wait(NULL) > 0);
        cleanup_on_exit();
        return 0;
    }
