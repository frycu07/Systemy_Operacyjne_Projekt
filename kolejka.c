#include <stdio.h>
#include "kolejka.h"

void wyczysc_kolejki() {
    int a = 0;
    printf("wyczysc kolejki dziala");
    int kolejki[] = {KOLEJKA_ZEWNETRZNA, KOLEJKA_REJESTRACJA, KOLEJKA_POZ, KOLEJKA_KARDIOLOG, KOLEJKA_OKULISTA, KOLEJKA_PEDIATRA, KOLEJKA_MEDYCYNA_PRACY};
    for (int i = 0; i < sizeof(kolejki) / sizeof(kolejki[0]); i++) {
        int msg_id = msgget(kolejki[i], IPC_CREAT | 0666);
        if (msg_id != -1) {
            if (msgctl(msg_id, IPC_RMID, NULL) == -1) {
                perror("[ERROR] Nie udało się usunąć kolejki");
                printf("[DEBUG] ID kolejki: %d, klucz: 0x%x\n", msg_id, kolejki[i]);
            } else {
                if (a)printf("[DEBUG] Kolejka o ID: %d (klucz: 0x%x) została usunięta\n", msg_id, kolejki[i]);
            }
        } else {
            perror("[ERROR] Nie udało się otworzyć kolejki");
            printf("[DEBUG] Klucz: 0x%x\n", kolejki[i]);
        }
    }
}