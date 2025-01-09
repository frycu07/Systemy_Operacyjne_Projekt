#include "kolejka.h"
#include <unistd.h>
#include "pacjent.h"
#include "rejestracja.h"
#include "lekarz.h"
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "czas.h"

// Klucz dla pamięci współdzielonej
#define PAMIEC_WSPOLDZIELONA_KLUCZ 6789

// Wskaźnik do pamięci współdzielonej
int *liczba_osob;

int main() {

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

    // Tworzenie procesów
    if (fork() == 0) zarzadz_kolejka_zewnetrzna(); // Zarządca kolejki zewnętrznej
    if (fork() == 0) rejestracja(); // Proces rejestracji
    if (fork() == 0) lekarz_poz(1, X1); // Lekarz POZ 1
    if (fork() == 0) lekarz_poz(2, X1); // Lekarz POZ 2
    if (fork() == 0) lekarz_specjalista(KOLEJKA_KARDIOLOG, X2); // Kardiolog
    if (fork() == 0) lekarz_specjalista(KOLEJKA_OKULISTA, X3); // Okulista
    if (fork() == 0) lekarz_specjalista(KOLEJKA_PEDIATRA, X4); // Pediatra
    if (fork() == 0) lekarz_specjalista(KOLEJKA_MEDYCYNA_PRACY, X5); // Medycyna pracy

    // Tworzenie procesów pacjentów
    for (int i = 0; i < 10; i++) {
        if (fork() == 0) pacjent(i);
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
