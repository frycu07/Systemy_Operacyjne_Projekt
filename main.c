#include "kolejka.h"
#include <unistd.h>
#include "pacjent.h"
#include "rejestracja.h"
#include "lekarz.h"
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "czas.h"
#include "procesy.h"
#include <signal.h>
#include <pthread.h>
#include <errno.h>

// Klucz dla pamięci współdzielonej
#define PAMIEC_WSPOLDZIELONA_KLUCZ 6789

// Wskaźnik do pamięci współdzielonej
int shm_id;
int *liczba_osob;

// Cleanup i signal handler
void cleanup_on_exit() {
    shmctl(shm_id, IPC_RMID, NULL); // Usuwanie pamięci współdzielonej
    usun_semafor_liczba_osob();
    printf("Wszystkie zasoby zostały wyczyszczone.\n");
    exit(0);
}

void signal_handler(int sig) {
    cleanup_on_exit();
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

// Funkcja monitorująca kolejkę rejestracji
void* monitoruj_kolejke_thread(void* arg) {
    int kolejka_rejestracja = *((int*)arg);
    monitoruj_kolejke_rejestracja(MAX_OSOB_W_PRZYCHODNI, kolejka_rejestracja);
    return NULL;
}

int main() {
    pthread_t cleaner_thread, monitor_thread;
    zakonczenie_poprzednich_procesow();
    wyczysc_kolejki(); // Usuwanie kolejek
    srand(time(NULL));  // Inicjalizacja generatora liczb losowych


    // Utworzenie segmentu pamięci współdzielonej
    shm_id = shmget(PAMIEC_WSPOLDZIELONA_KLUCZ, sizeof(int), IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("Błąd tworzenia pamięci współdzielonej");
        exit(1);
    }

    // Dołączenie pamięci współdzielonej
    liczba_osob = (int *)shmat(shm_id, NULL, 0);
    if (liczba_osob == (void *)-1) {
        perror("Błąd dołączania pamięci współdzielonej");
        exit(1);
    }

    // Inicjalizacja liczby osób
    *liczba_osob = 0;

    // Inicjalizacja semaforów
    key_t klucz_semafora = 1234; // Klucz semafora (dowolny unikalny numer)
    stworz_semafor_liczba_osob(klucz_semafora);

    // Tworzenie kolejki rejestracji
    int kolejka_rejestracja = msgget(KOLEJKA_REJESTRACJA, IPC_CREAT | 0666);
    if (kolejka_rejestracja == -1) {
        perror("Błąd tworzenia kolejki rejestracji");
        exit(1);
    }

    // Tworzenie procesów z logami
    if (fork() == 0) {
        log_process("START", "Zarzadca_Kolejka_Zewnetrzna", 0);
        zarzadz_kolejka_zewnetrzna();
        log_process("END", "Zarzadca_Kolejka_Zewnetrzna", 0);
        exit(0);
    }
    struct ZarzadzanieArgumenty arg = {MAX_OSOB_W_PRZYCHODNI, kolejka_rejestracja};


    pthread_t zarzadzanie_okienkami_thread;
    if (pthread_create(&zarzadzanie_okienkami_thread, NULL, (void *)zarzadz_okienkami, (void *)&arg) != 0) {
        perror("Błąd tworzenia wątku zarządzania okienkami");
        exit(1);
    }

    if (fork() == 0) {
        rejestracja(0);
        log_process("END", "Rejestracja", 0);
        exit(0);
    }

    // Tworzenie wątku monitorującego kolejkę rejestracji
    if (pthread_create(&monitor_thread, NULL, monitoruj_kolejke_thread, (void *)&kolejka_rejestracja) != 0) {
        perror("Błąd tworzenia wątku monitorującego kolejkę rejestracji");
        exit(1);
    }

    if (fork() == 0) {
        lekarz_poz(1, X1);
        log_process("END", "Lekarz_POZ", 1);
        exit(0);
    }

    if (fork() == 0) {
        lekarz_poz(2, X1);
        log_process("END", "Lekarz_POZ", 2);
        exit(0);
    }

    int kolejki_specjalistow[] = {KOLEJKA_KARDIOLOG, KOLEJKA_OKULISTA, KOLEJKA_PEDIATRA, KOLEJKA_MEDYCYNA_PRACY};
    int limity_specjalistow[] = {X2, X3, X4, X5};

    for (int i = 0; i < 4; i++) {
        if (fork() == 0) {
            lekarz_specjalista(kolejki_specjalistow[i], limity_specjalistow[i]);
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
    for (int i = 0; i < 15; i++) {
        if (fork() == 0) {
            printf("Przyszedł pacjent ID: %d\n", i);
            pacjent(i);
            log_process("END", "Pacjent", i);
            exit(0);
        }
        sleep(1); // Symulacja przybywania pacjentów
    }

    // Oczekiwanie na zakończenie wątków
    pthread_join(monitor_thread, NULL);
    pthread_join(cleaner_thread, NULL);

    // Oczekiwanie na zakończenie procesów
    while (wait(NULL) > 0);

    // Usunięcie pamięci współdzielonej
    shmctl(shm_id, IPC_RMID, NULL);

    //usuniecie semafora

    usun_semafor_liczba_osob();

    return 0;
}