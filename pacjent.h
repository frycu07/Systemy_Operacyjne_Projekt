#ifndef PACJENT_H
#define PACJENT_H

void pacjent(int id);

// Struktura pacjenta
typedef struct {
    int id;          // ID pacjenta
    int wiek;        // Wiek pacjenta
    int priorytet;   // 1 - VIP, 0 - normalny
    int lekarz;      // 0 - POZ, 1 - Kardiolog, itd.
} Pacjent;

// Struktura komunikatu
typedef struct {
    long typ;        // Typ komunikatu (1 - rejestracja, 2 - lekarz POZ, itd.)
    Pacjent pacjent; // Dane pacjenta
} Komunikat;


#endif