#include "kolejka.h"
#include <unistd.h>
#include <sys/errno.h>
#include <sys/shm.h>

#include "rejestracja.h"
#include "pacjent.h"
#include "procesy.h"
#include "kolejka.c"

void signal_handler_lekarz(int sig) {
    printf("[LEKARZ][SIGNAL_HANDLER] Otrzymano sygnał %d w procesie PID: %d\n", sig, getpid());
    printf("[LEKARZ][SIGNAL_HANDLER] Lekarz kończy pracę.\n");

    _exit(0); // Natychmiastowe zakończenie procesu
}

int shm_id; // ID pamięci współdzielonej
int *liczba_osob; // Wskaźnik do pamięci współdzielonej
int *suma_kolejek;

void uzyskaj_pamiec_wspoldzielona() {
    shm_id = shmget(PAMIEC_WSPOLDZIELONA_KLUCZ, sizeof(int), 0); // Odczyt istniejącej pamięci
    if (shm_id == -1) {
        perror("[REJESTRACJA] Błąd uzyskiwania dostępu do pamięci współdzielonej");
        exit(1);
    }

    liczba_osob = (int *) shmat(shm_id, NULL, 0);
    if (liczba_osob == (void *) -1) {
        perror("[REJESTRACJA] Błąd dołączania do pamięci współdzielonej");
        exit(1);
    }
    printf("[REJESTRACJA] Połączono z pamięcią współdzieloną. liczba_osob = %d\n", *liczba_osob);
}

void badania() {
    Komunikat komunikat;

    int kolejka_badania_in = msgget(KOLEJKA_BADANIA_BASE, IPC_CREAT | 0666);
    if (kolejka_badania_in == -1) {
        perror("Błąd otwierania kolejki badań ambulatoryjnych");
        exit(1);
    }
    int kolejka_badania_kardiolog = msgget(KOLEJKA_BADANIA_KARDIOLOG, IPC_CREAT | 0666);
    if (kolejka_badania_kardiolog == -1) {
        perror("Błąd otwierania kolejki badań ambulatoryjnych KARDIOLOG");
        exit(1);
    }
    int kolejka_badania_okulista = msgget(KOLEJKA_BADANIA_OKULISTA, IPC_CREAT | 0666);
    if (kolejka_badania_okulista == -1) {
        perror("Błąd otwierania kolejki badań ambulatoryjnych OKULISTA");
        exit(1);
    }
    int kolejka_badania_pediatra = msgget(KOLEJKA_BADANIA_PEDIATRA, IPC_CREAT | 0666);
    if (kolejka_badania_pediatra == -1) {
        perror("Błąd otwierania kolejki badań ambulatoryjnych PEDIATRA");
        exit(1);
    }
    int kolejka_badania_medycyna_pracy = msgget(KOLEJKA_BADANIA_MEDYCYNA, IPC_CREAT | 0666);
    if (kolejka_badania_medycyna_pracy == -1) {
        perror("Błąd otwierania kolejki badań ambulatoryjnych MEDYCYNA PRACY");
        exit(1);
    }


    if (msgrcv(kolejka_badania_in, &komunikat, sizeof(Pacjent), 0, 0) != -1) {
        printf("[BADANIA] Odebrano pacjenta ID: %d\n", komunikat.pacjent.id);
    } else {
        perror("Błąd odbierania pacjenta z kolejki badania");
    }
    sleep(2);

    switch (komunikat.pacjent.lekarz) {
                case 1: // Kardiolog
                {
                        if (msgsnd(kolejka_badania_kardiolog, &komunikat, sizeof(Pacjent), 0) == -1) {
                            perror("[ERROR] Nie udało się wysłać wiadomości do kolejki badania KARDIOLOG");
                        } else {
                            printf("[DEBUG] Pacjent ID: %d pomyślnie dodany do kolejki badania KARDIOLOG.\n", komunikat.pacjent.id);
                        }
                    break;
                    }

                case 2: // Okulista
                {
                        if (msgsnd(kolejka_badania_okulista, &komunikat, sizeof(Pacjent), 0) == -1) {
                            perror("[ERROR] Nie udało się wysłać wiadomości do kolejki badania OKULISTA");
                        } else {
                            printf("[DEBUG] Pacjent ID: %d pomyślnie dodany do kolejki badania OKULISTA.\n", komunikat.pacjent.id);
                        }
                        break;
                }
                case 3: // Pediatra
                {
                    if (msgsnd(kolejka_badania_pediatra, &komunikat, sizeof(Pacjent), 0) == -1) {
                        perror("[ERROR] Nie udało się wysłać wiadomości do kolejki badania PEDIATRA");
                    } else {
                        printf("[DEBUG] Pacjent ID: %d pomyślnie dodany do kolejki badania PEDIATRA.\n", komunikat.pacjent.id);
                    }
                    break;
                }
                case 4: // Medycyna pracy
                {
                    if (msgsnd(kolejka_badania_medycyna_pracy, &komunikat, sizeof(Pacjent), 0) == -1) {
                        perror("[ERROR] Nie udało się wysłać wiadomości do kolejki badania MEDYCYNA PRACY");
                    } else {
                        printf("[DEBUG] Pacjent ID: %d pomyślnie dodany do kolejki badania MEDYCYNA PRACY.\n", komunikat.pacjent.id);
                    }
                    break;
                }
    }
}


