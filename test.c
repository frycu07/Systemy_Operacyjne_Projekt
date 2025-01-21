//Do rejestracja

void skieruj_na_badanie(Komunikat *komunikat, int typ_kolejki_badan) {
    printf("Rejestracja: Pacjent ID: %d skierowany na badanie ambulatoryjne\n", komunikat->pacjent.id);
    msgsnd(typ_kolejki_badan, komunikat, sizeof(Pacjent), 0);
}

//Do lekarz.c

void lekarz_specjalista(int typ_kolejki, int limit_pacjentow) {
    log_process("START", "Lekarz_Specjalista", typ_kolejki);

    int kolejka_badan = msgget(typ_kolejki + 1000, IPC_CREAT | 0666); // Dodatkowa kolejka dla badań ambulatoryjnych
    if (kolejka_badan == -1) {
        perror("Błąd otwierania kolejki specjalisty");
        exit(1);
    }
    int kolejka = msgget(typ_kolejki, IPC_CREAT | 0666);
    if (kolejka == -1) {
        perror("Błąd otwierania kolejki specjalisty");
        exit(1);
    }

    int pacjenci_obsluzeni = 0;
    bool przychodnia_otwarta = true;

    while (pacjenci_obsluzeni < limit_pacjentow || !przychodnia_otwarta) {
        Komunikat komunikat;

        // Sprawdzenie godziny zamknięcia
        przychodnia_otwarta = czy_przychodnia_otwarta();

        // Priorytet dla pacjentów wracających z badań
        if (msgrcv(kolejka_badan, &komunikat, sizeof(Pacjent), 0, IPC_NOWAIT) != -1) {
            printf("Lekarz specjalista (typ kolejki: %d): Obsługuję pacjenta ID: %d powracającego z badań\n",
                   typ_kolejki, komunikat.pacjent.id);
        } else if (msgrcv(kolejka, &komunikat, sizeof(Pacjent), 0, IPC_NOWAIT) != -1) {
            printf("Lekarz specjalista (typ kolejki: %d): Obsługuję pacjenta ID: %d\n",
                   typ_kolejki, komunikat.pacjent.id);
        } else {
            sleep(1); // Czekanie na pacjentów
            continue;
        }

        log_process("OBSŁUGA", "Lekarz_Specjalista", komunikat.pacjent.id);

        sleep(2); // Symulacja czasu obsługi pacjenta
        pacjenci_obsluzeni++;

        // Decyzja o skierowaniu na badanie ambulatoryjne (10% szansy)
        if (przychodnia_otwarta && (rand() % 10 == 0)) {
            printf("Lekarz specjalista (typ kolejki: %d): Pacjent ID: %d skierowany na badanie ambulatoryjne\n",
                   typ_kolejki, komunikat.pacjent.id);
            msgsnd(kolejka_badan, &komunikat, sizeof(Pacjent), 0);
            continue; // Pacjent wróci do gabinetu bez kolejki
        }

        printf("Lekarz specjalista (typ kolejki: %d): Zakończono obsługę pacjenta ID: %d\n",
               typ_kolejki, komunikat.pacjent.id);
        log_process("ZAKOŃCZONO", "Lekarz_Specjalista", komunikat.pacjent.id);
    }

    printf("Lekarz specjalista (typ kolejki: %d): Osiągnięto limit pacjentów (%d/%d). Kończę pracę.\n",
           typ_kolejki, pacjenci_obsluzeni, limit_pacjentow);
    log_process("END", "Lekarz_Specjalista", typ_kolejki);

    // Usuwanie kolejek
    msgctl(kolejka, IPC_RMID, NULL);
    msgctl(kolejka_badan, IPC_RMID, NULL);

    exit(0);
}


//main

if (fork() == 0) {
    dyrektor(); // Rozpoczęcie pracy dyrektora
    printf("[DYREKTOR] Proces Dyrektora uruchomiony z PID: %d\n", getpid());
    exit(0);
}
