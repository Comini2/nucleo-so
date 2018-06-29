/*
TESTES PARA O NUCLEO DE TROCA DE MSG
*/

#include <stdio.h>
#include "C:/NUCLEO/NMSG/NMSG.h"
#include <limits.h>

void far processo1() {
    int i, status;
    for(i = 0; i<10; i++){
        /*
            envio a msg até que o processo "proc2" a receba
        */
        while((status = envia_msg("proc2", "ola")) != 2){
            if(!status)
                printf("\nReceptor não existe!");
            if(status)
                printf("\nFila do receptor cheia!");
        }
        printf("\nMensagem enviada!");

    }
    termina_processo();
}

void far processo2() {
    int i;
    char *buffer, *emissor;

    for(i = 0; i<10; i++){
        /*
        "proc2" irá receber e mostrar
        */
        recebe_msg(emissor, buffer);
        printf("\nRecebido: \"%s\" do processo %s.", buffer, emissor);
    }
    termina_processo();
}

int main() {
    cria_processo(processo1, "proc1", 10);
    cria_processo(processo2, "proc2", 10);

    dispara_sistema();
}
