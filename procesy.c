#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "procesy.h"

// Funkcja do uzyskania aktualnego czasu w formacie tekstowym
const char* current_time_string() {
    static char buffer[64];
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    strftime(buffer, 64, "%Y-%m-%d %H:%M:%S", tm_info);
    return buffer;
}
//
// // Funkcja logująca informacje o procesach
// void log_process(const char* status, const char* type, int id) {
//     FILE* log = fopen("procesy.log", "a");
//     if (!log) {
//         perror("Błąd otwierania pliku logów");
//         return;
//     }
//     fprintf(log, "[%s] PID:%d Type:%s ID:%d Time:%s\n", status, getpid(), type, id, current_time_string());
//     fclose(log);
// }
void log_process(const char* status, const char* type, int id) {
    FILE *log_file = fopen("procesy.log", "a");
    if (log_file != NULL) {
        fprintf(log_file, "[%s] Type:%s ID:%d Time:%s Called from PID:%d\n",
                status, type, id, current_time_string(), getpid());
        fclose(log_file);
    }
}

void zakonczenie_poprzednich_procesow() {
    char command[100];
    sprintf(command, "pkill -f %s", "Projekt_SO"); // Zamienia "Projekt_SO" na nazwę programu
    system(command); // Wymusza zakończenie procesów
}