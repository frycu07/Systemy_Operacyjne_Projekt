#ifndef CZAS_H
#define CZAS_H

#include <stdbool.h>

typedef struct {
    int godzina; // Godzina (np. 9)
    int minuta;  // Minuta (np. 15)
} Czas;

// Deklaracje zmiennych globalnych
extern Czas czas_otwarcia;
extern Czas czas_zamkniecia;

// Funkcje
Czas aktualny_czas();
int porownaj_czas(Czas czas1, Czas czas2);
bool czy_przychodnia_otwarta();

#endif