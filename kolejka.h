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
#define X1 20 // Limit dla każdego lekarza POZ
#define X2 5  // Limit dla kardiologa
#define X3 5  // Limit dla okulisty
#define X4 5  // Limit dla pediatry
#define X5 5  // Limit dla lekarza medycyny pracy

#define KOLEJKA_POZ 1235           // Kolejka dla lekarzy POZ
#define KOLEJKA_KARDIOLOG 1236     // Kolejka dla kardiologa
#define KOLEJKA_OKULISTA 1237      // Kolejka dla okulisty
#define KOLEJKA_PEDIATRA 1238      // Kolejka dla pediatry
#define KOLEJKA_MEDYCYNA_PRACY 1239 // Kolejka dla lekarza medycyny pracy

#define KOLEJKA_BADAŃ_BASE 2000 // Podstawowy numer dla kolejek badań ambulatoryjnych

#define KOLEJKA_BADAŃ_KARDIOLOG (KOLEJKA_KARDIOLOG + KOLEJKA_BADAŃ_BASE) // Kolejka badań dla kardiologa
#define KOLEJKA_BADAŃ_OKULISTA (KOLEJKA_OKULISTA + KOLEJKA_BADAŃ_BASE)   // Kolejka badań dla okulisty
#define KOLEJKA_BADAŃ_PEDIATRA (KOLEJKA_PEDIATRA + KOLEJKA_BADAŃ_BASE)   // Kolejka badań dla pediatry
#define KOLEJKA_BADAŃ_MEDYCYNA_PRACY (KOLEJKA_MEDYCYNA_PRACY + KOLEJKA_BADAŃ_BASE) // Kolejka dla medycyny pracy


void wyczysc_kolejki();

// Maksymalna liczba osób w przychodni
#define MAX_OSOB_W_PRZYCHODNI 30

// Globalna zmienna liczby osób
extern int *liczba_osob;

// Klucze do kolejek
#define KOLEJKA_REJESTRACJA 1240

// Funkcje związane z semaforami

void sprawdz_wartosc_semafora();
void stworz_semafor_liczba_osob(key_t klucz);
void usun_semafor_liczba_osob() ;
void zablokuj_semafor() ;
void odblokuj_semafor() ;

int stworz_semafor(key_t klucz);
void ustaw_wartosc_semafora(int semafor, int wartosc);
int pobierz_wartosc_semafora(int semafor);
void zwieksz_semafor(int semafor);
void zmniejsz_semafor(int semafor);
void usun_semafor(int semafor);


#endif