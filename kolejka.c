#include <stdio.h>
#include "kolejka.h"

void wyczysc_kolejki() {
    int kolejki[] = {KOLEJKA_ZEWNETRZNA, KOLEJKA_REJESTRACJA, KOLEJKA_POZ, KOLEJKA_KARDIOLOG, KOLEJKA_OKULISTA, KOLEJKA_PEDIATRA, KOLEJKA_MEDYCYNA_PRACY};
    for (int i = 0; i < sizeof(kolejki) / sizeof(kolejki[0]); i++) {
        int msg_id = msgget(kolejki[i], IPC_CREAT | 0666);
        if (msg_id != -1) {
            msgctl(msg_id, IPC_RMID, NULL);
        }
    }
}