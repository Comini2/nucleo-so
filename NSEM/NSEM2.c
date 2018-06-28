#include "c:\nucleo\NSEM\NSEM2.H"
#include <string.h>


typedef struct registros {
	unsigned int bx1, es1;
} regis;

typedef union k {
	regis x;
	char far *y;
} APONTA_REG_CRIT;

APONTA_REG_CRIT a;

/* variaveis globais */
    PTR_DESC_PROC prim = NULL; /* processo atual */
    PTR_DESC d_esc; /* descritor do escalador */
/* fim das variaveis globais */

void far inicia_semaforo(semaforo *sema,int n){
    sema->s = n;
    sema->q = NULL;
}

void far insere_fila_q(semaforo *sema){
    /* insere o processo bloqueado pela primitiva P na lista de processos bloqueados */
    PTR_DESC_PROC aux; /*descritor auxiliar */
    aux = sema->q;

    if(aux == NULL)
        sema->q = prim;
    else{
        while(aux->fila_sem != NULL)
            aux = aux->fila_sem;
        aux->fila_sem = prim;
        prim->fila_sem = NULL;
    }
}

void far remove_fila_semaforo(semaforo *sema){
    /*remove o processo da fila de semaforos */
    PTR_DESC_PROC aux;

    aux = sema->q;
    aux->estado = ativo;
    sema->q = aux->fila_sem;
}

PTR_DESC_PROC far procura_prox_ativo(){
    PTR_DESC_PROC temp = prim;

    while(1){
        temp = temp->prox_desc;
        if(temp->estado == ativo)
            return temp;
        if(temp == prim && temp->estado == terminado)
            return NULL;
    }
}

void far volta_dos() {
	disable();
	setvect(8, p_est->int_anterior);
	enable();
	exit(0);
}

void far p(semaforo *sema){
    PTR_DESC_PROC aux;

    disable();
    if(sema->s > 0) /* processo continua ativo */
        sema->s--;
    else{
        /* bloqueia o processo */
        insere_fila_q(sema);
        prim->estado = bloqueado; /* bloequeia o prim */
        aux = prim;

        if((prim = procura_prox_ativo()) == NULL){
            volta_dos();
        }
        transfer(aux->contexto,prim->contexto);
    }
    enable();
}

void far v(semaforo *sema){
    disable();
    if(sema->q != NULL){
        remove_fila_semaforo(sema);
    }
    else{
        sema->s++;
    }
    enable();
}

void far insere_final(PTR_DESC_PROC proc){
    if(!prim){
        prim = proc;
        prim->prox_desc = proc;
    }else{
        PTR_DESC_PROC temp = prim;

        while(temp->prox_desc != prim)
            temp = temp->prox_desc;

        temp->prox_desc = proc;
        proc->prox_desc = prim;
    }
}

void far cria_processo(void far (*end_proc)(), char *nome_proc){

    PTR_DESC_PROC aux = (PTR_DESC_PROC)malloc(sizeof(DESCRITOR_PROC));

    strcpy(aux->nome, nome_proc); /* define o nome do processo */
    aux->estado = ativo; /* define como ativo o processo criado */
    aux->contexto = cria_desc(); /* cria um descritor do processo */
    aux->fila_sem = NULL; /* variavel nova criada para os semaforos0 */

    newprocess(end_proc, aux->contexto);
    insere_final(aux); /* insere o processor ao final da fila */
}

void far escalador(){
    p_est->p_origem = d_esc; /* define como origem o descritor do escalador */
    p_est->p_destino = prim->contexto; /* o destino é o processo que está em PRIM */
    p_est->num_vetor = 8; /* interrupcao padrao */

    _AH = 0x34;
	_AL = 0x00;
	geninterrupt(0x21);
	a.x.bx1 = _BX;
	a.x.es1 = _ES;

    while(1){
        iotransfer();
        disable();
        if (*a.y == 0) {
            if((prim = procura_prox_ativo()) == NULL) volta_dos();
            p_est->p_destino = prim->contexto;
        }
        enable();
    }
}

void far dispara_sistema(){
    PTR_DESC desc_dispara = cria_desc(); /*cria um descritor auxiliar */
    d_esc = cria_desc(); /* cria um descritor para o escalador */
    newprocess(escalador, d_esc); /*executa o processor que está escalado */
    transfer(desc_dispara, d_esc); /* transfere o controle para o escalador */
}

void far termina_processo(){
    disable();
    prim->estado = terminado;
    enable();
    while(1);
}
