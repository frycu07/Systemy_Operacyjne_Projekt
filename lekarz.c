#include "kolejka.h"
#include <unistd.h>
#include "rejestracja.h"
#include "pacjent.h"
#include "procesy.h"

void lekarz_poz(int id, int limit_pacjentow) {
    log_process("START", "Lekarz_POZ", id); // Logowanie rozpoczęcia pracy lekarza POZ

    int kolejka = msgget(KOLEJKA_POZ, IPC_CREAT | 0666);
    if (kolejka == -1) {
        perror("Błąd otwierania kolejki POZ");
        exit(1);
    }

    int pacjenci_obsluzeni = 0;

    while (pacjenci_obsluzeni < limit_pacjentow) {
        Komunikat komunikat;

        if (msgrcv(kolejka, &komunikat, sizeof(Pacjent), 0, IPC_NOWAIT) != -1) {
            printf("Lekarz POZ %d: Obsługuję pacjenta ID: %d%s\n",
                   id, komunikat.pacjent.id,
                   komunikat.pacjent.rodzic_obecny ? " (z rodzicem)" : "");

            log_process("OBSŁUGA", "Lekarz_POZ", komunikat.pacjent.id); // Log obsługi pacjenta

            sleep(2); // Symulacja czasu obsługi pacjenta
            pacjenci_obsluzeni++;

            printf("Lekarz POZ %d: Zakończono obsługę pacjenta ID: %d%s (obsłużono: %d/%d)\n",
                   id, komunikat.pacjent.id,
                   komunikat.pacjent.rodzic_obecny ? " (z rodzicem)" : "",
                   pacjenci_obsluzeni, limit_pacjentow);

            log_process("ZAKOŃCZONO", "Lekarz_POZ", komunikat.pacjent.id); // Log zakończenia obsługi

            zakoncz_wizyte(komunikat.pacjent); // Zakończenie wizyty dla pacjenta (uwzględnia rodzica)
        } else {
            sleep(1); // Czekanie na pacjentów
        }
    }

    printf("Lekarz POZ %d: Osiągnięto limit pacjentów (%d/%d). Kończę pracę.\n",
           id, pacjenci_obsluzeni, limit_pacjentow);

    log_process("END", "Lekarz_POZ", id); // Log zakończenia pracy lekarza POZ
    exit(0);
}

void lekarz_specjalista(int typ_kolejki, int limit_pacjentow) {
    log_process("START", "Lekarz_Specjalista", typ_kolejki);

    int kolejka = msgget(typ_kolejki, IPC_CREAT | 0666);
    if (kolejka == -1) {
        perror("Błąd otwierania kolejki specjalisty");
        exit(1);
    }

    int pacjenci_obsluzeni = 0;

    while (pacjenci_obsluzeni < limit_pacjentow) {
        Komunikat komunikat;
        if (msgrcv(kolejka, &komunikat, sizeof(Pacjent), 0, IPC_NOWAIT) != -1) {
            printf("Lekarz specjalista (typ kolejki: %d): Obsługuję pacjenta ID: %d%s%s\n",
                   typ_kolejki,
                   komunikat.pacjent.id,
                   komunikat.pacjent.priorytet ? " (VIP)" : "",
                   komunikat.pacjent.rodzic_obecny ? " (z rodzicem)" : "");

            log_process("OBSŁUGA", "Lekarz_Specjalista", komunikat.pacjent.id);

            sleep(10); // Symulacja czasu obsługi pacjenta
            pacjenci_obsluzeni++;

            printf("Lekarz specjalista (typ kolejki: %d): Zakończono obsługę pacjenta ID: %d%s%s (obsłużono: %d/%d)\n",
                   typ_kolejki,
                   komunikat.pacjent.id,
                   komunikat.pacjent.priorytet ? " (VIP)" : "",
                   komunikat.pacjent.rodzic_obecny ? " (z rodzicem)" : "",
                   pacjenci_obsluzeni,
                   limit_pacjentow);

            log_process("ZAKOŃCZONO", "Lekarz_Specjalista", komunikat.pacjent.id);
            zakoncz_wizyte(komunikat.pacjent);
        } else {
            sleep(1);
        }
    }

    log_process("END", "Lekarz_Specjalista", typ_kolejki);
    exit(0);
}