void lekarz_poz(int id, int limit_pacjentow, int id_kolejka_VIP, int id_kolejka) {
    signal(SIGTERM, signal_handler_lekarz);

    //log_process("START", "Lekarz_POZ", id); // Logowanie rozpoczęcia pracy lekarza POZ
    printf("Lekarz POZ %d: Rozpoczęto pracę. Limit pacjentów: %d, PID: %d\n", id, limit_pacjentow, getpid());
    uzyskaj_pamiec_wspoldzielona();

    int kolejka = msgget(id_kolejka, IPC_CREAT | 0666);
    if (kolejka == -1) {
        perror("Błąd otwierania kolejki POZ");
        exit(1);
    }
    int kolejka_vip = msgget(id_kolejka_VIP, IPC_CREAT | 0666);
    if (kolejka_vip == -1) {
        perror("Błąd otwierania kolejki POZ VIP");
        exit(1);
    }
    int pacjenci_obsluzeni = 0;

    while (pacjenci_obsluzeni < limit_pacjentow) {
        sleep(1);
        Komunikat komunikat;

        if (msgrcv(kolejka_vip, &komunikat, sizeof(Pacjent), 0, IPC_NOWAIT) != -1) {
            printf("KROK 7 Lekarz POZ %d: Obsługuję VIP pacjenta ID: %d%s\n",
                   id, komunikat.pacjent.id,
                   komunikat.pacjent.rodzic_obecny ? " (z rodzicem)" : "");
        }
        // // Sprawdzanie zwykłej kolejki
        else if (msgrcv(kolejka, &komunikat, sizeof(Pacjent), 0, IPC_NOWAIT) != -1) {
            printf("KROK 7 Lekarz POZ %d: Obsługuję zwykłego pacjenta ID: %d%s\n",
                   id, komunikat.pacjent.id,
                   komunikat.pacjent.rodzic_obecny ? " (z rodzicem)" : "");
        } else {
            // Obsługa błędu
            if (errno == ENOMSG) {
                // Brak wiadomości w kolejce
                printf("WESZLO DO ELSE POZ: Brak wiadomości w kolejce\n");
            } else {
                // Inny błąd
                perror("Błąd podczas odbierania wiadomości");
            }
            sleep(1);
            continue;
        }

        sleep(2); // Symulacja czasu obsługi pacjenta
        pacjenci_obsluzeni++;

        printf("KROK 8 Lekarz POZ %d: Zakończono obsługę pacjenta ID: %d%s (obsłużono: %d/%d)\n",
               id, komunikat.pacjent.id,
               komunikat.pacjent.rodzic_obecny ? " (z rodzicem)" : "",
               pacjenci_obsluzeni, limit_pacjentow);
        zakoncz_wizyte(komunikat.pacjent); // Zakończenie wizyty dla pacjenta (uwzględnia rodzica)rodzica
    }

    printf("Lekarz POZ %d: Osiągnięto limit pacjentów (%d/%d). Kończę pracę.\n",
           id, pacjenci_obsluzeni, limit_pacjentow);

    exit(0);
}

