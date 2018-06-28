#include <stdio.h>
#include "c:\nucleo\NSEM\NSEM2.H"
#include <limits.h>

semaforo mutex;

void far processo1(){
    int i,j;
    p(&mutex);
    printf("processo 1 iniciado\n");
    /* OS 2 FOR REPRESENTAM A REGIÃO CRÍTICA */
    for(i = 0; i < INT_MAX; i++)
        for(j = 0; j < 50; j++);

    printf("processo 1 terminado\n\n");
    v(&mutex);
    termina_processo();
}

void far processo2(){
    int i,j;
    p(&mutex);
    printf("processo 2 iniciado\n");
    /* OS 2 FOR REPRESENTAM A REGIÃO CRÍTICA */
    for(i = 0; i < INT_MAX; i++)
        for(j = 0; j < 3; j++);
        
    printf("processo 2 terminado\n\n");
    v(&mutex);
    termina_processo();
}

void far processo3(){
    int i;
    p(&mutex);
    printf("processo 3 iniciado\n");
    /* O 1 FOR REPRESENTAM A REGIÃO CRÍTICA */
    for(i = 0; i < INT_MAX; i++);
    printf("processo 3 terminado\n\n");
    v(&mutex);
    termina_processo();
}

int main(){
    /* PERMITE-SE 2 PROCESSOS NA REGIÃO CRÍTICA SIMULTANEAMENTE */
    inicia_semaforo(&mutex,2);

    cria_processo(processo1,"Processo 1");
    cria_processo(processo2,"Processo 2");
    cria_processo(processo3,"Processo 3");

    dispara_sistema();
}
