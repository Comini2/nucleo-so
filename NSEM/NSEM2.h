#include "c:\tc\system.h"

typedef struct desc_p {
    char nome[35];
    enum{ativo, terminado, bloqueado} estado;
    PTR_DESC contexto;

    struct desc_p *fila_sem;    /* se estiver bloqueado, irá apontar para o proximo da fila de SEMAFOROS */
                                /* se não estiver bloqueado por semaforo, valera NULL */

    struct desc_p *prox_desc; /* aponta para o proximo processo */
} DESCRITOR_PROC;

typedef DESCRITOR_PROC *PTR_DESC_PROC;

typedef struct {
    int s; /* numero da parte inteira do semaforo */
    PTR_DESC_PROC q; /* a fila dos processor dos semaforos */
} semaforo;

extern void far inicia_semaforo(semaforo *sema,int n);

extern void far p(semaforo *sema);

extern void far v(semaforo *sema);

extern void far cria_processo(void far (*end_proc)(), char *nome_proc);

extern void far dispara_sistema();

extern void far termina_processo();
