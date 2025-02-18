#include "rejestracja.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "czas.c"
#include "pacjent.h"
#include "kolejka.h"
#include "kolejka.c"
#include "procesy.h"
#include "procesy.c"
#include "czas.h"
#include "lekarz.h"

#include <pthread.h>
#include <signal.h>
#include <sys/errno.h>
#include <sys/shm.h>

pthread_t kolejka_thread;
volatile bool czy_dziala = true;
int drugie_okienko_pid;
int end_rejestracja = 0;
int pacjenci_w_kolejce_POZ = 0;
int pacjenci_w_kolejce_KARDIOLOG = 0;
int pacjenci_w_kolejce_OKULISTA = 0;
int pacjenci_w_kolejce_PEDIATRA = 0;
int pacjenci_w_kolejce_MEDYCYNA_PRACY = 0;
int *suma_kolejek;
int shm_id;       // ID pamięci współdzielonej
int *liczba_osob; // Wskaźnik do pamięci współdzielonej
int shm_id2;

int id_sm_kolejki_lekarzy[5];
int *sm_kolejki_lekarzy[5];
int semafor_kolejki_lekarzy[5];
void create_osoby_do_przyjecia_lekarz(int id)
{
    key_t shm_key_base = 4321;  // Bazowy klucz dla pamięci współdzielonej
    key_t sem_key_base = 4322;  // Bazowy klucz dla semaforów

    for (int i = 0; i < 5; i++) {
        // Tworzenie klucza pamięci współdzielonej dla każdego lekarza
        key_t shm_keyx = shm_key_base + i;
        id_sm_kolejki_lekarzy[i] = shmget(shm_keyx, sizeof(int), IPC_CREAT | 0666);
        if (id_sm_kolejki_lekarzy[i] == -1) {
            perror("Błąd tworzenia pamięci współdzielonej");
            exit(1);
        }

        // Podłączamy pamięć współdzieloną i ZOSTAWIAMY ją podłączoną:
        sm_kolejki_lekarzy[i] = (int *)shmat(id_sm_kolejki_lekarzy[i], NULL, 0);
        if (sm_kolejki_lekarzy[i] == (void *)-1) {
            perror("Błąd dołączania pamięci współdzielonej");
            exit(1);
        }

        // Inicjalizacja licznika TYLKO w pierwszym okienku (jeśli tak zakładasz):
        if (id == 0) {
            *sm_kolejki_lekarzy[i] = 0;
        }

        // Tworzenie semafora dla każdego lekarza
        key_t sem_keyx = sem_key_base + i;
        semafor_kolejki_lekarzy[i] = semget(sem_keyx, 1, IPC_CREAT | 0666);
        if (semafor_kolejki_lekarzy[i] == -1) {
            perror("Błąd tworzenia semafora");
            exit(1);
        }

        // Inicjalizacja semafora tylko w pierwszym okienku, by nie nadpisać istniejącej wartości
        if (id == 0) {
            if (semctl(semafor_kolejki_lekarzy[i], 0, SETVAL, 1) == -1) {
                perror("Błąd inicjalizacji semafora");
                exit(1);
            }
        }

        printf("Utworzono pamięć współdzieloną i semafor dla lekarza %d\n", i);
    }
}
// void create_osoby_do_przyjecia_lekarz(int id) {
//     key_t shm_key_base = 4321;  // Bazowy klucz dla pamięci współdzielonej
//     key_t sem_key_base = 4322; // Bazowy klucz dla semaforów
//
//     for (int i = 0; i < 5; i++) {
//         // Tworzenie klucza pamięci współdzielonej dla każdego lekarza
//         key_t shm_keyx = shm_key_base + i;
//         id_sm_kolejki_lekarzy[i] = shmget(shm_keyx, sizeof(int), IPC_CREAT | 0666);
//         if (id_sm_kolejki_lekarzy[i] == -1) {
//             perror("Błąd tworzenia pamięci współdzielonej");
//             exit(1);
//         }
//
//         // Inicjalizacja pamięci współdzielonej
//         int *shared_mem = (int *)shmat(id_sm_kolejki_lekarzy[i], NULL, 0);
//         if (shared_mem == (void *)-1) {
//             perror("Błąd dołączania pamięci współdzielonej");
//             exit(1);
//         }
//         if(id==0) *shared_mem = 0; // Inicjalizacja liczby pacjentów na 0
//         shmdt(shared_mem); // Odłączanie pamięci współdzielonej
//
//         sm_kolejki_lekarzy[i] = shared_mem;
//         // Tworzenie semaforów dla każdego lekarza
//         key_t sem_keyx = sem_key_base + i;
//         semafor_kolejki_lekarzy[i] = semget(sem_keyx, 1, IPC_CREAT | 0666);
//         if (semafor_kolejki_lekarzy[i] == -1) {
//             perror("Błąd tworzenia semafora");
//             exit(1);
//         }
//
//         if (id ==0) {
//             // Inicjalizacja semafora
//             if (semctl(semafor_kolejki_lekarzy[i], 0, SETVAL, 1) == -1) {
//                 perror("Błąd inicjalizacji semafora");
//                 exit(1);
//             }
//         }
//
//         printf("Utworzono pamięć współdzieloną i semafor dla lekarza %d\n", i);
//     }
// }
void remove_osoby_do_przyjecia_lekarz() {
    for (int i = 0; i < 5; i++) {
        // Usuwanie pamięci współdzielonej
        if (shmctl(id_sm_kolejki_lekarzy[i], IPC_RMID, NULL) == -1) {
            perror("Błąd usuwania pamięci współdzielonej");
        } else {
            printf("Usunięto pamięć współdzieloną dla lekarza %d\n", i);
        }

        // Usuwanie semafora
        if (semctl(semafor_kolejki_lekarzy[i], 0, IPC_RMID) == -1) {
            perror("Błąd usuwania semafora");
        } else {
            printf("Usunięto semafor dla lekarza %d\n", i);
        }
    }
}

