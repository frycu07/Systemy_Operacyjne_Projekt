#ifndef REJESTRACJA_H
#define REJESTRACJA_H
#include "pacjent.h"

struct ZarzadzanieArgumenty {
    int max_osob_w_przychodni;
    int kolejka_rejestracja;
};

void rejestracja(int id);
void zakoncz_wizyte(Pacjent pacjent);
int aktualna_godzina();
void zarzadz_kolejka_zewnetrzna();
void zarzadz_okienkami(void *argumenty);
void monitoruj_kolejke_rejestracja(int N, int kolejka_rejestracja);

#endif