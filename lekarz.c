#include "kolejka.h"
#include <unistd.h>
#include "rejestracja.h"

void lekarz(int typ_kolejki) {
    int kolejka = (typ_kolejki == 0) ? KOLEJKA_POZ : KOLEJKA_SPECJALISTA;
    int kolejka_lekarz = msgget(kolejka, IPC_CREAT | 0666);

    if (kolejka_lekarz == -1) {
        perror("Błąd tworzenia kolejki lekarza");
        exit(1);
    }

    while (1) {
        Komunikat komunikat;
        if (msgrcv(kolejka_lekarz, &komunikat, sizeof(Pacjent), 0, 0) == -1) {
            perror("Błąd odbioru komunikatu u lekarza");
            continue;
        }

        printf("Lekarz: Obsługuję pacjenta ID: %d\n", komunikat.pacjent.id);
        sleep(2); // Symulacja czasu badania
        printf("Lekarz: Zakończono obsługę pacjenta ID: %d\n", komunikat.pacjent.id);

        // Pacjent kończy wizytę
        zakoncz_wizyte(komunikat.pacjent.id);
    }
}