void blokuj_osoby_do_przyjecia_lekarz(int typ_lekarza) {
    printf("Zablokowany semafor dla %d\n", typ_lekarza);
    zmniejsz_semafor(semafor_kolejki_lekarzy[typ_lekarza]);
}
void odblokuj_osoby_do_przyjecia_lekarz(int typ_lekarza) {
    printf("Odblokowany semafor dla %d\n", typ_lekarza);
    zwieksz_semafor(semafor_kolejki_lekarzy[typ_lekarza]);
}
void dodaj_pacjenta_do_kolejki(int typ_lekarza) {
    (*sm_kolejki_lekarzy[typ_lekarza])++;
}
int ilosc_pacjentow_w_kolejce(int typ_lekarza) {
    return *sm_kolejki_lekarzy[typ_lekarza];
}

void* uruchom_zarzadzanie_kolejka(void* arg) {
        zarzadz_kolejka_zewnetrzna();
    return NULL;
}

void zakonczenie_procesow() {
    pid_t pid;
    while ((pid = waitpid(-1, NULL, WNOHANG)) > 0) {
        printf("Proces o PID %d został zakończony.\n", pid);
    }
}

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
void pamiec_wspoldzielona2() {
    shm_id2 = shmget(PAMIEC_WSPOLDZIELONA_KLUCZ2, sizeof(int), IPC_CREAT | 0666);
    if (shm_id2 == -1) {
        perror("Błąd tworzenia pamięci współdzielonej");
        exit(1);
    }

    suma_kolejek = (int *)shmat(shm_id2, NULL, 0);
    if (suma_kolejek == (void *)-1) {
        perror("Błąd dołączania pamięci współdzielonej");
        exit(1);
    }
}

void rejestracja_end() {
    end_rejestracja = 1;
}

