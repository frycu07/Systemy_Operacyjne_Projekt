#ifndef PROCESY_H
#define PROCESY_H

const char* current_time_string();
void log_process(const char* status, const char* type, int id);
void zakonczenie_poprzednich_procesow();

typedef struct {
    int kolejka_rejestracja;
    int semafor_rejestracja;
    int max_osob_w_przychodni;
} ArgumentyRejestracja;

#endif