#include "kolejka.h"
#include <unistd.h>
#include <time.h>
#include "rejestracja.h"
#include "czas.h"
#include "pacjent.h"
#include "procesy.h"

void rejestracja() {
    log_process("START", "Rejestracja", 0);  // Logowanie rozpoczęcia rejestracji

    int kolejka_rejestracja = msgget(KOLEJKA_REJESTRACJA, IPC_CREAT | 0666);
    int kolejka_poz = msgget(KOLEJKA_POZ, IPC_CREAT | 0666);
    int kolejka_kardiolog = msgget(KOLEJKA_KARDIOLOG, IPC_CREAT | 0666);
    int kolejka_okulista = msgget(KOLEJKA_OKULISTA, IPC_CREAT | 0666);
    int kolejka_pediatra = msgget(KOLEJKA_PEDIATRA, IPC_CREAT | 0666);
    int kolejka_medycyna_pracy = msgget(KOLEJKA_MEDYCYNA_PRACY, IPC_CREAT | 0666);

    if (kolejka_rejestracja == -1 || kolejka_poz == -1 || kolejka_kardiolog == -1 ||
        kolejka_okulista == -1 || kolejka_pediatra == -1 || kolejka_medycyna_pracy == -1) {
        perror("Błąd tworzenia kolejek");
        exit(1);
    }

    while (1) {
        Komunikat komunikat;
        if (msgrcv(kolejka_rejestracja, &komunikat, sizeof(Pacjent), 0, 0) != -1) {
            printf("Rejestracja: Odebrano pacjenta ID: %d\n", komunikat.pacjent.id);

            // Logowanie odbioru pacjenta w rejestracji
            log_process("ODEBRANO", "Rejestracja", komunikat.pacjent.id);

            // Skierowanie pacjenta do odpowiedniej kolejki
            switch (komunikat.pacjent.lekarz) {
                case 0: // POZ
                    msgsnd(kolejka_poz, &komunikat, sizeof(Pacjent), 0);
                    printf("Rejestracja: Pacjent ID: %d skierowany do kolejki POZ.\n", komunikat.pacjent.id);
                    log_process("SKIEROWANO", "POZ", komunikat.pacjent.id);
                    break;
                case 1: // Kardiolog
                    msgsnd(kolejka_kardiolog, &komunikat, sizeof(Pacjent), 0);
                    printf("Rejestracja: Pacjent ID: %d skierowany do kardiologa.\n", komunikat.pacjent.id);
                    log_process("SKIEROWANO", "Kardiolog", komunikat.pacjent.id);
                    break;
                case 2: // Okulista
                    msgsnd(kolejka_okulista, &komunikat, sizeof(Pacjent), 0);
                    printf("Rejestracja: Pacjent ID: %d skierowany do okulisty.\n", komunikat.pacjent.id);
                    log_process("SKIEROWANO", "Okulista", komunikat.pacjent.id);
                    break;
                case 3: // Pediatra
                    msgsnd(kolejka_pediatra, &komunikat, sizeof(Pacjent), 0);
                    printf("Rejestracja: Pacjent ID: %d skierowany do pediatry.\n", komunikat.pacjent.id);
                    log_process("SKIEROWANO", "Pediatra", komunikat.pacjent.id);
                    break;
                case 4: // Medycyna pracy
                    msgsnd(kolejka_medycyna_pracy, &komunikat, sizeof(Pacjent), 0);
                    printf("Rejestracja: Pacjent ID: %d skierowany do lekarza medycyny pracy.\n", komunikat.pacjent.id);
                    log_process("SKIEROWANO", "Medycyna_pracy", komunikat.pacjent.id);
                    break;
            }
        }
    }


}

void zakoncz_wizyte(int id) {
    zablokuj_semafor();  // Zablokowanie semafora przed modyfikacją liczba_osob

    (*liczba_osob)--;
    printf("Pacjent ID: %d opuścił przychodnię. Liczba osób w przychodni: %d\n", id, (*liczba_osob));  // Logowanie

    odblokuj_semafor();  // Odblokowanie semafora po zakończeniu operacji
}

int aktualna_godzina() {
    time_t teraz = time(NULL);
    struct tm *czas = localtime(&teraz);
    return czas->tm_hour; // Zwraca aktualną godzinę
}

void zarzadz_kolejka_zewnetrzna() {
    int kolejka_zewnetrzna = msgget(KOLEJKA_ZEWNETRZNA, IPC_CREAT | 0666);
    int kolejka_rejestracja = msgget(KOLEJKA_REJESTRACJA, IPC_CREAT | 0666);

    if (kolejka_zewnetrzna == -1 || kolejka_rejestracja == -1) {
        perror("Błąd otwierania kolejek");
        exit(1);
    }

    while (1) {
        Komunikat komunikat;

        if (msgrcv(kolejka_zewnetrzna, &komunikat, sizeof(Pacjent), 0, 0) != -1) {
            Czas teraz = aktualny_czas();

            semafor_op(-1);
            if (porownaj_czas(teraz, czas_otwarcia) >= 0 && porownaj_czas(teraz, czas_zamkniecia) < 0 &&
                *liczba_osob < MAX_OSOB_W_PRZYCHODNI) {
                (*liczba_osob)++;
                wyswietl_liczbe_osob();
                semafor_op(1);

                komunikat.typ = komunikat.pacjent.id + 1;
                msgsnd(kolejka_rejestracja, &komunikat, sizeof(Pacjent), 0);
                printf("Pacjent ID: %d został wpuszczony do przychodni z kolejki zewnętrznej.\n",
                       komunikat.pacjent.id);
                } else {
                    semafor_op(1);
                    msgsnd(kolejka_zewnetrzna, &komunikat, sizeof(Pacjent), 0);
                    sleep(1);
                }
        }
    }
}