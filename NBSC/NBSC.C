#include <NBSC.h>
#include <string.h>

typedef struct desc_p {
    char nome[35];
    enum{ativo, terminado} estado;
    PTR_DESC contexto;
    struct desc_p *prox_desc;
} DESCRITOR_PROC;

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

void far cria_processo(void far (*end_proc)(), char *nome_proc){
    PTR_DESC_PROC aux = (PTR_DESC_PROC)malloc(sizeof(DESCRITOR_PROC));
    strcpy(aux->nome, nome_proc);
    aux->estado = ativo;
    aux->contexto = cria_desc();
    newprocess(end_proc, aux->contexto);
    insere_final(aux);
}

void far volta_dos() {
	disable();
	setvect(8, p_est->int_anterior);
	enable();
	exit(0);
}

void far escalador(){
    p_est->p_origem = d_esc;
    p_est->p_destino = prim->contexto;
    p_est->num_vetor = 8;
    while(1){
        iotransfer();
        disable();
        if((prim = procura_prox_ativo()) == NULL) volta_dos();
        p_est->p_destino = prim->contexto;
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