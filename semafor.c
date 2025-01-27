#include <sys/sem.h>
#include "kolejka.h"

// Tworzenie semafora
int stworz_semafor(key_t klucz) {
    int a = 0; //DEBUG
    int semafor = semget(klucz, 1, IPC_CREAT | 0666);
    if (a)printf("ID semafora: %d, klucz: 0x%x\n", semafor, klucz);
    if (semafor == -1) {
        perror("Błąd tworzenia semafora");
        exit(1);
    }
    return semafor;
}

int uzyskaj_dostep_do_semafora(key_t klucz_semafora) {
        int semafor = semget(klucz_semafora, 1, 0);
        if (semafor == -1) {
            printf("[SEMAFOR] Błąd dostępu do semafora o kluczu %d\n", klucz_semafora);
            perror("Błąd dostępu do semafora o kluczu");
            exit(1);
        }
        return semafor;
    }

// Ustawianie wartości semafora
void ustaw_wartosc_semafora(int semafor, int wartosc) {
    if (semctl(semafor, 0, SETVAL, wartosc) == -1) {
        perror("Błąd ustawiania wartości semafora");
        exit(1);
    }
}

// Pobieranie wartości semafora
int pobierz_wartosc_semafora(int semafor) {
    int wartosc = semctl(semafor, 0, GETVAL);
    if (wartosc == -1) {
        perror("Błąd pobierania wartości semafora");
        exit(1);
    }
    return wartosc;
}

// Operacja na semaforze: zwiększanie
void zwieksz_semafor(int semafor) {
    int a = 0; //DEBUG
    struct sembuf operacja = {0, 1, 0}; // Zwiększenie o 1
    if (a)printf("[DEBUG] Zwiększam semafor rejestracja: %d\n", semafor);
    if (semop(semafor, &operacja, 1) == -1) {
        perror("Błąd zwiększania semafora");
        exit(1);
    }
}

// Operacja na semaforze: zmniejszanie
void zmniejsz_semafor(int semafor) {
    struct sembuf operacja = {0, -1, 0}; // Zmniejszenie o 1
    int a = 0; //DEBUG
    if (a)printf("[DEBUG] Zmniejszam semafor rejestracja: %d\n", semafor);
    if (semop(semafor, &operacja, 1) == -1) {
        perror("Błąd zmniejszania semafora");
        exit(1);
    }
}

// Usuwanie semafora
void usun_semafor(int semafor) {
    int a = 0; //DEBUG
    if (a)printf("[DEBUG] Usuwam semafor rejestracja: %d\n", semafor);
    if (semctl(semafor, 0, IPC_RMID) == -1) {
        perror("Błąd usuwania semafora rejestracja");
        exit(1);
    }
}
