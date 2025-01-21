#include <sys/sem.h>
#include "kolejka.h"

int semafor_liczba_osob;

void sprawdz_wartosc_semafora() {
    int val = semctl(semafor_liczba_osob, 0, GETVAL);
    if (val == -1) {
        perror("Błąd odczytu wartości semafora");
        return;
    }
    printf("Aktualna wartość semafora: %d\n", val);
}

void stworz_semafor_liczba_osob(key_t klucz) {
    int a =0; //DEBUG
    semafor_liczba_osob = semget(klucz, 1, IPC_CREAT | 0666);
    if (a) printf("[STWORZ_SEMAFOR_LB_OSOB]ID semafora liczba osob: %d, klucz: 0x%x\n", semafor_liczba_osob, klucz);
    if (semafor_liczba_osob == -1) {
        perror("Błąd tworzenia semafora liczba_osob");
        exit(1);
    }
    // Ustawienie wartości semafora na 1
    union semun {
        int val;
        struct semid_ds *buf;
        unsigned short *array;
    } argument;
    argument.val = 1;
    if (semctl(semafor_liczba_osob, 0, SETVAL, argument) == -1) {
        perror("Błąd ustawienia wartości semafora liczba_osob");
        exit(1);
    }
}

void usun_semafor_liczba_osob() {
    int a = 0; //DEBUG
    if (a)printf("[usun_semafor_liczba_osob][DEBUG] Próba usunięcia semafora liczba osob: ID = %d\n", semafor_liczba_osob);
    if (semctl(semafor_liczba_osob, 0, IPC_RMID) == -1) {
        perror("[ERROR] Błąd usuwania semafora liczba osob");
    } else {
        if (a)printf("[usun_semafor_liczba_osob][DEBUG] Semafor liczba osob został usunięty (ID = %d)\n", semafor_liczba_osob);
    }
}
void zablokuj_semafor() {
    int a = 0;
    struct sembuf operacja = {0, -1, 0};  // Zablokowanie (sem_op = -1)
if (a)printf("[DEBUG] Blokuję semafor liczba osob: %d\n", semafor_liczba_osob);
    if (semop(semafor_liczba_osob, &operacja, 1) == -1) {
        perror("Błąd blokowania semafora liczba_osob");
        fprintf(stderr, "ID semafora: %d\n", semafor_liczba_osob);
        exit(1);
    }
}

void odblokuj_semafor() {
    int a = 0;
    struct sembuf operacja = {0, 1, 0};  // Odblokowanie (sem_op = 1)
    if (a)printf("[DEBUG] Odblokuję semafor liczba osob: %d\n", semafor_liczba_osob);
    if (semop(semafor_liczba_osob, &operacja, 1) == -1) {
        perror("Błąd odblokowywania semafora liczba_osob");
        fprintf(stderr, "ID semafora: %d\n", semafor_liczba_osob);
        exit(1);
    }
}

// Tworzenie semafora
int stworz_semafor(key_t klucz) {
    int a = 0; //DEBUG
    int semafor = semget(klucz, 1, IPC_CREAT | 0666);
    if (a)printf("ID semafora rejestracja: %d, klucz: 0x%x\n", semafor, klucz);
    if (semafor == -1) {
        perror("Błąd tworzenia semafora");
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
    if (a)printf("[DEBUG] Zwiększam semafor rejestracja: %d\n", semafor);
    if (semctl(semafor, 0, IPC_RMID) == -1) {
        perror("Błąd usuwania semafora rejestracja");
        exit(1);
    }
}
