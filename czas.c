#include "czas.h"
#include <time.h>

// Definicje zmiennych globalnych
Czas czas_otwarcia = {8, 15};    // Otwarcie o 9:15
Czas czas_zamkniecia = {23, 30}; // Zamknięcie o 18:30

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