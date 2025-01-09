#include "kolejka.h"
#include <unistd.h>
#include "rejestracja.h"
#include "pacjent.h"


void lekarz_poz(int id, int limit_pacjentow) {
    int kolejka = msgget(KOLEJKA_POZ, IPC_CREAT | 0666);
    if (kolejka == -1) {
        perror("Błąd otwierania kolejki POZ");
        exit(1);
    }

    int pacjenci_obsluzeni = 0;

    while (pacjenci_obsluzeni < limit_pacjentow) {
        Komunikat komunikat;

        if (msgrcv(kolejka, &komunikat, sizeof(Pacjent), 0, IPC_NOWAIT) != -1) {
            printf("Lekarz POZ %d: Obsługuję pacjenta ID: %d\n", id, komunikat.pacjent.id);
            sleep(2); // Symulacja czasu obsługi pacjenta
            pacjenci_obsluzeni++;
            printf("Lekarz POZ %d: Zakończono obsługę pacjenta ID: %d (obsłużono: %d/%d)\n",
                   id, komunikat.pacjent.id, pacjenci_obsluzeni, limit_pacjentow);
            zakoncz_wizyte(komunikat.pacjent.id);
        } else {
            // Kolejka pusta, lekarz czeka na pacjentów
            sleep(1);
        }
    }

    printf("Lekarz POZ %d: Osiągnięto limit pacjentów (%d/%d). Kończę pracę.\n",
           id, pacjenci_obsluzeni, limit_pacjentow);
    exit(0);
}

void lekarz_specjalista(int typ_kolejki, int limit_pacjentow) {
    int kolejka = msgget(typ_kolejki, IPC_CREAT | 0666);
    if (kolejka == -1) {
        perror("Błąd otwierania kolejki specjalisty");
        exit(1);
    }

    int licznik = 0;

    while (licznik < limit_pacjentow) {
        Komunikat komunikat;
        if (msgrcv(kolejka, &komunikat, sizeof(Pacjent), 0, 0) != -1) {
            printf("Lekarz specjalista (typ kolejki: %d): Obsługuję pacjenta ID: %d\n", typ_kolejki, komunikat.pacjent.id);
            sleep(10);
            licznik++;
            printf("Lekarz specjalista (typ kolejki: %d): Zakończono obsługę pacjenta ID: %d (obsłużono: %d/%d)\n",
                   typ_kolejki, komunikat.pacjent.id, licznik, limit_pacjentow);
            zakoncz_wizyte(komunikat.pacjent.id);
        } else {
            perror("Błąd odbierania komunikatu z kolejki specjalisty");
        }
    }

    printf("Lekarz specjalista (typ kolejki: %d): Osiągnięto limit pacjentów (%d/%d). Kończę pracę.\n",
           typ_kolejki, licznik, limit_pacjentow);
    exit(0);
}