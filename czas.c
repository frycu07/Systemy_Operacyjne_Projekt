#include "czas.h"
#include <time.h>
#include <stdbool.h>

// Definicje zmiennych globalnych
Czas czas_otwarcia = {8, 15};    // Otwarcie o 8:15
Czas czas_zamkniecia = {23, 30}; // Zamknięcie o 23:30

// Funkcja zwracająca aktualny czas
Czas aktualny_czas() {
    time_t teraz = time(NULL);
    struct tm *czas = localtime(&teraz);
    Czas wynik = {czas->tm_hour, czas->tm_min};
    return wynik;
}

// Funkcja porównująca dwa czasy
int porownaj_czas(Czas czas1, Czas czas2) {
    if (czas1.godzina < czas2.godzina) return -1;
    if (czas1.godzina > czas2.godzina) return 1;
    if (czas1.minuta < czas2.minuta) return -1;
    if (czas1.minuta > czas2.minuta) return 1;
    return 0; // Czasy są równe
}

// Funkcja sprawdzająca, czy przychodnia jest otwarta
bool czy_przychodnia_otwarta() {
    Czas godzina = aktualny_czas();
    return porownaj_czas(godzina, czas_otwarcia) >= 0 && porownaj_czas(godzina, czas_zamkniecia) < 0;
}