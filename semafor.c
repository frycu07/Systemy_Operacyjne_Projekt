#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include "kolejka.h"

int semafor_id;

void inicjalizuj_semafor() {
    semafor_id = semget(5678, 1, IPC_CREAT | 0666);
    if (semafor_id == -1) {
        perror("Błąd tworzenia semafora");
        exit(1);
    }

    union semun {
        int val;
        struct semid_ds *buf;
        unsigned short *array;
    } arg;
    arg.val = 1;

    if (semctl(semafor_id, 0, SETVAL, arg) == -1) {
        perror("Błąd inicjalizacji wartości semafora");
        exit(1);
    }
}

void usun_semafor() {
    if (semctl(semafor_id, 0, IPC_RMID) == -1) {
        perror("Błąd usuwania semafora");
    }
}

void semafor_op(int delta) {
    struct sembuf operacja;
    operacja.sem_num = 0;    // Indeks semafora
    operacja.sem_op = delta; // Operacja (-1, 1)
    operacja.sem_flg = 0;    // Brak dodatkowych flag

    //printf("sem_num: %d, sem_op: %d, sem_flg: %d\n", operacja.sem_num, operacja.sem_op, operacja.sem_flg);

    if (semop(semafor_id, &operacja, 1) == -1) {
        perror("Błąd operacji na semaforze");
        exit(1);
    }
}

void sprawdz_wartosc_semafora() {
    int val = semctl(semafor_id, 0, GETVAL);
    if (val == -1) {
        perror("Błąd odczytu wartości semafora");
    } else {
        printf("Aktualna wartość semafora: %d\n", val);
    }
}

void wyswietl_liczbe_osob() {
    printf("Obecna liczba osób w przychodni: %d\n", *liczba_osob);
}