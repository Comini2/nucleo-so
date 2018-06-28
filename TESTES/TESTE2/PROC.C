#include <stdio.h>
#include <NPRIO.h>
#define QTDE 0

void far processo0() {
    int i;
    for(i = 0; i<QTDE; i++){
        printf("\nProcesso 0");
    }
    termina_processo();
}

void far processo1() {
   int i;
    for(i = 0; i<QTDE; i++){
        printf("\nProcesso 1");
    }
    termina_processo();
}

void far processo2() {
    int i;
    for(i = 0; i<QTDE; i++){
        printf("\nProcesso 2");
    }
    termina_processo();
}

void far processo3() {
   int i;
    for(i = 0; i<QTDE; i++){
        printf("\nProcesso 3");
    }
    termina_processo();
}

void far processo4() {
   int i;
    for(i = 0; i<QTDE; i++){
        printf("\nProcesso 4");
    }
    termina_processo();
}

int main() {

    cria_processo(processo0, "proc0", 0);
    cria_processo(processo1, "proc1", 1);
    cria_processo(processo2, "proc2", 2);
    cria_processo(processo3, "proc3", 3);
    cria_processo(processo4, "proc4", 0);
    dispara_sistema();
}