#include "c:\nucleo\NMSG\NMSG.H"
#include <string.h>

typedef struct tipo_msg{
    /* estrutura basica de uma mensagem */
    char emissor[35]; /*processo emissor */
    char msg[25]; /* mensagem */
    int flag; /* msg lida ou não */
    struct tipo_msg *prox_msg; /* ponteiro para a proxima mensagem */
} mensagem;

typedef mensagem *PTR_MSG;

typedef struct desc_p {
    char nome[35];
    enum{bloq_env, bloq_rec, ativo, terminado} estado;
    PTR_DESC contexto;
    struct desc_p *prox_desc;

    PTR_MSG fila_msg; /* ponteiro para a fila de msg */
    int tam_fila;   /*tamanho da fila */
    int qtde_msg;   /* quantidade de mensagens */
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

PTR_DESC_PROC prim = NULL;

PTR_DESC d_esc;

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

PTR_MSG far inicializa_fila_msg(int max_tam){
    /* cria a fila estatica */
    int i;
    PTR_MSG inicio, aux, aux2;

    max_tam = max_tam > 10 ? max_tam = 10 : max_tam;

    inicio = (PTR_MSG)malloc(sizeof(mensagem));
    inicio->flag = 0;

    aux = inicio;
    aux->prox_msg = NULL;

    for(i=0;i<max_tam;i++){
        aux2 = (PTR_MSG)malloc(sizeof(mensagem));
        aux2->flag = 0;
        aux->prox_msg = aux2;
        aux = aux2;
    }
    aux2->prox_msg = NULL;

    return inicio;
}

void far volta_dos() {
	disable();
	setvect(8, p_est->int_anterior);
	enable();
	exit(0);
}

void far cria_processo(void far (*end_proc)(), char *nome_proc, int max_tam){

    PTR_DESC_PROC aux = (PTR_DESC_PROC)malloc(sizeof(DESCRITOR_PROC));

    strcpy(aux->nome, nome_proc);
    aux->estado = ativo;
    aux->contexto = cria_desc();
    aux->fila_msg = inicializa_fila_msg(max_tam);
    aux->qtde_msg = 0;
    aux->tam_fila = max_tam;

    newprocess(end_proc, aux->contexto);
    insere_final(aux);
}

PTR_DESC_PROC far procura_proc(char *nome){
    PTR_DESC_PROC temp = prim;
    if(temp != NULL){
        temp = prim->prox_desc;
        while(temp != prim){
            if(temp->estado != terminado && strcmp(nome, temp->nome) == 0)
                return temp;
            temp = temp->prox_desc;
        }
    }
    return NULL;
}

int far envia_msg(char *nome_receptor, char *mensagem){
    PTR_DESC_PROC receptor, aux;
    PTR_MSG msg;

    disable();
    receptor = procura_proc(nome_receptor);

    if(!receptor){
        enable();
        return 0;
    }
    if(receptor->qtde_msg == receptor->tam_fila){
        enable();
        return 1;
    }

    msg = receptor->fila_msg;
    while(msg->flag) msg = msg->prox_msg;

    msg->flag = 1;
    strcpy(msg->emissor, prim->nome);
    strcpy(msg->msg, mensagem);
    receptor->qtde_msg++;

    if(receptor->estado == bloq_rec)
        receptor->estado = ativo;

    prim->estado = bloq_env;
    aux = prim;
    if((prim = procura_prox_ativo()) == NULL){
        enable();
        volta_dos();
    }

    transfer(aux->contexto, prim->contexto);

    return 2;
}

void far recebe_msg(char *nome_emissor, char *mensagem){
    PTR_DESC_PROC emissor, aux;
    PTR_MSG msg;

    disable();

    if(!prim->qtde_msg){
        prim->estado = bloq_rec;
        aux = prim;
        if((prim = procura_prox_ativo()) == NULL){
            enable();
            volta_dos();
        }

        transfer(aux->contexto, prim->contexto);
    }

    msg = prim->fila_msg;
    while(!msg->flag) msg = msg->prox_msg;
    msg->flag = 0;
    strcpy(nome_emissor, msg->emissor);
    strcpy(mensagem, msg->msg);
    prim->qtde_msg--;

    emissor = procura_proc(nome_emissor);
    if(emissor->estado == bloq_env)
        emissor-> estado = ativo;
    enable();
}

void far escalador(){
    p_est->p_origem = d_esc;
    p_est->p_destino = prim->contexto;
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
            if((prim = procura_prox_ativo()) == NULL) volta_dos();
            p_est->p_destino = prim->contexto;
        }
        enable();
    }
}

void far dispara_sistema(){
    PTR_DESC desc_dispara = cria_desc();
    d_esc = cria_desc();
    newprocess(escalador, d_esc);
    transfer(desc_dispara, d_esc);
}

void far termina_processo(){
    disable();
    prim->estado = terminado;
    enable();
    while(1);
}
