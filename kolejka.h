#ifndef KOLEJKI_H
#define KOLEJKI_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Maksymalna liczba osób w przychodni
#define MAX_OSOB_W_PRZYCHODNI 5

// Globalna zmienna liczby osób
extern int *liczba_osob;

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

// Klucze do kolejek
#define KOLEJKA_REJESTRACJA 1234
#define KOLEJKA_POZ 1235
#define KOLEJKA_SPECJALISTA 1236

// Funkcje związane z semaforami
void inicjalizuj_semafor();
void usun_semafor();
void semafor_op(int delta);
void wyswietl_liczbe_osob();

#endif