void rejestracja(int id) {
    if (id == 0) {
        signal(SIGINT,remove_osoby_do_przyjecia_lekarz);
        signal(SIGTERM,remove_osoby_do_przyjecia_lekarz);
    }
    signal(SIGUSR1,rejestracja_end);
    create_osoby_do_przyjecia_lekarz(id);
    //log_process("START", "Rejestracja", id);  // Logowanie rozpoczęcia rejestracji
    printf("Uruchomiono kolejke rejestracja %d\n", id);

   int kolejka_rejestracja = msgget(KOLEJKA_REJESTRACJA, IPC_CREAT | 0666);
if (kolejka_rejestracja == -1) {
    perror("[ERROR] Nie udało się utworzyć kolejki KOLEJKA_REJESTRACJA");
} else {
    printf("[DEBUG] Utworzono kolejkę KOLEJKA_REJESTRACJA z ID: %d\n", kolejka_rejestracja);
}

int kolejka_poz = msgget(KOLEJKA_POZ, IPC_CREAT | 0666);
if (kolejka_poz == -1) {
    perror("[ERROR] Nie udało się utworzyć kolejki KOLEJKA_POZ");
} else {
    printf("[DEBUG] Utworzono kolejkę KOLEJKA_POZ z ID: %d\n", kolejka_poz);
}

int kolejka_kardiolog = msgget(KOLEJKA_KARDIOLOG, IPC_CREAT | 0666);
if (kolejka_kardiolog == -1) {
    perror("[ERROR] Nie udało się utworzyć kolejki KOLEJKA_KARDIOLOG");
} else {
    printf("[DEBUG] Utworzono kolejkę KOLEJKA_KARDIOLOG z ID: %d\n", kolejka_kardiolog);
}

int kolejka_okulista = msgget(KOLEJKA_OKULISTA, IPC_CREAT | 0666);
if (kolejka_okulista == -1) {
    perror("[ERROR] Nie udało się utworzyć kolejki KOLEJKA_OKULISTA");
} else {
    printf("[DEBUG] Utworzono kolejkę KOLEJKA_OKULISTA z ID: %d\n", kolejka_okulista);
}

int kolejka_pediatra = msgget(KOLEJKA_PEDIATRA, IPC_CREAT | 0666);
if (kolejka_pediatra == -1) {
    perror("[ERROR] Nie udało się utworzyć kolejki KOLEJKA_PEDIATRA");
} else {
    printf("[DEBUG] Utworzono kolejkę KOLEJKA_PEDIATRA z ID: %d\n", kolejka_pediatra);
}

int kolejka_medycyna_pracy = msgget(KOLEJKA_MEDYCYNA_PRACY, IPC_CREAT | 0666);
if (kolejka_medycyna_pracy == -1) {
    perror("[ERROR] Nie udało się utworzyć kolejki KOLEJKA_MEDYCYNA_PRACY");
} else {
    printf("[DEBUG] Utworzono kolejkę KOLEJKA_MEDYCYNA_PRACY z ID: %d\n", kolejka_medycyna_pracy);
}

int kolejka_vip_poz = msgget(KOLEJKA_VIP_POZ, IPC_CREAT | 0666);
if (kolejka_vip_poz == -1) {
    perror("[ERROR] Nie udało się utworzyć kolejki KOLEJKA_VIP_POZ");
} else {
    printf("[DEBUG] Utworzono kolejkę KOLEJKA_VIP_POZ z ID: %d\n", kolejka_vip_poz);
}

int kolejka_vip_kardiolog = msgget(KOLEJKA_VIP_KARDIOLOG, IPC_CREAT | 0666);
if (kolejka_vip_kardiolog == -1) {
    perror("[ERROR] Nie udało się utworzyć kolejki KOLEJKA_VIP_KARDIOLOG");
} else {
    printf("[DEBUG] Utworzono kolejkę KOLEJKA_VIP_KARDIOLOG z ID: %d\n", kolejka_vip_kardiolog);
}

int kolejka_vip_okulista = msgget(KOLEJKA_VIP_OKULISTA, IPC_CREAT | 0666);
if (kolejka_vip_okulista == -1) {
    perror("[ERROR] Nie udało się utworzyć kolejki KOLEJKA_VIP_OKULISTA");
} else {
    printf("[DEBUG] Utworzono kolejkę KOLEJKA_VIP_OKULISTA z ID: %d\n", kolejka_vip_okulista);
}

int kolejka_vip_pediatra = msgget(KOLEJKA_VIP_PEDIATRA, IPC_CREAT | 0666);
if (kolejka_vip_pediatra == -1) {
    perror("[ERROR] Nie udało się utworzyć kolejki KOLEJKA_VIP_PEDIATRA");
} else {
    printf("[DEBUG] Utworzono kolejkę KOLEJKA_VIP_PEDIATRA z ID: %d\n", kolejka_vip_pediatra);
}

int kolejka_vip_medycyna_pracy = msgget(KOLEJKA_VIP_MEDYCYNA_PRACY, IPC_CREAT | 0666);
if (kolejka_vip_medycyna_pracy == -1) {
    perror("[ERROR] Nie udało się utworzyć kolejki KOLEJKA_VIP_MEDYCYNA_PRACY");
} else {
    printf("[DEBUG] Utworzono kolejkę KOLEJKA_VIP_MEDYCYNA_PRACY z ID: %d\n", kolejka_vip_medycyna_pracy);
}
    uzyskaj_pamiec_wspoldzielona();

    int semafor_rejestracja = uzyskaj_dostep_do_semafora(klucz_semafora_rejestracja);
    int semafor_suma_kolejek = uzyskaj_dostep_do_semafora(klucz_semafor_suma_kolejek);

    printf("[DEBUG] Proces %d używa kolejki: %d, semafora: %d\n", getpid(), kolejka_rejestracja, semafor_rejestracja);


    while (1) {
        if (end_rejestracja) break;
        Komunikat komunikat2;


        zmniejsz_semafor(semafor_rejestracja);

        // Odbiór pacjenta z kolejki rejestracji
        if (msgrcv(kolejka_rejestracja, &komunikat2, sizeof(Pacjent), 0, 0) != -1) {
            printf("KROK 5 [R] Rejestracja %d: Odebrano pacjenta ID: %d\n", id, komunikat2.pacjent.id);
        }
        else {
            perror("Błąd odbierania pacjenta z kolejki rejestracji");
            continue;
        }

        zwieksz_semafor(semafor_rejestracja);

        if (!czy_przychodnia_otwarta() && sprawdz_kolejke(kolejka_rejestracja)  == 0) {
            int lek_num = komunikat2.pacjent.lekarz;
            char lek_nazw[20];
            switch (komunikat2.pacjent.lekarz) {
                case 0: // POZ
                    strcpy(lek_nazw, "POZ");
                break;
                case 1: // Kardiolog
                    strcpy(lek_nazw, "KARDIOLOG");
                break;
                case 2: // Okulista
                    strcpy(lek_nazw, "OKULISTA");
                break;
                case 3: // Pediatra
                    strcpy(lek_nazw, "PEDIATRA");
                break;
                case 4: // Medycyna pracy
                    strcpy(lek_nazw, "MEDYCYNA PRACY");
                break;
                default:
                    strcpy(lek_nazw, "NIEZNANY");
                break;
            }
                RaportPacjenta raport = {komunikat2.pacjent.id, lek_nazw, "REJESTRACJA"};
                zapisz_do_raportu(raport);
                zakoncz_wizyte(komunikat2.pacjent);
                break;
            }

            sleep(2);
            printf("KROK 6 ");
            blokuj_osoby_do_przyjecia_lekarz(komunikat2.pacjent.lekarz);
            printf ("ILOSC PACJENTOW W KOLEJCE: %d, lekarz: %d\n", ilosc_pacjentow_w_kolejce(komunikat2.pacjent.lekarz), komunikat2.pacjent.lekarz);
            // Skierowanie pacjenta do odpowiedniej kolejki
            switch (komunikat2.pacjent.lekarz) {
                case 0: // POZ
                {
                    if (ilosc_pacjentow_w_kolejce(komunikat2.pacjent.lekarz)< X1+X1) {
                        dodaj_pacjenta_do_kolejki(komunikat2.pacjent.lekarz);
                        dodaj_suma_kolejek();
                        int kolejka_docelowa = komunikat2.pacjent.priorytet ? kolejka_vip_poz : kolejka_poz;
                        printf("[DEBUG] Kolejka docelowa (POZ): %d, Pacjenci przeszli przez kolejke: %d\n", kolejka_docelowa, pacjenci_w_kolejce_POZ);
                        printf("[DEBUG] Wysyłam wiadomość do kolejki: msqid=%d, msgsz=%lu, mtype=%ld\n",
                         kolejka_docelowa, sizeof(Pacjent), komunikat2.typ);
                        if (msgsnd(kolejka_docelowa, &komunikat2, sizeof(Pacjent), 0) == -1) {
                            perror("[ERROR] Nie udało się wysłać wiadomości do kolejki POZ");
                        } else {
                            printf("[DEBUG] Pacjent ID: %d pomyślnie dodany do kolejki POZ.\n", komunikat2.pacjent.id);
                        }
                    } else {
                        printf("Rejestracja %d: Limit pacjentów POZ osiągnięty. Pacjent ID: %d nie może zostać skierowany.\n", id, komunikat2.pacjent.id);
                        RaportPacjenta raport = {komunikat2.pacjent.id, "POZ", "REJESTRACJA"};
                        zapisz_do_raportu(raport);
                        zakoncz_wizyte(komunikat2.pacjent);
                    }
                    break;
                }
                case 1: // Kardiolog
                {
                    if (ilosc_pacjentow_w_kolejce(komunikat2.pacjent.lekarz) < X2) {
                        dodaj_pacjenta_do_kolejki(komunikat2.pacjent.lekarz);
                        dodaj_suma_kolejek();
                        int kolejka_docelowa = komunikat2.pacjent.priorytet ? kolejka_vip_kardiolog : kolejka_kardiolog;
                        printf("pacjenci_w_kolejce_KARDIOLOG: %d\n", pacjenci_w_kolejce_KARDIOLOG);
                        printf("[DEBUG] Wysyłam wiadomość do kolejki: msqid=%d, msgsz=%lu, mtype=%ld\n",
                         kolejka_docelowa, sizeof(Pacjent), komunikat2.typ);
                        if (msgsnd(kolejka_docelowa, &komunikat2, sizeof(Pacjent), 0) == -1) {
                            perror("[ERROR] Nie udało się wysłać wiadomości do kolejki KARDIOLOG");
                        } else {
                            printf("[DEBUG] Pacjent ID: %d pomyślnie dodany do kolejki KARDIOLOG.\n", komunikat2.pacjent.id);
                        }
                    } else {
                        printf("Rejestracja %d: Limit pacjentów KARDIOLOG osiągnięty. Pacjent ID: %d nie może zostać skierowany.\n", id, komunikat2.pacjent.id);
                       // log_process("ODMOWA", "Kardiolog", komunikat2.pacjent.id);
                        RaportPacjenta raport = {komunikat2.pacjent.id, "KARDIOLOG", "REJESTRACJA"};
                        zapisz_do_raportu(raport);
                        zakoncz_wizyte(komunikat2.pacjent);
                    }
                    break;
                }
                case 2: // Okulista
                {
                    if (ilosc_pacjentow_w_kolejce(komunikat2.pacjent.lekarz) < X3) {
                        dodaj_pacjenta_do_kolejki(komunikat2.pacjent.lekarz);
                        dodaj_suma_kolejek();
                        int kolejka_docelowa = komunikat2.pacjent.priorytet ? kolejka_vip_okulista : kolejka_okulista;
                        printf("pacjenci_w_kolejce_OKULISTA: %d\n", pacjenci_w_kolejce_OKULISTA);
                        printf("[DEBUG] Wysyłam wiadomość do kolejki: msqid=%d, msgsz=%lu, mtype=%ld\n",
                         kolejka_docelowa, sizeof(Pacjent), komunikat2.typ);
                        msgsnd(kolejka_docelowa, &komunikat2, sizeof(Pacjent), 0);
                        printf("Rejestracja %d: Pacjent ID: %d skierowany do kolejki OKULISTA.\n", id, komunikat2.pacjent.id);
                       // log_process("SKIEROWANO", "Okulista", komunikat2.pacjent.id);
                    } else {
                        printf("Rejestracja %d: Limit pacjentów OKULISTA osiągnięty. Pacjent ID: %d nie może zostać skierowany.\n", id, komunikat2.pacjent.id);
                        //log_process("ODMOWA", "Okulista", komunikat2.pacjent.id);
                        RaportPacjenta raport = {komunikat2.pacjent.id, "OKULISTA", "REJESTRACJA"};
                        zapisz_do_raportu(raport);
                        zakoncz_wizyte(komunikat2.pacjent);
                    }
                    break;
                }
                case 3: // Pediatra
                {
                    if (ilosc_pacjentow_w_kolejce(komunikat2.pacjent.lekarz) < X4) {
                        dodaj_pacjenta_do_kolejki(komunikat2.pacjent.lekarz);
                        dodaj_suma_kolejek();
                        int kolejka_docelowa = komunikat2.pacjent.priorytet ? kolejka_vip_pediatra : kolejka_pediatra;
                        printf("pacjenci_w_kolejce_PEDIATRA: %d\n", pacjenci_w_kolejce_PEDIATRA);
                        printf("[DEBUG] Wysyłam wiadomość do kolejki: msqid=%d, msgsz=%lu, mtype=%ld\n",
                         kolejka_docelowa, sizeof(Pacjent), komunikat2.typ);
                        msgsnd(kolejka_docelowa, &komunikat2, sizeof(Pacjent), 0);
                        printf("Rejestracja %d: Pacjent ID: %d skierowany do kolejki PEDIATRA.\n", id, komunikat2.pacjent.id);
                       // log_process("SKIEROWANO", "Pediatra", komunikat2.pacjent.id);
                    } else {
                        printf("Rejestracja %d: Limit pacjentów PEDIATRA osiągnięty. Pacjent ID: %d nie może zostać skierowany.\n", id, komunikat2.pacjent.id);
                       // log_process("ODMOWA", "Pediatra", komunikat2.pacjent.id);
                        RaportPacjenta raport = {komunikat2.pacjent.id, "PEDIATRA", "REJESTRACJA"};
                        zapisz_do_raportu(raport);
                        zakoncz_wizyte(komunikat2.pacjent);
                    }
                    break;
                }
                case 4: // Medycyna pracy
                {
                    if (ilosc_pacjentow_w_kolejce(komunikat2.pacjent.lekarz) < X5) {
                        dodaj_pacjenta_do_kolejki(komunikat2.pacjent.lekarz);
                        dodaj_suma_kolejek();
                        int kolejka_docelowa = komunikat2.pacjent.priorytet ? kolejka_vip_medycyna_pracy : kolejka_medycyna_pracy;
                        printf("Pacjenci_w_kolejce_MEDYCYNA_PRACY: %d\n", pacjenci_w_kolejce_MEDYCYNA_PRACY);
                        printf("[DEBUG] Wysyłam wiadomość do kolejki: msqid=%d, msgsz=%lu, mtype=%ld\n",
                         kolejka_docelowa, sizeof(Pacjent), komunikat2.typ);
                        msgsnd(kolejka_docelowa, &komunikat2, sizeof(Pacjent), 0);
                        printf("Rejestracja %d: Pacjent ID: %d skierowany do kolejki MEDYCYNA PRACY.\n", id, komunikat2.pacjent.id);
                       // log_process("SKIEROWANO", "Medycyna_Pracy", komunikat2.pacjent.id);
                    } else {
                        printf("Rejestracja %d: Limit pacjentów MEDYCYNA PRACY osiągnięty. Pacjent ID: %d nie może zostać skierowany.\n", id, komunikat2.pacjent.id);
                        //log_process("ODMOWA", "Medycyna_Pracy", komunikat2.pacjent.id);
                        RaportPacjenta raport = {komunikat2.pacjent.id, "MEDYCYNA PRACY", "REJESTRACJA"};
                        zapisz_do_raportu(raport);
                        zakoncz_wizyte(komunikat2.pacjent);
                    }
                    break;
                }
            }
        odblokuj_osoby_do_przyjecia_lekarz(komunikat2.pacjent.lekarz);
    }

    if(id == 0) {
        remove_osoby_do_przyjecia_lekarz();
    }
}



