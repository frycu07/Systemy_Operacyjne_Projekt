#include <stdio.h>
#include "kolejka.h"
#include "semafor.c"

void wyczysc_kolejki() {
    int a = 0;
    printf("wyczysc kolejki dziala");
    int kolejki[] = {KOLEJKA_ZEWNETRZNA, KOLEJKA_REJESTRACJA, KOLEJKA_POZ, KOLEJKA_KARDIOLOG, KOLEJKA_OKULISTA, KOLEJKA_PEDIATRA, KOLEJKA_MEDYCYNA_PRACY};
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
    zmniejsz_semafor(semafor_liczba_osob);
    *liczba_osob += zmiana;
    printf("[Monitorowanie] CALKOWITA Liczba osób w przychodni: %d\n", *liczba_osob);
    //log_process("ZMIANA LICZBY OSOB", "Zmien liczbe", *liczba_osob);
    zwieksz_semafor(semafor_liczba_osob);
}
