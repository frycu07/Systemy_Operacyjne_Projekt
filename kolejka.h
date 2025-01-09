#ifndef KOLEJKI_H
#define KOLEJKI_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define KOLEJKA_ZEWNETRZNA 1230 // Kolejka zewnętrzna

// Limity pacjentów dla lekarzy
#define X1 10 // Limit dla każdego lekarza POZ
#define X2 5  // Limit dla kardiologa
#define X3 5  // Limit dla okulisty
#define X4 5  // Limit dla pediatry
#define X5 5  // Limit dla lekarza medycyny pracy

#define KOLEJKA_POZ 1235           // Kolejka dla lekarzy POZ
#define KOLEJKA_KARDIOLOG 1236     // Kolejka dla kardiologa
#define KOLEJKA_OKULISTA 1237      // Kolejka dla okulisty
#define KOLEJKA_PEDIATRA 1238      // Kolejka dla pediatry
#define KOLEJKA_MEDYCYNA_PRACY 1239 // Kolejka dla lekarza medycyny pracy

// Maksymalna liczba osób w przychodni
#define MAX_OSOB_W_PRZYCHODNI 5

// Globalna zmienna liczby osób
extern int *liczba_osob;


// Klucze do kolejek
#define KOLEJKA_REJESTRACJA 1240

// Funkcje związane z semaforami
void inicjalizuj_semafor();
void usun_semafor();
void semafor_op(int delta);
void sprawdz_wartosc_semafora();
void wyswietl_liczbe_osob();
void inicjalizuj_semafor_liczba_osob();

void zablokuj_semafor();
void odblokuj_semafor();

void loguj_liczba_osob(const char* akcja);

void usun_semafor_liczba_osob();

#endif