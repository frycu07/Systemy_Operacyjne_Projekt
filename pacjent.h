#ifndef PACJENT_H
#define PACJENT_H
#include <sys/types.h>

// Struktura pacjenta
typedef struct {
    int id;          // ID pacjenta
    int wiek;        // Wiek pacjenta
    int priorytet;   // 1 - VIP, 0 - normalny
    int rodzic_obecny; // 1 - rodzic obecny, 0 - brak rodzica
    int lekarz;      // 0 - POZ, 1 - Kardiolog, itd.
    pid_t pid;
    int koniec;
} Pacjent;

void pacjent_zarzadzanie(Pacjent pacjent);

// Struktura komunikatu
typedef struct {
    long typ;        // Typ komunikatu (1 - rejestracja, 2 - lekarz POZ, itd.)
    Pacjent pacjent; // Dane pacjenta
} Komunikat;



#endif