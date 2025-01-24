#ifndef REJESTRACJA_H
#define REJESTRACJA_H
#include "pacjent.h"

struct ZarzadzanieArgumenty {
    int max_osob_w_przychodni;
    int kolejka_rejestracja;
};

typedef struct {
    int id;               // ID pacjenta
    char *skierowanie_do; // Gdzie skierowano pacjenta (np. "Kardiolog")
    char *wystawil;    // Kto wystawił skierowanie (np. "Rejestracja")
} RaportPacjenta;


void zarzadz_kolejka_zewnetrzna();
void rejestracja(int id);
int aktualna_godzina();
void zarzadz_i_monitoruj_rejestracje();
void zapisz_do_raportu(RaportPacjenta pacjent);


#endif