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
        return;
    }
    printf("Aktualna wartość semafora: %d\n", val);
}

void wyswietl_liczbe_osob() {
    printf("Obecna liczba osób w przychodni: %d\n", *liczba_osob);
}

int semafor_liczba_osob;

void inicjalizuj_semafor_liczba_osob() {
    semafor_liczba_osob = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    if (semafor_liczba_osob == -1) {
        perror("Błąd tworzenia semafora liczba_osob");
        exit(1);
    }

    union semun {
        int val;
        struct semid_ds *buf;
        unsigned short *array;
    } arg;
    arg.val = 1;  // Semafor binarny (wartość początkowa 1)

    if (semctl(semafor_liczba_osob, 0, SETVAL, arg) == -1) {
        perror("Błąd ustawiania wartości semafora liczba_osob");
        exit(1);
    }
}

void zablokuj_semafor() {
    struct sembuf operacja = {0, -1, 0};  // Zablokowanie (sem_op = -1)
    if (semop(semafor_liczba_osob, &operacja, 1) == -1) {
        perror("Błąd blokowania semafora liczba_osob");
        exit(1);
    }
}

void odblokuj_semafor() {
    struct sembuf operacja = {0, 1, 0};  // Odblokowanie (sem_op = 1)
    if (semop(semafor_liczba_osob, &operacja, 1) == -1) {
        perror("Błąd odblokowywania semafora liczba_osob");
        exit(1);
    }
}

void loguj_liczba_osob(const char* akcja) {
    FILE* log = fopen("liczba_osob.log", "a");
    if (log != NULL) {
        fprintf(log, "[%s] Liczba osób w przychodni: %d\n", akcja, *liczba_osob);
        fclose(log);
    } else {
        perror("Błąd otwierania pliku logów");
    }
}

void usun_semafor_liczba_osob() {
    if (semctl(semafor_liczba_osob, 0, IPC_RMID) == -1) {
        perror("Błąd usuwania semafora liczba_osob");
    } else {
        printf("Semafor liczba_osob został usunięty.\n");
    }
}