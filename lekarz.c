#include "kolejka.h"
#include <unistd.h>
#include <sys/shm.h>

#include "rejestracja.h"
#include "pacjent.h"
#include "procesy.h"
#include "kolejka.c"

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
int shm_id;       // ID pamięci współdzielonej
int *liczba_osob; // Wskaźnik do pamięci współdzielonej

void uzyskaj_pamiec_wspoldzielona() {
    shm_id = shmget(PAMIEC_WSPOLDZIELONA_KLUCZ, sizeof(int), 0); // Odczyt istniejącej pamięci
    if (shm_id == -1) {
        perror("[REJESTRACJA] Błąd uzyskiwania dostępu do pamięci współdzielonej");
        exit(1);
    }

    liczba_osob = (int *)shmat(shm_id, NULL, 0);
    if (liczba_osob == (void *)-1) {
        perror("[REJESTRACJA] Błąd dołączania do pamięci współdzielonej");
        exit(1);
    }
    printf("[REJESTRACJA] Połączono z pamięcią współdzieloną. liczba_osob = %d\n", *liczba_osob);
}

void lekarz_poz(int id, int limit_pacjentow) {
    signal(SIGTERM, signal_handler_lekarz);

    //log_process("START", "Lekarz_POZ", id); // Logowanie rozpoczęcia pracy lekarza POZ
    printf("Lekarz POZ %d: Rozpoczęto pracę. Limit pacjentów: %d, PID: %d\n", id, limit_pacjentow, getpid());
    uzyskaj_pamiec_wspoldzielona();
    int semafor_POZ = uzyskaj_dostep_do_semafora(klucz_semafor_poz);
    int kolejka = msgget(KOLEJKA_POZ, IPC_CREAT | 0666);
    if (kolejka == -1) {
        perror("Błąd otwierania kolejki POZ");
        exit(1);
    }
    int kolejka_vip = msgget(KOLEJKA_VIP_POZ, IPC_CREAT | 0666);

    int pacjenci_obsluzeni = 0;

     while (pacjenci_obsluzeni < limit_pacjentow) {
         Komunikat komunikat;

         zmniejsz_semafor(semafor_POZ);

         if (pacjenci_obsluzeni >= limit_pacjentow) {
             printf("Lekarz POZ %d: Limit pacjentów osiągnięty. Kończę pracę.\n", id);
             break;
         }
         // Sprawdzanie kolejki VIP
         if (msgrcv(kolejka_vip, &komunikat, sizeof(Pacjent), 0, IPC_NOWAIT) != -1) {
             printf("KROK 6 Lekarz POZ %d: Obsługuję VIP pacjenta ID: %d%s\n",
                    id, komunikat.pacjent.id,
                    komunikat.pacjent.rodzic_obecny ? " (z rodzicem)" : "");
             zwieksz_semafor(semafor_POZ);
             sleep(2); // Symulacja czasu obsługi pacjenta
             pacjenci_obsluzeni++;

             printf("KROK 7 Lekarz POZ %d: Zakończono obsługę pacjenta ID: %d%s (obsłużono: %d/%d)\n",
                    id, komunikat.pacjent.id,
                    komunikat.pacjent.rodzic_obecny ? " (z rodzicem)" : "",
                    pacjenci_obsluzeni, limit_pacjentow);

             //log_process("ZAKOŃCZONO", "Lekarz_POZ", komunikat.pacjent.id); // Log zakończenia obsługi
             zakoncz_wizyte(komunikat.pacjent); // Zakończenie wizyty dla pacjenta (uwzględnia rodzica)
         }
         // Sprawdzanie zwykłej kolejki
         else if (msgrcv(kolejka, &komunikat, sizeof(Pacjent), 0, IPC_NOWAIT) != -1) {
             printf("KROK 6 Lekarz POZ %d: Obsługuję zwykłego pacjenta ID: %d%s\n",
                    id, komunikat.pacjent.id,
                    komunikat.pacjent.rodzic_obecny ? " (z rodzicem)" : "");
             zwieksz_semafor(semafor_POZ);
             sleep(2); // Symulacja czasu obsługi pacjenta
             pacjenci_obsluzeni++;

             printf("KROK 7 Lekarz POZ %d: Zakończono obsługę pacjenta ID: %d%s (obsłużono: %d/%d)\n",
                    id, komunikat.pacjent.id,
                    komunikat.pacjent.rodzic_obecny ? " (z rodzicem)" : "",
                    pacjenci_obsluzeni, limit_pacjentow);

             //log_process("ZAKOŃCZONO", "Lekarz_POZ", komunikat.pacjent.id); // Log zakończenia obsługi
             zakoncz_wizyte(komunikat.pacjent); // Zakończenie wizyty dla pacjenta (uwzględnia rodzica)
         }
         // Brak pacjentów w żadnej kolejce
         else {
             zwieksz_semafor(semafor_POZ);
             sleep(1); // Czekanie na pacjentów
             continue;
         }

             //log_process("OBSŁUGA", "Lekarz_POZ", komunikat.pacjent.id); // Log obsługi pacjenta
         }

    printf("Lekarz POZ %d: Osiągnięto limit pacjentów (%d/%d). Kończę pracę.\n",
           id, pacjenci_obsluzeni, limit_pacjentow);
    cleanup_lekarz(kolejka);
    //log_process("END", "Lekarz_POZ", id); // Log zakończenia pracy lekarza POZ
    exit(0);
}

