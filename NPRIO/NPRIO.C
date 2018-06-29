#include "C:/NUCLEO/NPRIO/NPRIO.H"
#include <string.h>
#include <stdio.h>
#include <time.h>

/* Núcleo implementado com fila de prioridades */

/********* DECLARAÇÃO DO DESCRITOR DE PROCESSO (BCP) *********/

typedef struct desc_p {
    char nome[35];
    enum{ativo, terminado} estado;
    PTR_DESC contexto;
    struct desc_p *prox_desc;
    int prioridade;
    clock_t t_criacao, t_termino;
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

/* PONTEIRO PARA A FILA DE PRIORIDADES */
PTR_DESC_PROC *fila = NULL; 

/* PONTEIRO PARA ULTIMO PROCESSO EXECUTADO */
PTR_DESC_PROC proc_atual = NULL; 

/* PONTEIRO PARA O DESCRITOR DO ESCALADOR */
PTR_DESC d_esc; 

/* VARIÁVEL PARA ARMAZENAR A PRIORIDADE ATUAL */
static int prior_atual = MAX_PRIOR - 1; 

/***************************************************************************/

/******************************* ROTINAS DO S.O. *******************************/

/************ ROTINAS UTILIZADAS PELO ESCALADOR DE PROCESSOS ************/

void far escalador();

PTR_DESC_PROC far procura_proc_ativo();

PTR_DESC_PROC far procura_prox_proc_ativo(PTR_DESC_PROC proc);

void far volta_dos();

/******************* ROTINAS UTILIZADAS PELO USUÁRIO ********************/

void far cria_processo(void far (*end_proc)(), char *nome_proc, int prioridade);

void far termina_processo();

void far dispara_sistema();

/**************** ROTINAS REFERENTE A FILA DE PRIORIDADE ***************/

void far inicializa_fila();

void far insere_final(PTR_DESC_PROC proc, int prioridade);

void far imprime_fila_prioridades();

void far imprime_processos();

/*************************** FIM DAS ROTINAS DO S.O. ***************************/

/************************** IMPLEMENTAÇÃO DAS ROTINAS **************************/



/* Rotina responsável por alocar memória para a fila de prioridades, e inicia-las com ponteiro NULL. */
void far inicializa_fila(){
    int i;
    fila = (PTR_DESC_PROC*) malloc(sizeof(PTR_DESC_PROC) * MAX_PRIOR);
    for(i = 0; i < MAX_PRIOR; i++){
        fila[i] = NULL;
    }
}

/* Rotina responsável por inserir um processo no final da fila de certa prioridade. */
void far insere_final(PTR_DESC_PROC proc, int prioridade){
    if(!fila[prioridade]){  /*Se a fila não foi iniciada ainda, faça este processo como a cabeça da fila. */
        fila[prioridade] = proc;
    }else{
        PTR_DESC_PROC temp = fila[prioridade];

        while(temp->prox_desc != NULL)
            temp = temp->prox_desc;

        temp->prox_desc = proc;
    }
}


/* Rotina responsável por imprimir a fila de prioridades (apenas para checagem). */
void far imprime_fila_prioridades(){
    int i;

    printf("\n------------- FILA DE PRIORIDADES -------------\n");
    for(i = MAX_PRIOR-1; i>=0; i--){
        PTR_DESC_PROC temp = fila[i];
        printf("\nFILA[%d]: ", i);
        while(temp != NULL){
            printf("%s -> ", temp->nome);
            temp = temp->prox_desc;
        }
        printf("NULL\n");
    }
}

/*Rotina responsável por imprimir todos os processos em suas respectivas prioridades. */
void far imprime_processos(){
    int prioridade = -1;
    PTR_DESC_PROC temp = fila[0];
        
    while(temp != NULL){
        if(temp->prioridade != prioridade){
            prioridade = temp->prioridade;
            printf("\n ------------ PROCESSOS DE PRIORIDADE %d ------------ \n", prioridade);
        }
        printf("\t%s -> TERMINADO - Levou %ld pulsos de clock.\n",temp->nome, temp->t_termino - temp->t_criacao);
        temp = temp->prox_desc;
    }
}

/* Implementação da co-rotina do escalador, o controle da fila de prioridades é feito pela rotina procura_prox_proc_ativo. */

void far escalador(){
    p_est->p_origem = d_esc;                    /* ------------------------------------------- */
    p_est->p_destino = proc_atual->contexto;    /* Inicializando o vetor de interrupção do DOS */
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
            if((proc_atual = procura_prox_proc_ativo(proc_atual)) == NULL) volta_dos();
            p_est->p_destino = proc_atual->contexto;
        }
        enable();
    }
}

/* Rotina que procura QUALQUER processo ativo, da prioridade maior para a menor.
   Só é chamada quando toda a fila de prioridades já foi executada e a prioridade
   necessita ser reiniciada. */
