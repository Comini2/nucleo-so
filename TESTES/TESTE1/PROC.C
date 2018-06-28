#include <stdio.h>
#include "C:/NUCLEO/NBSC/NBSC.h"
#include <limits.h>

void far processo1() {
    int i;
    for(i = 0; i<INT_MAX; i++){
        printf("Processo 1\n");
    }
    termina_processo();
}

void far processo2() {
   int i;
    for(i = 0; i<INT_MAX; i++){
        printf("Processo 2\n");
    }
    termina_processo();
}

int main() {
    cria_processo(processo1, "proc1");
    cria_processo(processo2, "proc2");

    dispara_sistema();
}