void lekarz_specjalista(int typ_kolejki, int limit_pacjentow) {
    //log_process("START", "Lekarz_Specjalista", typ_kolejki);
    printf("Lekarz Specjalista  %d: Rozpoczęto pracę. Limit pacjentów: %d, PID: %d\n", typ_kolejki, limit_pacjentow, getpid());
    int kolejka = msgget(typ_kolejki, IPC_CREAT | 0666);
    if (kolejka == -1) {
        perror("Błąd otwierania kolejki specjalisty");
        exit(1);
    }
    uzyskaj_pamiec_wspoldzielona();

    int pacjenci_obsluzeni = 0;

    while (pacjenci_obsluzeni < limit_pacjentow) {
        Komunikat komunikat;

        if (msgrcv(kolejka, &komunikat, sizeof(Pacjent), 0, 0) != -1) {
            printf("KROK 6 Lekarz specjalista (typ kolejki: %d): Obsługuję pacjenta ID: %d%s%s\n",
                   typ_kolejki,
                   komunikat.pacjent.id,
                   komunikat.pacjent.priorytet ? " (VIP)" : "",
                   komunikat.pacjent.rodzic_obecny ? " (z rodzicem)" : "");

            //log_process("OBSŁUGA", "Lekarz_Specjalista", komunikat.pacjent.id);

            sleep(2); // Symulacja czasu obsługi pacjenta
            pacjenci_obsluzeni++;

            printf("KROK 7 Lekarz specjalista (typ kolejki: %d): Zakończono obsługę pacjenta ID: %d%s%s (obsłużono: %d/%d)\n",
                   typ_kolejki,
                   komunikat.pacjent.id,
                   komunikat.pacjent.priorytet ? " (VIP)" : "",
                   komunikat.pacjent.rodzic_obecny ? " (z rodzicem)" : "",
                   pacjenci_obsluzeni,
                   limit_pacjentow);

            //log_process("ZAKOŃCZONO", "Lekarz_Specjalista", komunikat.pacjent.id);
            zakoncz_wizyte(komunikat.pacjent);
        } else {
            sleep(1);
        }
    }

    //log_process("END", "Lekarz_Specjalista", typ_kolejki);
    exit(0);
}



int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "[LEKARZ][ERROR] Nieprawidłowa liczba argumentów. Oczekiwano 2.\n");
        fprintf(stderr, "[LEKARZ][USAGE] ./lekarz <typ> <id_kolejki>\n");
        return 1;
    }


    printf("[LEKARZ][INFO] Uruchomiono lekarza\n");

    // Konwersja argumentów
    int typ = atoi(argv[1]);       // Typ lekarza (0 = POZ, 1 = specjalista)
    int id_kolejki = atoi(argv[2]); // ID lekarza lub kolejki

    if (typ == 0) {
        lekarz_poz(id_kolejki, X1);
    } else if (typ == 1) {
        lekarz_specjalista(id_kolejki, X2 );
    } else if (typ == 2) {
        lekarz_specjalista(id_kolejki, X3 );
    } else if (typ == 3) {
        lekarz_specjalista(id_kolejki, X4 );
    } else if (typ == 4) {
        lekarz_specjalista(id_kolejki, X5 );
    } else {
        fprintf(stderr, "[LEKARZ][ERROR] Nieznany typ lekarza: %d\n", typ);
        return 1;
    }

    return 0;
}