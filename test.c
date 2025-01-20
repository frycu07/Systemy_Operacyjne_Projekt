#include "main.c"
#include "pacjent.c"

int main() {
    shmctl(shm_id, IPC_RMID, NULL); // Usuwanie pamięci współdzielonej
    semctl(semafor_rejestracja, 0, IPC_RMID); // Usuwanie semafora rejestracji
    wyczysc_kolejki(); // Usuwanie kolejek
    printf("Wszystkie zasoby zostały wyczyszczone.\n");
    exit(0);
}