PTR_DESC_PROC far procura_proc_ativo(){
    prior_atual = MAX_PRIOR - 1; /* Reinicia para a maior prioridade. */
    for(; prior_atual >= 0; prior_atual--){ /* Desce por todas as prioridades à procura de um processo ativo. */
        PTR_DESC_PROC temp = fila[prior_atual];
        if(!temp)
            continue;
        do {
            if(temp->estado == ativo)       /* Se o processo estiver ativo, retorna o processo*/
                return temp;
            
            if(prior_atual < MAX_PRIOR - 1)         /* Se o prioridade da iteração atual for menor que a prioridade máxima, */
                if(temp == fila[prior_atual + 1])   /* checa se o processo atual é igual à cabeça da fila da prioridade superior, */
                    break;                          /* se for, significa que já se passou por todos os processos dessa prioridade.*/
        } while((temp = temp->prox_desc) != NULL);
    }
    return NULL;    /*Se o loop foi terminado e nenhum processo ativo foi encontrado, significa que não há processos ativos.*/
}

/* Rotina reponsável por procurar o PRÓXIMO processo ativo na fila de prioridades. */
PTR_DESC_PROC far procura_prox_proc_ativo(PTR_DESC_PROC proc){
    PTR_DESC_PROC temp = proc->prox_desc;
    if(!temp){ /* Se o prox processo for NULL, ou a fila da prioridade atual é vazia, ou a execução de uma das filas foi terminada. */
        if(--prior_atual >= 0){ /*Decrementa a prioridade atual e checa se é maior que 0. */
            if(!fila[prior_atual] || fila[prior_atual]->estado == terminado) /* Se a cabeça da fila da prioridade atual é NULL ou ja foi terminado: */
                return procura_prox_proc_ativo(fila[prior_atual]);           /* continua a procurar os processos a partir dele. */
            return fila[prior_atual];   /*Senão, retorna a cabeça da fila da prioridade atual e continua a execução a partir dele. */
        }
        return procura_proc_ativo();    /*Se a prior_atual < 0 significa que toda a fila ja foi executade, procura por qualquer processo ativo.*/
    }
    if(temp->estado == ativo) /*Se o prox processo não for NULL e for ativo, retorna este processo e continua a execução a partir dele. */
        return temp;
    
    return procura_prox_proc_ativo(temp); /*Se o prox processo estiver terminado, continua a procurar a partir dele. */
}

/* Rotina responsável por devolver o controle ao DOS */
void far volta_dos() {
	disable();
	setvect(8, p_est->int_anterior);
	enable();
    imprime_processos();
	exit(0);
}

/* Rotina responsável por criar o processo e iniciar seus campos. */
void far cria_processo(void far (*end_proc)(), char *nome_proc, int prioridade){
    PTR_DESC_PROC aux = (PTR_DESC_PROC)malloc(sizeof(DESCRITOR_PROC));
    if(!fila)   /* Se a fila ainda não foi iniciado, i.e. este é o primeiro processo a ser inserido, inicializa a fila. */
        inicializa_fila();
    strcpy(aux->nome, nome_proc);
    aux->estado = ativo;
    aux->contexto = cria_desc();    /* Cria o descritor do processo. */
    aux->prioridade = prioridade;
    aux->t_criacao = clock();       /* Salva o momento da criação do processo. */
    aux->prox_desc = NULL;
    newprocess(end_proc, aux->contexto);
    insere_final(aux, prioridade);  /* Insere o processo no final da fila de sua prioridade. */
}

/* Rotina responsável por disparar o escalador de processos */
void far dispara_sistema(){
    int i;
    PTR_DESC desc_dispara = cria_desc();    /* Ponteiro para o descritor dessa rotina */
    d_esc = cria_desc();                    /* Cria o descritor do escalador de processos */
    
    for(i = 0; i < MAX_PRIOR-1; i++){   /*Loop responsável por concatenar as filas de prioridade, */
        insere_final(fila[i+1], i);     /*insere a cabeça da fila de prioridade superior na fila  */
    }                                   /*de prioridade inferior. */

    imprime_fila_prioridades();         /* Imprime a fila de prioridades. */

    for(; prior_atual >= 0; prior_atual--)  /*Define por qual fila de prioridades deve-se começar a execução. */
        if(fila[prior_atual] != NULL)
            proc_atual = fila[prior_atual]; /*Execução começa a partir da cabeça da fila de processo não nula de maior prioridade. */
                   
    newprocess(escalador, d_esc);
    transfer(desc_dispara, d_esc);      /* Transfere o controle do núcleo para o escalador de processos. */
}

/* Rotina reponsável por terminar o processo corrente, deve ser chamada pelo usuário */
void far termina_processo(){
    disable();
    proc_atual->estado = terminado;
    proc_atual->t_termino = clock();
    enable();
    while(1);
}
