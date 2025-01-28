#include <stdio.h>
#include "kolejka.h"

#include <signal.h>
#include <unistd.h>
#include "semafor.c"


void wyczysc_kolejki() {
    int a = 0;
    printf("wyczysc kolejki dziala");
    int kolejki[] = {KOLEJKA_BADANIA_BASE,KOLEJKA_BADANIA_MEDYCYNA,KOLEJKA_BADANIA_OKULISTA, KOLEJKA_BADANIA_PEDIATRA, KOLEJKA_BADANIA_KARDIOLOG, KOLEJKA_VIP_POZ, KOLEJKA_VIP_OKULISTA, KOLEJKA_VIP_PEDIATRA, KOLEJKA_VIP_KARDIOLOG, KOLEJKA_VIP_MEDYCYNA_PRACY, KOLEJKA_ZEWNETRZNA, KOLEJKA_REJESTRACJA, KOLEJKA_POZ, KOLEJKA_KARDIOLOG, KOLEJKA_OKULISTA, KOLEJKA_PEDIATRA, KOLEJKA_MEDYCYNA_PRACY};
    for (int i = 0; i < sizeof(kolejki) / sizeof(kolejki[0]); i++) {
        int msg_id = msgget(kolejki[i], IPC_CREAT | 0666);
        if (msg_id != -1) {
            if (msgctl(msg_id, IPC_RMID, NULL) == -1) {
                perror("[ERROR] Nie udało się usunąć kolejki");
                printf("[DEBUG] ID kolejki: %d, klucz: 0x%x\n", msg_id, kolejki[i]);
            } else {
                if (a)printf("[DEBUG] Kolejka o ID: %d (klucz: 0x%x) została usunięta\n", msg_id, kolejki[i]);
            }
        } else {
            perror("[ERROR] Nie udało się otworzyć kolejki");
            printf("[DEBUG] Klucz: 0x%x\n", kolejki[i]);
        }
    }

}

void zmien_liczba_osob(int zmiana) {

    int semafor_liczba_osob = uzyskaj_dostep_do_semafora(klucz_liczba_osob);
    //printf("[ZMIEN LICZBE OSOB]WARTOSC SEMAFORA LICZBA OSOB: %d\n", pobierz_wartosc_semafora(semafor_liczba_osob));
    printf("[Monitorowanie] CALKOWITA Liczba osób w przychodni przed zmiana: %d, ZMIANA: %d\n", *liczba_osob, zmiana);
    zmniejsz_semafor(semafor_liczba_osob);
    *liczba_osob += zmiana;
    printf("[Monitorowanie] CALKOWITA Liczba osób w przychodni po zmianie: %d, ZMIANA: %d\n", *liczba_osob, zmiana);
    zwieksz_semafor(semafor_liczba_osob);
}
int sprawdz_kolejke(int kolejka_id) {
    struct msqid_ds statystyki;

    // Pobranie informacji o kolejce
    if (msgctl(kolejka_id, IPC_STAT, &statystyki) == -1) {
        perror("Błąd pobierania informacji o kolejce");
        exit(1);
    }

    // Zwrócenie liczby wiadomości w kolejce
    return (int)statystyki.msg_qnum;
}

void zakoncz_wizyte(Pacjent pacjent) {

    // Zmniejsz `liczba_osob` w zależności od obecności rodzica.
    if (pacjent.rodzic_obecny) {
        zmien_liczba_osob(-2);
        printf("KROK 9 Pacjent ID: %d (z rodzicem) opuścił przychodnię. Liczba osób w przychodni: %d\n",
               pacjent.id, *liczba_osob);
    } else {
        zmien_liczba_osob(-1);
        printf("KROK 9 Pacjent ID: %d opuścił przychodnię. Liczba osób w przychodni: %d\n",
               pacjent.id, *liczba_osob);
    }
    printf("[DEBUG] Proces pacjenta ID: %d kończy się.\n", pacjent.id);
    sleep(1);
    if (kill(pacjent.pid, SIGTERM) == -1) {
        printf("Nie udało się zakończyć procesu pacjenta: %d\n", pacjent.pid);
        perror("[REJESTRACJA][ERROR] Nie udało się zakończyć procesu pacjenta");
    }
}

void dodaj_suma_kolejek() {
    int semafor_suma_kolejek = uzyskaj_dostep_do_semafora(klucz_semafor_suma_kolejek);
    zmniejsz_semafor(semafor_suma_kolejek);
    *suma_kolejek += 1;
    printf("[Monitorowanie] SUMA KOLEJEK Liczba osób w przychodni: %d\n", *suma_kolejek);
    zwieksz_semafor(semafor_suma_kolejek);
}