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


// Klucz dla pamięci współdzielonej
#define PAMIEC_WSPOLDZIELONA_KLUCZ 6789

// Wskaźnik do pamięci współdzielonej
int shm_id;
int *liczba_osob;
void cleanup_on_exit() {
    shmctl(shm_id, IPC_RMID, NULL); // Usuwanie pamięci współdzielonej
    usun_semafor();                // Usuwanie semafora
    usun_semafor_liczba_osob();    // Usuwanie semafora liczby osób
    printf("Wszystkie zasoby zostały wyczyszczone.\n");
    exit(0);
}

// Funkcja do obsługi sygnałów
void signal_handler(int sig) {
    cleanup_on_exit();
}

int main() {

    signal(SIGINT, signal_handler);  // Obsługa sygnału CTRL+C
    signal(SIGTERM, signal_handler); // Obsługa sygnału kill
    signal(SIGKILL, signal_handler); // Obsługa siłowego zakończenia programu (jeśli to możliwe)

    // Reszta kodu main...
    // Utworzenie segmentu pamięci współdzielonej
    int shm_id = shmget(PAMIEC_WSPOLDZIELONA_KLUCZ, sizeof(int), IPC_CREAT | 0666);
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

    // Inicjalizacja semafora
    inicjalizuj_semafor();
    inicjalizuj_semafor_liczba_osob();

    // Tworzenie procesów z logowaniem
    if (fork() == 0) {
        log_process("START", "Zarzadca_Kolejka_Zewnetrzna", 0);
        zarzadz_kolejka_zewnetrzna();
        log_process("END", "Zarzadca_Kolejka_Zewnetrzna", 0);
        exit(0);
    }

    if (fork() == 0) {
        log_process("START", "Rejestracja", 0);
        rejestracja();
        log_process("END", "Rejestracja", 0);
        exit(0);
    }

    if (fork() == 0) {
        log_process("START", "Lekarz_POZ", 1);
        lekarz_poz(1, X1);
        log_process("END", "Lekarz_POZ", 1);
        exit(0);
    }

    if (fork() == 0) {
        log_process("START", "Lekarz_POZ", 2);
        lekarz_poz(2, X1);
        log_process("END", "Lekarz_POZ", 2);
        exit(0);
    }

    int kolejki_specjalistow[] = {KOLEJKA_KARDIOLOG, KOLEJKA_OKULISTA, KOLEJKA_PEDIATRA, KOLEJKA_MEDYCYNA_PRACY};
    int limity_specjalistow[] = {X2, X3, X4, X5};

    for (int i = 0; i < 4; i++) {
        if (fork() == 0) {
            log_process("START", "Lekarz_Specjalista", kolejki_specjalistow[i]);
            lekarz_specjalista(kolejki_specjalistow[i], limity_specjalistow[i]);
            log_process("END", "Lekarz_Specjalista", kolejki_specjalistow[i]);
            exit(0);
        }
    }

    // Tworzenie procesów pacjentów
    for (int i = 0; i < 2; i++) {
        if (fork() == 0) {
            log_process("START", "Pacjent", i);
            pacjent(i);
            log_process("END", "Pacjent", i);
            exit(0);
        }
        sleep(1); // Symulacja przybywania pacjentów
    }
    // Oczekiwanie na zakończenie procesów
    while (wait(NULL) > 0);

    // Usunięcie pamięci współdzielonej
    shmctl(shm_id, IPC_RMID, NULL);

    // Usunięcie semafora
    usun_semafor();
    usun_semafor_liczba_osob();

    return 0;
}
