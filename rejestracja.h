#ifndef REJESTRACJA_H
#define REJESTRACJA_H
#include "pacjent.h"

struct ZarzadzanieArgumenty {
    int max_osob_w_przychodni;
    int kolejka_rejestracja;
};

typedef struct {
    int id;               // ID pacjenta
    char skierowanie_do[50]; // Gdzie skierowano pacjenta (np. "Kardiolog")
    char wystawil[50];    // Kto wystawił skierowanie (np. "Rejestracja")
} RaportPacjenta;


void rejestracja(int id, int semafor_rejestracja);
void zakoncz_wizyte(Pacjent pacjent);
int aktualna_godzina();
void zarzadz_kolejka_zewnetrzna();
void zarzadz_i_monitoruj_rejestracje(void *argumenty);
void zapisz_do_raportu(RaportPacjenta pacjent);


#endif