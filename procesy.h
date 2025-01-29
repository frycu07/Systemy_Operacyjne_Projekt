#ifndef PROCESY_H
#define PROCESY_H

const char* current_time_string();
void log_process(const char* status, const char* type, int id);
void zakonczenie_poprzednich_procesow();
void znajdz_i_zakoncz_procesy(const char *nazwa_procesu);
void wyczysc_procesy();


#endif