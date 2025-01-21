#ifndef DYREKTOR_H
#define DYREKTOR_H

#include <sys/types.h>

// Funkcje
void dyrektor(); // Główna funkcja dyrektora
void zarejestruj_pid_lekarzy(pid_t pid_poz1, pid_t pid_poz2, pid_t pid_spec[]); // Rejestracja PID lekarzy
void ewakuacja_pacjentow(); // Ewakuacja pacjentów
void obsluga_sygnalu_dyrektora(int sig); // Obsługa sygnałów dla dyrektora

#endif // DYREKTOR_H