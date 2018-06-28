#include "NPRIO.h"
#include <string.h>
#include <stdio.h>

typedef struct desc_p {
    char nome[35];
    enum{ativo, terminado} estado;
    PTR_DESC contexto;
    struct desc_p *prox_desc;
} DESCRITOR_PROC;

typedef struct registros {
	unsigned int bx1, es1;
} regis;

typedef union k {
	regis x;
	char far *y;
} APONTA_REG_CRIT;

APONTA_REG_CRIT a;

typedef DESCRITOR_PROC *PTR_DESC_PROC;

PTR_DESC_PROC *fila = NULL;

PTR_DESC_PROC proc_atual = NULL;

PTR_DESC d_esc;

static int procs_ativos = 0;

FILE* saida;

void far imprimeFila();

void far inicializa_fila(){
    int i;
    fila = (PTR_DESC_PROC*) malloc(sizeof(PTR_DESC_PROC) * MAX_PRIOR);
    for(i = 0; i < MAX_PRIOR; i++){
        fila[i] = NULL;
    }
}

void far insere_final(PTR_DESC_PROC proc, int prioridade){
    if(!fila[prioridade]){
        fila[prioridade] = proc;
    }else{
        PTR_DESC_PROC temp = fila[prioridade];

        while(temp->prox_desc != NULL)
            temp = temp->prox_desc;

        temp->prox_desc = proc;
    }
}

PTR_DESC_PROC far procura_prox_ativo(){
    static int prior_atual = MAX_PRIOR - 1;
    PTR_DESC_PROC temp = proc_atual->prox_desc;
    if(temp == NULL){
        if(--prior_atual >= 0){
            if(!fila[prior_atual] || fila[prior_atual]->estado == terminado){
                proc_atual = fila[prior_atual];
                return procura_prox_ativo();
            }
            return fila[prior_atual];
        }
        if(procs_ativos){
            prior_atual = MAX_PRIOR - 1;
            proc_atual = fila[prior_atual];
            return procura_prox_ativo();
        }
        return NULL;
    }
    if(temp->estado == ativo)
        return temp;
    
    proc_atual = temp;
    return procura_prox_ativo();
}

void far cria_processo(void far (*end_proc)(), char *nome_proc, int prioridade){
    PTR_DESC_PROC aux = (PTR_DESC_PROC)malloc(sizeof(DESCRITOR_PROC));
    if(!fila)
        inicializa_fila();
    strcpy(aux->nome, nome_proc);
    aux->estado = ativo;
    aux->contexto = cria_desc();
    aux->prox_desc = NULL;
    newprocess(end_proc, aux->contexto);
    insere_final(aux, prioridade);
    procs_ativos++;
}

void far volta_dos() {
	disable();
	setvect(8, p_est->int_anterior);
	enable();
    printf("\nFim de execução!");
	exit(0);
}

void far escalador(){
    p_est->p_origem = d_esc;
    p_est->p_destino = proc_atual->contexto;
    p_est->num_vetor = 8;

    _AH = 0x34;
	_AL = 0x00;
	geninterrupt(0x21);
	a.x.bx1 = _BX;
	a.x.es1 = _ES;

    while(1){
        iotransfer();
        disable();
        if (*a.y == 0) {
            if((proc_atual = procura_prox_ativo()) == NULL) volta_dos();
            p_est->p_destino = proc_atual->contexto;
        }
        enable();
    }
}

void far dispara_sistema(){
    PTR_DESC desc_dispara;
    int i;

    for(i = 0; i < MAX_PRIOR-1; i++){
        insere_final(fila[i+1], i);
    }

    imprimeFila();
    
    proc_atual = fila[MAX_PRIOR-1];

    desc_dispara = cria_desc();
    d_esc = cria_desc();
    newprocess(escalador, d_esc);
    transfer(desc_dispara, d_esc);
}

void far termina_processo(){
    disable();
    proc_atual->estado = terminado;
    procs_ativos--;
    enable();
    printf("\nProcesso %s terminado!", proc_atual->nome);
    while(1);
}

void far imprimeFila(){
    int i;
    for(i = 0; i < MAX_PRIOR; i++){
        PTR_DESC_PROC temp = fila[i];
        printf("Fila %d: ", i);
        while(temp != NULL){
            printf("%s -> ", temp->nome);
            temp = temp->prox_desc;
        }
        printf("\n");
    }
}