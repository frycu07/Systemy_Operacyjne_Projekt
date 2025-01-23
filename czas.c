#include "czas.h"
#include <time.h>
#include <stdbool.h>

// Definicje zmiennych globalnych
Czas czas_otwarcia = {00, 00};    // Otwarcie o 8:15
Czas czas_zamkniecia = {22, 50}; // Zamknięcie o 23:30

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
    int porownanie_otwarcia = porownaj_czas(godzina, czas_otwarcia);

    int porownanie_zamkniecia = porownaj_czas(godzina, czas_zamkniecia);

    // Debug: Wyświetlanie informacji o czasie
    // printf("[DEBUG] Aktualny czas: %02d:%02d\n", godzina.godzina, godzina.minuta);
    // printf("[DEBUG] Porównanie z czasem otwarcia: %d\n", porownanie_otwarcia);
    // printf("[DEBUG] Porównanie z czasem zamknięcia: %d\n", porownanie_zamkniecia);

    // Sprawdzenie, czy przychodnia jest otwarta
    bool otwarta = porownanie_otwarcia >= 0 && porownanie_zamkniecia < 0;
    // printf("[DEBUG] Czy przychodnia otwarta? %s\n", otwarta ? "TAK" : "NIE");

    return otwarta;
}