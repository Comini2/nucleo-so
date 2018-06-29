#include "c:\nucleo\nbsc\NBSC.H"
#include <string.h>

/* Núcleo básico com Round Robin. */

/********* DECLARAÇÃO DO DESCRITOR DE PROCESSO (BCP) *********/

typedef struct desc_p {
    char nome[35];
    enum{ativo, terminado} estado;
    PTR_DESC contexto;
    struct desc_p *prox_desc;
} DESCRITOR_PROC;

typedef DESCRITOR_PROC *PTR_DESC_PROC;

/*************************************************************/

/********* PONTEIROS DA REGIÃO CRÍTICA DO DOS *********/

typedef struct registros {
	unsigned int bx1, es1;
} regis;

typedef union k {
	regis x;
	char far *y;
} APONTA_REG_CRIT;

APONTA_REG_CRIT a;

/******************************************************/


/**************************** VARIÁVEIS GLOBAIS ****************************/

/* Ponteiro para a fila circular de processos. */
PTR_DESC_PROC prim = NULL;

/* Ponteiro para o descritor do escalador. */
PTR_DESC d_esc;

/***************************************************************************/

/******************************* ROTINAS DO S.O. *******************************/

/************ ROTINAS UTILIZADAS PELO ESCALADOR DE PROCESSOS ************/

void far escalador();

PTR_DESC_PROC far procura_prox_ativo();

void far volta_dos();

/******************* ROTINAS UTILIZADAS PELO USUÁRIO ********************/

void far cria_processo(void far (*end_proc)(), char *nome_proc);

void far termina_processo();

void far dispara_sistema();

/**************** ROTINAS REFERENTE A FILA CIRCULAR DE PROCESSOS ***************/

void far insere_final(PTR_DESC_PROC proc);

/*************************** FIM DAS ROTINAS DO S.O. ***************************/


/************************** IMPLEMENTAÇÃO DAS ROTINAS **************************/

/* Rotina responsável por inserir um processo no final da fila circular. */
void far insere_final(PTR_DESC_PROC proc){
    if(!prim){  /*Se a fila não foi iniciada ainda, faça este processo como a cabeça da fila. */
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

/* Implementação da co-rotina do escalador, o controle da fila de prioridades é feito pela rotina procura_prox_proc_ativo. */
void far escalador(){
    p_est->p_origem = d_esc;                    /* ------------------------------------------- */
    p_est->p_destino = prim->contexto;          /* Inicializando o vetor de interrupção do DOS */
    p_est->num_vetor = 8;                       /* ------------------------------------------- */

    _AH = 0x34;                                 /* --------------------------------------------------- */
	_AL = 0x00;                                 
	geninterrupt(0x21);                         /* Inicializando os ponteiros da região crítica do DOS */
	a.x.bx1 = _BX;
	a.x.es1 = _ES;                              /* --------------------------------------------------- */

    while(1){
        iotransfer();
        disable();
        if (!*a.y) { /*Se o processo não está na região crítica do DOS, troca o processo, senão continua a execução. */
            if((prim = procura_prox_ativo()) == NULL) volta_dos();
            p_est->p_destino = prim->contexto;
        }
        enable();
    }
}

/* Rotina responsável por procurar um processo ativo, utilizando o algoritmo de Round Robin. */
PTR_DESC_PROC far procura_prox_ativo(){ 
    PTR_DESC_PROC temp = prim;
    while(1){
        temp = temp->prox_desc;
        if(temp->estado == ativo)
            return temp;
        if(temp == prim && temp->estado == terminado)   /*Se demos a volta na fila circular e este ja está terminado, */
            return NULL;                                /*acabaram os processos, então retorna NULL.*/
    }
}

/* Rotina responsável por devolver o controle ao DOS */
void far volta_dos() {
	disable();
	setvect(8, p_est->int_anterior);
	enable();
	exit(0);
}

/* Rotina responsável por criar o descritor do processo e inseri-lo na fila circular. */
void far cria_processo(void far (*end_proc)(), char *nome_proc){
    PTR_DESC_PROC aux = (PTR_DESC_PROC)malloc(sizeof(DESCRITOR_PROC));
    strcpy(aux->nome, nome_proc);
    aux->estado = ativo;
    aux->contexto = cria_desc();
    newprocess(end_proc, aux->contexto);
    insere_final(aux);
}

/* Rotina responsável por disparar o escalador de processos */
void far dispara_sistema(){
    PTR_DESC desc_dispara = cria_desc();    /* Ponteiro para o descritor dessa rotina */
    d_esc = cria_desc();                    /* Cria o ponteiro do descritor do escalador */
    newprocess(escalador, d_esc);
    transfer(desc_dispara, d_esc);          /* Transfere o controle do núcleo para o escalador de processos. */
}

/* Rotina reponsável por terminar o processo corrente, deve ser chamada pelo usuário */
void far termina_processo(){
    disable();
    prim->estado = terminado;
    enable();
    while(1);
}
