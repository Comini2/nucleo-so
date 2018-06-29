#include <stdio.h>
#include "C:/NUCLEO/NPRIO/NPRIO.h"
#include <limits.h>

/* TESTE DE EXECUÇÃO DO NÚCLEO COM FILAS DE PRIORIDADE. */

void far processo0() {
    int i, j;
    for(i = 0; i<INT_MAX; i++)
        for(j = 0; j<50; j++);
        termina_processo();
}

void far processo1() {
    int i, j;
    for(i = 0; i<INT_MAX; i++)
        for(j = 0; j<50; j++);
        termina_processo();
}

void far processo2() {
    int i, j;
    for(i = 0; i<INT_MAX; i++)
        for(j = 0; j<50; j++);
        termina_processo();
}

void far processo3() {
    int i, j;
    for(i = 0; i<INT_MAX; i++)
        for(j = 0; j<50; j++);
        termina_processo();
}
void far processo4() {
    int i, j;
    for(i = 0; i<INT_MAX; i++)
        for(j = 0; j<50; j++);
        termina_processo();
}

void far processo5() {
    int i, j;
    for(i = 0; i<INT_MAX; i++)
        for(j = 0; j<50; j++);
        termina_processo();
}

int main() {

    cria_processo(processo0, "proc0", 0);
    cria_processo(processo1, "proc1", 1);
    cria_processo(processo2, "proc2", 2);
    cria_processo(processo3, "proc3", 3);
    cria_processo(processo4, "proc4", 0);
    cria_processo(processo5, "proc5", 3);
    dispara_sistema();
}