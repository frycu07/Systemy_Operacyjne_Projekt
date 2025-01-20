#include "kolejka.h"
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

// Klucz dla pamięci współdzielonej
#define PAMIEC_WSPOLDZIELONA_KLUCZ 6789

// Wskaźnik do pamięci współdzielonej
int shm_id;
int *liczba_osob;
int semafor_rejestracja;
pid_t my_pid;
bool zasoby_wyczyszczone = false;

// Cleanup i signal handler
void cleanup_on_exit() {
    int a = 0; //DEBUG
    if (a) printf("[CLEANUP_ON_EXIT] zasoby_wyczyszczone = %d\n", zasoby_wyczyszczone);
    if (zasoby_wyczyszczone) {
        printf("[CLEANUP_ON_EXIT][DEBUG] Zasoby już zostały wyczyszczone. Pomijam.\n");
        return;
    }
    // Usuwanie semafora rejestracji
    if (semctl(semafor_rejestracja, 0, IPC_RMID) == -1) {
        perror("[CLEANUP_ON_EXIT][ERROR] Nie udało się usunąć semafora rejestracja");
    } else {
        if (a) printf("[CLEANUP_ON_EXIT][DEBUG] Semafor rejestracja został usunięty (ID = %d)\n", semafor_rejestracja);

    }
    usun_semafor_liczba_osob();
    wyczysc_kolejki();
    // Usuwanie pamięci współdzielonej
    if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
        perror("[CLEANUP_ON_EXIT][ERROR] Nie udało się usunąć pamięci współdzielonej");
    } else {
        if (a)printf("[CLEANUP_ON_EXIT]ID pamięci współdzielonej: %d\n", shm_id);
        if (a)printf("[CLEANUP_ON_EXIT][DEBUG] Pamięć współdzielona została usunięta\n");
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

// Funkcja czyszcząca zakończone procesy
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
    int a = 0;
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
    my_pid = getpid(); // Ustaw PID procesu głównego
    printf("[MAIN] Proces główny uruchomiony z PID: %d\n", my_pid);
    int a=0;
    pthread_t cleaner_thread, monitor_thread;
    zakonczenie_poprzednich_procesow();
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    srand(time(NULL));  // Inicjalizacja generatora liczb losowych

    // Utworzenie segmentu pamięci współdzielonej
    pamiec_wspoldzielona();

    key_t klucz_liczba_osob = 1234;
    stworz_semafor_liczba_osob(klucz_liczba_osob);

    // Inicjalizacja semafora rejestracji na wartość 1 (odblokowany)
    key_t klucz_semafora_rejestracja = 1233; // Unikalny klucz semafora
    semafor_rejestracja = stworz_semafor(klucz_semafora_rejestracja);

    if (semafor_rejestracja == -1) {
        perror("[ERROR] Nie udało się utworzyć semafora dla rejestracji");
        exit(1);
    }
    zwieksz_semafor(semafor_rejestracja); // Odblokowanie semafora
    if (a) printf("[MAIN][DEBUG] Semafor rejestracji został utworzony (warrosc = %d)\n", semafor_rejestracja);

    if (fork() == 0) {
        int a = 0; //DEBUG
        if (a)printf("[MAIN][DEBUG] Uruchamiam proces rejestracji (ID: 0)\n");
        rejestracja(0, semafor_rejestracja);
        if (a)printf("[MAIN][DEBUG] Uruchomiono proces: PID = %d, typ = Rejestracja 0\n", getpid());
        log_process("END", "Rejestracja", 0);
        exit(0);
    }
    if (fork() == 0) {
        zarzadz_kolejka_zewnetrzna();
        printf("[DEBUG] Uruchomiono proces: PID = %d, typ = Zarzadzanie kolejka zew\n", getpid());
        exit(0);
    }

    // Tworzenie kolejki rejestracji
    int kolejka_rejestracja = msgget(KOLEJKA_REJESTRACJA, IPC_CREAT | 0666);
    if (kolejka_rejestracja == -1) {
        perror("Błąd tworzenia kolejki rejestracji");
        exit(1);
    }

    ArgumentyRejestracja args = {kolejka_rejestracja, semafor_rejestracja, MAX_OSOB_W_PRZYCHODNI};
    if (pthread_create(&monitor_thread, NULL, (void *)zarzadz_i_monitoruj_rejestracje, (void *)&args) != 0) {
        perror("Błąd tworzenia wątku monitorującego");
        exit(1);
    }

    int b = 1; // TWORZENIE LEKARZY

    if (fork() == 0) {
        lekarz_poz(1, X1);
        if (b)printf("[MAIN][DEBUG] Uruchomiono proces: PID = %d, typ = LEKARZ POZ1\n", getpid());
        log_process("END", "Lekarz_POZ", 1);
        exit(0);
    }

    if (fork() == 0) {
        lekarz_poz(2, X1);
        if (b)printf("[MAIN][DEBUG] Uruchomiono proces: PID = %d, typ = LEKARZ POZ2\n", getpid());
        log_process("END", "Lekarz_POZ", 2);
        exit(0);
    }

    int kolejki_specjalistow[] = {KOLEJKA_KARDIOLOG, KOLEJKA_OKULISTA, KOLEJKA_PEDIATRA, KOLEJKA_MEDYCYNA_PRACY};
    int limity_specjalistow[] = {X2, X3, X4, X5};

    for (int i = 0; i < 4; i++) {
        if (fork() == 0) {
            lekarz_specjalista(kolejki_specjalistow[i], limity_specjalistow[i]);
            if (b)printf("[DEBUG] Uruchomiono proces: PID = %d, typ = %x\n", getpid(), kolejki_specjalistow[i]);
            log_process("END", "Lekarz_Specjalista", kolejki_specjalistow[i]);
            exit(0);
        }
    }

    // Tworzenie wątku do oczyszczania zakończonych procesów
    if (pthread_create(&cleaner_thread, NULL, process_cleaner, NULL) != 0) {
        perror("Błąd tworzenia wątku czyszczącego");
        exit(1);
    }

    // Tworzenie procesów pacjentów
    for (int i = 0; i < 100; i++) {
        if (fork() == 0) {
            printf("KROK 1 Przyszedł pacjent ID: %d\n", i);
            pacjent(i);
            log_process("END", "Pacjent", i);
            exit(0);
        }
        sleep(1); // Symulacja przybywania pacjentów
    }
//sleep(10);
    // Oczekiwanie na zakończenie wątków
    pthread_join(monitor_thread, NULL);
    pthread_join(cleaner_thread, NULL);

    // Oczekiwanie na zakończenie procesów
    while (wait(NULL) > 0);

    return 0;
}
