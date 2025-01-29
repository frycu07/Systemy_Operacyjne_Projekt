#ifndef KOLEJKI_H
#define KOLEJKI_H

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pacjent.h"
// Maksymalna liczba osób w przychodni
#define MAX_OSOB_W_PRZYCHODNI 20

#define PAMIEC_WSPOLDZIELONA_KLUCZ 6789
#define PAMIEC_WSPOLDZIELONA_KLUCZ2 6790

#define KOLEJKA_ZEWNETRZNA 1230 // Kolejka zewnętrzna
#define KOLEJKA_REJESTRACJA 1240

#define KOLEJKA_POZ 1235           // Kolejka dla lekarzy POZ
#define KOLEJKA_KARDIOLOG 1236     // Kolejka dla kardiologa
#define KOLEJKA_OKULISTA 1237      // Kolejka dla okulisty
#define KOLEJKA_PEDIATRA 1238      // Kolejka dla pediatry
#define KOLEJKA_MEDYCYNA_PRACY 1239 // Kolejka dla lekarza medycyny pracy

#define KOLEJKA_VIP_POZ 1335
#define KOLEJKA_VIP_KARDIOLOG 1336
#define KOLEJKA_VIP_OKULISTA 1337
#define KOLEJKA_VIP_PEDIATRA 1338
#define KOLEJKA_VIP_MEDYCYNA_PRACY 1339


#define KOLEJKA_BADANIA_BASE 1250
#define KOLEJKA_BADANIA_KARDIOLOG (KOLEJKA_KARDIOLOG + KOLEJKA_BADANIA_BASE) // Kolejka badań dla kardiologa
#define KOLEJKA_BADANIA_OKULISTA (KOLEJKA_OKULISTA + KOLEJKA_BADANIA_BASE)   // Kolejka badań dla okulisty
#define KOLEJKA_BADANIA_PEDIATRA (KOLEJKA_PEDIATRA + KOLEJKA_BADANIA_BASE)   // Kolejka badań dla pediatry
#define KOLEJKA_BADANIA_MEDYCYNA (KOLEJKA_MEDYCYNA_PRACY + KOLEJKA_BADANIA_BASE) // Kolejka dla medycyny pracy

key_t klucz_liczba_osob = 3001;
key_t klucz_semafora_rejestracja = 3233;// Unikalny klucz semafora
key_t klucz_semafor_suma_kolejek = 3235;

// // Limity pacjentów dla lekarzy
#define X1 10  // Limit dla każdego lekarza POZ
#define X2 5  // Limit dla kardiologa
#define X3 5  // Limit dla okulisty
#define X4 5  // Limit dla pediatry
#define X5 5  // Limit dla lekarza medycyny pracy




void wyczysc_kolejki();

void zakoncz_wizyte(Pacjent pacjent);

// // Globalna zmienna liczby osób
extern int *suma_kolejek;
extern int *liczba_osob;


extern int semafor_rejestracja;
extern int semafor_liczba_osob;
extern int semafor_suma_kolejek;


// // Funkcje związane z semaforami
//

int stworz_semafor(key_t klucz);
void ustaw_wartosc_semafora(int semafor, int wartosc);
int pobierz_wartosc_semafora(int semafor);
void zwieksz_semafor(int semafor);
void zmniejsz_semafor(int semafor);
void usun_semafor(int semafor);
int uzyskaj_dostep_do_semafora(key_t klucz_semafora);

void dodaj_suma_kolejek();
void zmien_liczba_osob(int zmiana);
int sprawdz_kolejke(int kolejka);

#endif