void zarzadz_kolejka_zewnetrzna() {
    Komunikat komunikat2;
    int kolejka_zewnetrzna = msgget(KOLEJKA_ZEWNETRZNA, IPC_CREAT | 0666);
    int kolejka_rejestracja = msgget(KOLEJKA_REJESTRACJA, IPC_CREAT | 0666);
    //printf("uruchomiono kolejke zewnetrzna\n");
    if (kolejka_zewnetrzna == -1 || kolejka_rejestracja == -1) {
        perror("Błąd otwierania kolejek");
        exit(1);
    }
    uzyskaj_dostep_do_semafora(klucz_liczba_osob);
    uzyskaj_pamiec_wspoldzielona();
    uzyskaj_dostep_do_semafora(klucz_semafor_suma_kolejek);
    pamiec_wspoldzielona2();
    int liczba_osob_zarejestrowanych = 0;
    int suma = X1+X1+X2+X3+X4+X5;
    while (czy_dziala) {
        komunikat2.typ = 1;

        if (msgrcv(kolejka_zewnetrzna, &komunikat2, sizeof(Pacjent), 1, 0) != -1) {
            printf("KROK 3 Odebrano wiadomość w kolejce zewnetrznej. Pacjent ID: %d\n", komunikat2.pacjent.id);
        } else {
            printf("[DEBUG] msgrcv zwrócił błąd\n");
            if (errno == ENOMSG) {
                printf("[INFO] Brak wiadomości w kolejce\n");
            } else {
                perror("[ERROR] msgrcv niepowodzenie");
                printf("[DEBUG] Wartość errno: %d\n", errno);
            }
        }
        if (czy_przychodnia_otwarta() == 1) {
            if ((suma < *suma_kolejek) && (liczba_osob_zarejestrowanych != 0)) {
                printf("Lekarze maja pelne kolejki suma: %d MAX:%d KROK 3' ZKZ Przychodnia zamknięta. Pacjent ID: %d nie może wejść. Wszyscy lekarze maja pelne kolejki\n", suma, *suma_kolejek, komunikat2.pacjent.id);
                czy_dziala = 0;
                continue;
            }
            int wymagane_miejsce = komunikat2.pacjent.rodzic_obecny ? 2 : 1;
            //printf("[DEBUG] liczba_osob:%d , wymagane_miejsce: %d, MAX_OSOB_W_PRZYCHODNI: %d\n",
            //*liczba_osob, wymagane_miejsce, MAX_OSOB_W_PRZYCHODNI);
            if (*liczba_osob + wymagane_miejsce <= MAX_OSOB_W_PRZYCHODNI) {
                // Tworzenie procesu rodzica, jeśli obecny
                if (komunikat2.pacjent.rodzic_obecny) {
                    if (fork() == 0) {
                        printf("Stworzono Rodzic dla pacjenta %d, PID: %d\n", komunikat2.pacjent.id, getpid());
                        pause(); // Rodzic czeka z dzieckiem. Proces tworzony jako rejestracja
                    }
                }

                // Wysyłanie pacjenta do kolejki rejestracyjnej

                komunikat2.typ = 1;

                if (msgsnd(kolejka_rejestracja, &komunikat2, sizeof(Pacjent), 0 == -1)) {
                    printf("Błąd wysyłania wiadomości\n");
                }

                printf("KROK 4 Pacjent ID: %d%s%s został wpuszczony do przychodni z kolejki zewnętrznej.\n",
                       komunikat2.pacjent.id,
                       komunikat2.pacjent.priorytet ? " (VIP)" : "",
                       komunikat2.pacjent.rodzic_obecny ? " (z rodzicem)" : "");
                printf("Wymagane miejsca: %d\n", wymagane_miejsce);
                zmien_liczba_osob(wymagane_miejsce); // Zmiana liczby osób w przychodni
                liczba_osob_zarejestrowanych++;

            } else {
                // Kolejka zewnętrzna czeka, jeśli brak miejsca w przychodni
                msgsnd(kolejka_zewnetrzna, &komunikat2, sizeof(Pacjent), 0);
                printf("KROK 4' Pacjent ID: %d%s%s musi poczekać na wejście do przychodni.\n",
                        komunikat2.pacjent.id,
                        komunikat2.pacjent.priorytet ? " (VIP)" : "",
                        komunikat2.pacjent.rodzic_obecny ? " (z rodzicem)" : "");
                sleep(2);
            }
        }
        else {
            printf("[DEBUG] Wszedłem do bloku: przychodnia zamknięta.\n");
            printf("KROK 3' Pacjent ID: %d nie może wejść - przychodnia zamknięta.\n", komunikat2.pacjent.id );
            sleep(2); // Oczekiwanie przed ponowną próbą
    }


    }
         // Krótka przerwa przed następną iteracją
         sleep(1);
        }