void lekarz_specjalista(int typ_lekarz, int limit_pacjentow, int id_kolejka_VIP, int id_kolejka, int id_kolejka_badania) {
    //log_process("START", "Lekarz_Specjalista", typ_kolejki);
    printf("Lekarz Specjalista  %d: Rozpoczęto pracę. Limit pacjentów: %d, PID: %d\n", typ_lekarz, limit_pacjentow,
           getpid());
    int kolejka = msgget(id_kolejka, IPC_CREAT | 0666);
    if (kolejka == -1) {
        perror("Błąd otwierania kolejki specjalisty");
        exit(1);
    }
    int kolejka_VIP = msgget(id_kolejka_VIP, IPC_CREAT | 0666);
    if (kolejka_VIP == -1) {
        perror("Błąd otwierania kolejki specjalisty");
        exit(1);
    }

    int kolejka_badania_base = msgget(KOLEJKA_BADANIA_BASE, IPC_CREAT | 0666);
    if (kolejka_badania_base == -1) {
        perror("Błąd otwierania kolejki badań ambulatoryjnych");
        exit(1);
    }
    int kolejka_badania = msgget(id_kolejka_badania, IPC_CREAT | 0666);
    if (kolejka_badania == -1) {
        perror("Błąd otwierania kolejki badań ambulatoryjnych powrotnych");
        exit(1);
    }

    uzyskaj_pamiec_wspoldzielona();

    int pacjenci_obsluzeni = 0;

    while (pacjenci_obsluzeni < limit_pacjentow) {
        Komunikat komunikat;
        if (msgrcv(kolejka_VIP, &komunikat, sizeof(Pacjent), 0, IPC_NOWAIT) != -1) {
            printf("Wrocil z badan\n");
        }
        else if (msgrcv(kolejka_VIP, &komunikat, sizeof(Pacjent), 0, IPC_NOWAIT) != -1);
        else if (msgrcv(kolejka, &komunikat, sizeof(Pacjent), 0, IPC_NOWAIT) != -1);
        else {
            // Obsługa błędu
            if (errno == ENOMSG) {
                // Brak wiadomości w kolejce
                printf("WESZLO DO ELSE SPEC: Brak wiadomości w kolejce\n");
            } else {
                // Inny błąd
                perror("Błąd podczas odbierania wiadomości");
            }
            sleep(1);
            continue;
        }

        printf("KROK 7 Lekarz specjalista (typ kolejki: %d): Obsługuję pacjenta ID: %d%s%s\n",
               kolejka_VIP,
               komunikat.pacjent.id,
               komunikat.pacjent.priorytet ? " (VIP)" : "",
               komunikat.pacjent.rodzic_obecny ? " (z rodzicem)" : "");

        sleep(2); // Symulacja czasu obsługi pacjenta


        double losowa_liczba = (double) rand() / RAND_MAX; // Liczba w zakresie 0.0 - 1.0
        if (losowa_liczba < 0.1) {
            // Jeśli liczba jest poniżej 0.1 (10%)
            printf("Pacjent ID: %d został skierowany na badania ambulatoryjne.\n", komunikat.pacjent.id);
            // Wysyłanie pacjenta do kolejki badań ambulatoryjnych
            if (msgsnd(kolejka_badania, &komunikat, sizeof(Pacjent), 0) == -1) {
                perror("Błąd wysyłania do kolejki badań ambulatoryjnych");
            }
        }
        else {
            pacjenci_obsluzeni++;
        }
        printf(
            "KROK 8 Lekarz specjalista (typ kolejki: %d): Zakończono obsługę pacjenta ID: %d%s%s (obsłużono: %d/%d)\n",
            typ_lekarz,
            komunikat.pacjent.id,
            komunikat.pacjent.priorytet ? " (VIP)" : "",
            komunikat.pacjent.rodzic_obecny ? " (z rodzicem)" : "",
            pacjenci_obsluzeni,
            limit_pacjentow);
        if (losowa_liczba >= 0.1) {
            zakoncz_wizyte(komunikat.pacjent);
        }
    }
    printf("Lekarz Specjalista %d: Osiągnięto limit pacjentów (%d/%d). Kończę pracę.\n",
           typ_lekarz, pacjenci_obsluzeni, limit_pacjentow);
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
    int typ = atoi(argv[1]); // Typ lekarza (0 = POZ, 1 = specjalista)
    int id_lekarza = atoi(argv[2]); // ID lekarza lub kolejki

    if (typ == 0) {
        lekarz_poz(id_lekarza, X1, KOLEJKA_VIP_POZ, KOLEJKA_POZ);
    } else if (typ == 1) {
        lekarz_specjalista(typ, X2, KOLEJKA_VIP_KARDIOLOG, KOLEJKA_KARDIOLOG, KOLEJKA_BADANIA_KARDIOLOG);
    } else if (typ == 2) {
        lekarz_specjalista(typ, X3, KOLEJKA_VIP_OKULISTA, KOLEJKA_OKULISTA, KOLEJKA_BADANIA_OKULISTA);
    } else if (typ == 3) {
        lekarz_specjalista(typ, X4, KOLEJKA_VIP_PEDIATRA, KOLEJKA_PEDIATRA, KOLEJKA_BADANIA_PEDIATRA);
    } else if (typ == 4) {
        lekarz_specjalista(typ, X5, KOLEJKA_VIP_MEDYCYNA_PRACY, KOLEJKA_MEDYCYNA_PRACY, KOLEJKA_BADANIA_MEDYCYNA);
    } else {
        fprintf(stderr, "[LEKARZ][ERROR] Nieznany typ lekarza: %d\n", typ);
        return 1;
    }

    return 0;
}
