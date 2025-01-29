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
#include "procesy.c"

// Klucz dla pamięci współdzielonej


// Wskaźnik do pamięci współdzielonej
int shm_id;
int shm_id2;
int *liczba_osob;
int semafor_rejestracja;
int semafor_liczba_osob;
int semafor_suma_kolejek;
int *suma_kolejek;
pid_t my_pid;
bool zasoby_wyczyszczone = false;
static bool pamiec_usunieta = false;
static bool pamiec_usunieta2 = false;
Pacjent losuj_pacjenta(int id) {
    Pacjent pacjent;

    // Ustawienie dynamicznego ziarna dla rand() w oparciu o czas i ID pacjenta
    srand((unsigned int)(time(NULL) + id * 1000 + rand()));

    pacjent.id = id;

    pacjent.wiek = rand() % 100 + 1;


    // Losowanie priorytetu VIP w zależności od wieku
    pacjent.priorytet = (rand() % 100 < ((pacjent.wiek > 45) ? 30 : 10)) ? 1 : 0;

    // Rodzic obecny tylko dla osób poniżej 18 roku życia
    pacjent.rodzic_obecny = (pacjent.wiek < 18) ? 1 : 0;

    // Losowanie lekarza z preferencjami dla popularniejszych specjalności
    int los_lekarz = rand() % 100;
    if (los_lekarz < 60) {
        pacjent.lekarz = 0; // 60% szans na lekarza POZ
    } else if (los_lekarz < 70) {
        pacjent.lekarz = 1; // 10% szans na kardiologa
    } else if (los_lekarz < 80) {
        pacjent.lekarz = 2; // 10% szans na okulistę
    } else if (los_lekarz < 90) {
        pacjent.lekarz = 3; // 10% szans na pediatrę
    } else {
        pacjent.lekarz = 4; // 10% szans na medycynę pracy
    }

    pacjent.pid = getpid();
    return pacjent;
}
void zakonczenie_procesow() {
    pid_t pid;
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
        printf("Proces o PID %d został zakończony.\n", pid);
    }
}// Funkcja do wyszukiwania PID procesów po nazwie i ich zakończenia
void cleanup_on_exit() {
    int a = 1; //DEBUG
    if (a) printf("[CLEANUP_ON_EXIT] zasoby_wyczyszczone = %d\n", zasoby_wyczyszczone);
    if (zasoby_wyczyszczone) {
        printf("[CLEANUP_ON_EXIT][DEBUG] Zasoby już zostały wyczyszczone. Pomijam.\n");
        return;
        }
    zakonczenie_procesow();
    wyczysc_procesy();
    wyczysc_kolejki();
    usun_semafor(semafor_liczba_osob);
    usun_semafor(semafor_rejestracja);
    usun_semafor(semafor_suma_kolejek);
    while (wait(NULL) > 0);


    if (!pamiec_usunieta && !pamiec_usunieta2) {
        if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
            perror("[CLEANUP_ON_EXIT][ERROR] Nie udało się usunąć pamięci współdzielonej");
        } else {
            if (a)printf("[CLEANUP_ON_EXIT]ID pamięci współdzielonej: %d\n", shm_id);
            if (a)printf("[CLEANUP_ON_EXIT][DEBUG] Pamięć współdzielona została usunięta\n");
            pamiec_usunieta = true;
        }
        if (shmctl(shm_id2, IPC_RMID, NULL) == -1) {
            perror("[CLEANUP_ON_EXIT][ERROR] Nie udało się usunąć pamięci współdzielonej 2");
        } else {
            if (a)printf("[CLEANUP_ON_EXIT]ID pamięci współdzielonej 2: %d\n", shm_id2);
            if (a)printf("[CLEANUP_ON_EXIT][DEBUG] Pamięć współdzielona 2 została usunięta\n");
            pamiec_usunieta2 = true;
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
        if(a) printf("[DEBUG] liczba osob Pamięć współdzielona utworzona i zainicjalizowana. Wartość początkowa: %d\n", *liczba_osob);

     }
void pamiec_wspoldzielona2() {
    int a = 1;
    // Tworzenie segmentu pamięci współdzielonej
    shm_id2 = shmget(PAMIEC_WSPOLDZIELONA_KLUCZ2, sizeof(int), IPC_CREAT | 0666);
    if(a) printf("[PAMIEC_WSPOLDZIELONA][DEBUG] shm_id2 = %d\n", shm_id2);
    if (shm_id2 == -1) {
        perror("Błąd tworzenia pamięci współdzielonej");
        exit(1);
    }

    suma_kolejek = (int *)shmat(shm_id2, NULL, 0);
    if (a) printf("[PAMIEC_WSPOLDZIELONA][DEBUG] suma kolejek= %d\n", *suma_kolejek);
    if (suma_kolejek == (void *)-1) {
        perror("Błąd dołączania pamięci współdzielonej");
        exit(1);
    }
    //Inicjalizacja wartości w pamięci współdzielonej
    *suma_kolejek = 0;
    if(a) printf("[DEBUG]suma kolejek Pamięć współdzielona utworzona i zainicjalizowana. Wartość początkowa: %d\n", *suma_kolejek);
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
        pamiec_wspoldzielona2();
        semafor_liczba_osob = stworz_semafor(klucz_liczba_osob);
        zwieksz_semafor(semafor_liczba_osob);
        printf("WARTOSC SEMAFORA LICZBA OSOB: %d\n", pobierz_wartosc_semafora(semafor_liczba_osob));
        // Inicjalizacja semafora rejestracji na wartość 1 (odblokowany)

        semafor_rejestracja = stworz_semafor(klucz_semafora_rejestracja);
        zwieksz_semafor(semafor_rejestracja); // Odblokowanie semafora
        printf("WARTOSC SEMAFORA REJESTRACJA: %d\n", pobierz_wartosc_semafora(semafor_rejestracja));

        semafor_suma_kolejek = stworz_semafor(klucz_semafor_suma_kolejek);
        zwieksz_semafor(semafor_suma_kolejek);
        printf("WARTOSC SEMAFORA suma kolejek: %d\n", pobierz_wartosc_semafora(semafor_suma_kolejek));
    // Inicjalizacja semafora rejestracji na wartość 1 (odblokowany)
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



    int b = 1; // Flaga debugowania
    pid_t pid_poz1, pid_poz2, pid_spec[4];

    // Tworzenie lekarzy POZ
    if ((pid_poz1 = fork()) == 0) {
        execl("./lekarz", "lekarz", "0", "1", NULL); // Typ 0 (POZ), ID 1
        perror("[MAIN][ERROR] Nie udało się uruchomić lekarza POZ1");
        exit(1);
    }
    if (b) printf("[MAIN][DEBUG] Uruchomiono proces: PID = %d, typ = LEKARZ POZ1\n", pid_poz1);

    if ((pid_poz2 = fork()) == 0) {
        execl("./lekarz", "lekarz", "0", "2", NULL); // Typ 0 (POZ), ID 2
        perror("[MAIN][ERROR] Nie udało się uruchomić lekarza POZ2");
        exit(1);
    }
    if (b) printf("[MAIN][DEBUG] Uruchomiono proces: PID = %d, typ = LEKARZ POZ2\n", pid_poz2);

    pid_t pid_dyr;

    if ((pid_dyr = fork()) == 0) {
        execl("./dyrektor", NULL);
        perror("[MAIN][ERROR] Nie udało się uruchomić Dyrektor");
        exit(1);
    }
    if (b) printf("[MAIN][DEBUG] Uruchomiono proces dyrektor: PID = %d\n", pid_dyr);

    // Tworzenie lekarzy specjalistów
     for (int i = 0; i < 4; i++) {
         if ((pid_spec[i] = fork()) == 0) {
             char kolejka_str[10];
             char typ_lekarza_str[10];
             snprintf(typ_lekarza_str, sizeof(typ_lekarza_str), "%d", i + 1);

             // Wywołanie execl
             execl("./lekarz", "lekarz", typ_lekarza_str, "0", NULL);

             // Jeśli execl się nie uda, wypisanie błędu i zakończenie procesu
             perror("[MAIN][ERROR] Nie udało się uruchomić lekarza specjalisty");
             exit(1);
         }
         if (b) printf("[MAIN][DEBUG] Uruchomiono proces: PID = %d, typ = LEKARZ SPECJALISTA (%d)\n", pid_spec[i], i+1);
    }

    //Tworzenie procesów pacjentów
    for (int i = 0; i < 30; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            // Proces potomny - pacjent
            Pacjent pacjent = losuj_pacjenta(i);
            // Konwersja pól struktury na ciągi znaków
            char id_str[10], wiek_str[10], priorytet_str[10], rodzic_str[10], lekarz_str[10], pid_str[10];
            sprintf(id_str, "%d", pacjent.id);
            sprintf(wiek_str, "%d", pacjent.wiek);
            sprintf(priorytet_str, "%d", pacjent.priorytet);
            sprintf(rodzic_str, "%d", pacjent.rodzic_obecny);
            sprintf(lekarz_str, "%d", pacjent.lekarz);
            sprintf(pid_str , "%d", pacjent.pid);

            // Uruchomienie procesu pacjent
            if (execl("./pacjent", "pacjent", id_str, wiek_str, priorytet_str, rodzic_str, lekarz_str, pid_str, NULL) == -1) {
                perror("[MAIN][ERROR] Nie udało się uruchomić procesu pacjent");
                exit(1);
            }
        } else if (pid < 0) {
            perror("[MAIN][ERROR] Fork dla pacjenta nie powiódł się");
            exit(1);
        }
        // Proces rodzica - kontynuuje tworzenie kolejnych pacjentów
        printf("[MAIN][DEBUG] Utworzono proces pacjenta z PID: %d\n", pid);
        sleep(1);// Symulacja przybywania pacjentów
    }
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