void zarzadz_i_monitoruj_rejestracje() {
    //printf("Monitorowanie sie zaczelo\n");
    // Uruchomienie procesu rejestracji (okienko 0)
    //printf("[DEBUG] Proces główny PID: %d\n", getpid());
    sleep(1);
    pid_t pid = fork();
    if (pid == -1) {
        perror("[ERROR] Nie udało się stworzyć procesu dziecka dla rejestracja(0)");
    } else if (pid == 0) {
        printf("[DEBUG] Proces dziecka uruchomiony, PID: %d\n", getpid());
        rejestracja(0);
    } else {
        printf("[DEBUG] Proces rodzica, PID dziecka: %d\n", pid);
    }
        struct msqid_ds statystyki;

        // Flaga określająca, czy drugie okienko jest aktywne
        bool drugie_okienko_aktywne = false;

        // Zmienna przechowująca PID drugiego okienka
        pid_t drugie_okienko_pid = -1;
        int semafor_rejestracja = uzyskaj_dostep_do_semafora(klucz_semafora_rejestracja);
        int kolejka_rejestracja = msgget(KOLEJKA_REJESTRACJA, IPC_CREAT | 0666);

        if (kolejka_rejestracja == -1) {
            perror("[Monitorowanie] Błąd dostępu do kolejki rejestracji");
            exit(1);
        }
    if (msgctl(kolejka_rejestracja, IPC_STAT, &statystyki) == -1) {
        perror("[Monitorowanie] Błąd pobierania statystyk kolejki");
        exit(0); // Pomijaj iterację, jeśli nie można pobrać statystyk
    }
        while (1) {
            // Pobierz aktualne statystyki kolejki rejestracji
            if (msgctl(kolejka_rejestracja, IPC_STAT, &statystyki) == -1) {
                perror("[Monitorowanie] Błąd pobierania statystyk kolejki");
                continue; // Pomijaj iterację, jeśli nie można pobrać statystyk
            }
            // Liczba pacjentów w kolejce
            long liczba_pacjentow = statystyki.msg_qnum;

            // Debug: Wyświetl aktualną liczbę pacjentów
            printf("[Monitorowanie][ZIMR] Aktualna liczba pacjentów w kolejce rejestracja: %ld\n", liczba_pacjentow);
            sleep(1);
            // Jeżeli liczba pacjentów przekracza MAX_OSOB_W_PRZYCHODNI / 2, otwórz drugie okienko
            if ((liczba_pacjentow > MAX_OSOB_W_PRZYCHODNI / 2) && !drugie_okienko_aktywne) {

                drugie_okienko_pid = fork();
                if (drugie_okienko_pid < 0) {
                    perror("[ERROR] Nie udało się stworzyć procesu dziecka dla rejestracja(1)");
                    exit(2);
                } else if (drugie_okienko_pid == 0) {
                    printf("[DEBUG] Proces dziecka uruchomiony dla drugiego okienka, PID: %d\n", getpid());
                    rejestracja(1);
                    exit(0);
                } else {
                    printf("[DEBUG] Proces rodzica, PID dziecka drugei okienko: %d\n", drugie_okienko_pid);

                    drugie_okienko_aktywne = true;
                }
            }

            // Jeżeli liczba pacjentów spadnie poniżej MAX_OSOB_W_PRZYCHODNI / 3, zamknij drugie okienko
            if ((liczba_pacjentow < MAX_OSOB_W_PRZYCHODNI / 3) && drugie_okienko_aktywne) {
                zmniejsz_semafor(semafor_rejestracja); // Blokowanie dostępu
                //int semafor_zamkniecie = pobierz_wartosc_semafora(klucz_semafor_zamkniecie);
                //zmniejsz_semafor(semafor_zamkniecie);
                printf("Drugie okienko rejestracji (PID: %d) zostało zamknięte.\n", drugie_okienko_pid);
                if (drugie_okienko_pid > 0) {
                    if (kill(drugie_okienko_pid, SIGUSR1) == -1) {
                        perror("[REJESTRACJA][ERROR] Nie udało się zakończyć procesu drugiego okienka");
                    }
                    zwieksz_semafor(semafor_rejestracja);
                }
                drugie_okienko_aktywne = false;
                }
            }

            sleep(1); // Odczekaj przed kolejną kontrolą
        }

