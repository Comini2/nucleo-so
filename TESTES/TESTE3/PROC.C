#include <stdio.h>
#include "c:\nucleo\NSEM\NSEM2.H"
#include <limits.h>
/*

TESTES PARA O NSEM ==> SEMAFORO

*/
semaforo mutex;

void far processo1(){
    int i,j;

    /* estou entrando em uma regiao critica */
    p(&mutex);
    printf("processo 1 iniciado\n");
    /* OS 2 FORes REPRESENTAM A REGIÃO CRÍTICA */
    for(i = 0; i < INT_MAX; i++)
        for(j = 0; j < 50; j++);
    printf("processo 1 terminado\n\n");
    v(&mutex);
    /* saí da região crítica */
    termina_processo();
}

void far processo2(){
    int i,j;

    /* entrando em uma regiao critica */
    p(&mutex);
    printf("processo 2 iniciado\n");
    /* OS 2 FOR REPRESENTAM A REGIÃO CRÍTICA */
    for(i = 0; i < INT_MAX; i++)
        for(j = 0; j < 3; j++);

    printf("processo 2 terminado\n\n");
    v(&mutex);
    /* saindo da região critica */

    termina_processo();
}

void far processo3(){
    int i;

    /* entrando em uma regiao critica */
    p(&mutex);
    printf("processo 3 iniciado\n");
    /* O 1 FOR REPRESENTAM A REGIÃO CRÍTICA */
    for(i = 0; i < INT_MAX; i++);
    printf("processo 3 terminado\n\n");
    v(&mutex);
    /*saindo de uma regiao critica */

    termina_processo();
}

int main(){
    /* PERMITE-SE >>"2"<< PROCESSOS NA REGIÃO CRÍTICA SIMULTANEAMENTE */
    inicia_semaforo(&mutex,2);

    cria_processo(processo1,"Processo 1");
    cria_processo(processo2,"Processo 2");
    cria_processo(processo3,"Processo 3");

    /*
    neste exemplo, o processo 3 será bloqueado, pois é permitido que apenas 2 processos
    fiquem na regiao critica
    */

    dispara_sistema();
}
