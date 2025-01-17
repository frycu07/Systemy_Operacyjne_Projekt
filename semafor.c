#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
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
    semafor_liczba_osob = semget(klucz, 1, IPC_CREAT | 0666);
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
    if (semctl(semafor_liczba_osob, 0, IPC_RMID) == -1) {
        perror("Błąd usuwania semafora");
    }
}

void zablokuj_semafor() {
    struct sembuf operacja = {0, -1, 0};  // Zablokowanie (sem_op = -1)
    if (semop(semafor_liczba_osob, &operacja, 1) == -1) {
        perror("Błąd blokowania semafora liczba_osob");
        fprintf(stderr, "ID semafora: %d\n", semafor_liczba_osob);
        exit(1);
    }
}

void odblokuj_semafor() {
    struct sembuf operacja = {0, 1, 0};  // Odblokowanie (sem_op = 1)
    if (semop(semafor_liczba_osob, &operacja, 1) == -1) {
        perror("Błąd odblokowywania semafora liczba_osob");
        fprintf(stderr, "ID semafora: %d\n", semafor_liczba_osob);
        exit(1);
    }
}