//
void zapisz_do_raportu(RaportPacjenta pacjent) {
    FILE *plik = fopen("rapodrt_zienny.txt", "a"); // Otwórz plik w trybie dopisywania
    if (plik == NULL) {
        perror("Nie można otworzyć pliku raportu dziennego");
        exit(1);
    }

    // Zapisz informacje do pliku
    fprintf(plik, "ID Pacjenta: %d\n", pacjent.id);
    fprintf(plik, "Skierowanie do: %s\n", pacjent.skierowanie_do);
    fprintf(plik, "Wystawił: %s\n", pacjent.wystawil);
    fprintf(plik, "------------------------\n");

    fclose(plik); // Zamknij plik
}

int main() {


    printf("[REJESTRACJA][START] PROGRAM rejestracja uruchomiony. PID: %d\n", getpid());

    int semafor_rejestracja = uzyskaj_dostep_do_semafora(klucz_semafora_rejestracja);
    int semafor_liczba_osob; uzyskaj_dostep_do_semafora(klucz_liczba_osob);

    //Utworzenie wątku dla zarzadz_kolejka_zewnetrzna
    if (pthread_create(&kolejka_thread, NULL, uruchom_zarzadzanie_kolejka, NULL) != 0) {
        perror("[REJESTRACJA][ERROR] Nie udało się utworzyć wątku dla zarzadz_kolejka_zewnetrzna");
        exit(1);
    }

    //Uruchomienie rejestracja() w głównym wątku
    zarzadz_i_monitoruj_rejestracje();

    //Oczekiwanie na zakończenie wątku zarzadz_kolejka_zewnetrzna
    pthread_cancel(kolejka_thread);
    pthread_join(kolejka_thread, NULL);

    return 0;
}