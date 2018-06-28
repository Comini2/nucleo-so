#include <stdio.h>
#include "C:/NUCLEO/NPRIO/NPRIO.h"
#include <limits.h>

void far processo() {
    int i, j;
    for(i = 0; i<INT_MAX; i++)
        for(j = 0; j<INT_MAX; j++);
    termina_processo();
}

int main() {

    cria_processo(processo, "proc0", 0);
    cria_processo(processo, "proc1", 1);
    cria_processo(processo, "proc2", 2);
    cria_processo(processo, "proc3", 3);
    cria_processo(processo, "proc4", 0);
    dispara_sistema();
}