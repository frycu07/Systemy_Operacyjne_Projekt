#include "kolejka.h"
#include <unistd.h>
#include "rejestracja.h"
#include "pacjent.h"
#include "procesy.h"

void cleanup_lekarz(int kolejka_id) {
    if (msgctl(kolejka_id, IPC_RMID, NULL) == -1) {
        perror("[LEKARZ][CLEANUP] Błąd usuwania kolejki wiadomości");
    } else {
        printf("[LEKARZ][CLEANUP] Kolejka wiadomości (ID: %d) została usunięta.\n", kolejka_id);
    }
}
void signal_handler_lekarz(int sig) {
    printf("[LEKARZ][SIGNAL_HANDLER] Otrzymano sygnał %d w procesie PID: %d\n", sig, getpid());
    printf("[LEKARZ][SIGNAL_HANDLER] Lekarz kończy pracę.\n");

    _exit(0); // Natychmiastowe zakończenie procesu
}

void lekarz_poz(int id, int limit_pacjentow) {
    signal(SIGTERM, signal_handler_lekarz);
    log_process("START", "Lekarz_POZ", id); // Logowanie rozpoczęcia pracy lekarza POZ
    printf("Lekarz POZ %d: Rozpoczęto pracę. Limit pacjentów: %d, PID: %d\n", id, limit_pacjentow, getpid());


    int kolejka = msgget(KOLEJKA_POZ, IPC_CREAT | 0666);
    if (kolejka == -1) {
        perror("Błąd otwierania kolejki POZ");
        exit(1);
    }

    int pacjenci_obsluzeni = 0;

    while (pacjenci_obsluzeni < limit_pacjentow) {
        Komunikat komunikat;

        if (pacjenci_obsluzeni >= limit_pacjentow) {
            printf("Lekarz POZ %d: Limit pacjentów osiągnięty. Kończę pracę.\n", id);
            break;
        }
        if (msgrcv(kolejka, &komunikat, sizeof(Pacjent), 0, 0) != -1) {
            printf("KROK 6 Lekarz POZ %d: Obsługuję pacjenta ID: %d%s\n",
                   id, komunikat.pacjent.id,
                   komunikat.pacjent.rodzic_obecny ? " (z rodzicem)" : "");

            log_process("OBSŁUGA", "Lekarz_POZ", komunikat.pacjent.id); // Log obsługi pacjenta

            sleep(2); // Symulacja czasu obsługi pacjenta
            pacjenci_obsluzeni++;

            printf("KROK 7 Lekarz POZ %d: Zakończono obsługę pacjenta ID: %d%s (obsłużono: %d/%d)\n",
                   id, komunikat.pacjent.id,
                   komunikat.pacjent.rodzic_obecny ? " (z rodzicem)" : "",
                   pacjenci_obsluzeni, limit_pacjentow);

            log_process("ZAKOŃCZONO", "Lekarz_POZ", komunikat.pacjent.id); // Log zakończenia obsługi
           // zakoncz_wizyte(komunikat.pacjent); // Zakończenie wizyty dla pacjenta (uwzględnia rodzica)
        } else {
            sleep(1); // Czekanie na pacjentów
        }
    }

    printf("Lekarz POZ %d: Osiągnięto limit pacjentów (%d/%d). Kończę pracę.\n",
           id, pacjenci_obsluzeni, limit_pacjentow);
    cleanup_lekarz(kolejka);
    log_process("END", "Lekarz_POZ", id); // Log zakończenia pracy lekarza POZ
    exit(0);
}

void lekarz_specjalista(int typ_kolejki, int limit_pacjentow) {
    log_process("START", "Lekarz_Specjalista", typ_kolejki);
    printf("Lekarz Specjalista  %d: Rozpoczęto pracę. Limit pacjentów: %d, PID: %d\n", typ_kolejki, limit_pacjentow, getpid());
    int kolejka = msgget(typ_kolejki, IPC_CREAT | 0666);
    if (kolejka == -1) {
        perror("Błąd otwierania kolejki specjalisty");
        exit(1);
    }

    int pacjenci_obsluzeni = 0;

    while (pacjenci_obsluzeni < limit_pacjentow) {
        Komunikat komunikat;

        if (msgrcv(kolejka, &komunikat, sizeof(Pacjent), 0, 0) != -1) {
            printf("KROK 6 Lekarz specjalista (typ kolejki: %d): Obsługuję pacjenta ID: %d%s%s\n",
                   typ_kolejki,
                   komunikat.pacjent.id,
                   komunikat.pacjent.priorytet ? " (VIP)" : "",
                   komunikat.pacjent.rodzic_obecny ? " (z rodzicem)" : "");

            log_process("OBSŁUGA", "Lekarz_Specjalista", komunikat.pacjent.id);

            sleep(2); // Symulacja czasu obsługi pacjenta
            pacjenci_obsluzeni++;

            printf("KROK 7 Lekarz specjalista (typ kolejki: %d): Zakończono obsługę pacjenta ID: %d%s%s (obsłużono: %d/%d)\n",
                   typ_kolejki,
                   komunikat.pacjent.id,
                   komunikat.pacjent.priorytet ? " (VIP)" : "",
                   komunikat.pacjent.rodzic_obecny ? " (z rodzicem)" : "",
                   pacjenci_obsluzeni,
                   limit_pacjentow);

            log_process("ZAKOŃCZONO", "Lekarz_Specjalista", komunikat.pacjent.id);
            //zakoncz_wizyte(komunikat.pacjent);
        } else {
            sleep(1);
        }
    }

    log_process("END", "Lekarz_Specjalista", typ_kolejki);
    exit(0);
}

int sprawdz_kolejke(int kolejka_id) {
    struct msqid_ds statystyki;

    // Pobranie informacji o kolejce
    if (msgctl(kolejka_id, IPC_STAT, &statystyki) == -1) {
        perror("Błąd pobierania informacji o kolejce");
        exit(1);
    }

    // Zwrócenie liczby wiadomości w kolejce
    return (int)statystyki.msg_qnum;
}

