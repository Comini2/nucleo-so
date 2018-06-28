#include <NPRIO.h>
#include <string.h>
#include <stdio.h>

typedef struct desc_p {
    char nome[35];
    enum{ativo, terminado} estado;
    PTR_DESC contexto;
    struct desc_p *prox_desc;
} DESCRITOR_PROC;

typedef DESCRITOR_PROC *PTR_DESC_PROC;

PTR_DESC_PROC *fila = NULL;

PTR_DESC_PROC proc_atual = NULL;

PTR_DESC d_esc;

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

        proc->prox_desc = NULL;
        temp->prox_desc = proc;
    }
    printf("\nInserindo %s -> fila[%d]\n", proc->nome, prioridade);

}

PTR_DESC_PROC far procura_prox_ativo(){
    static int prior_atual = MAX_PRIOR - 1;
    PTR_DESC_PROC temp = proc_atual;
    while(1){
        temp = temp->prox_desc;
        if(temp == NULL){
            if(prior_atual < 0)
                return NULL;
            while(--prior_atual > 0 && fila[prior_atual]){
                if(fila[prior_atual]->estado == ativo)
                    return fila[prior_atual];
                proc_atual = fila[prior_atual];
                return procura_prox_ativo();
            }
        }
        if(temp->estado == ativo)
            return temp;
    }
}

void far cria_processo(void far (*end_proc)(), char *nome_proc, int prioridade){
    PTR_DESC_PROC aux = (PTR_DESC_PROC)malloc(sizeof(DESCRITOR_PROC));
    if(!fila)
        inicializa_fila();
    strcpy(aux->nome, nome_proc);
    aux->estado = ativo;
    aux->contexto = cria_desc();
    newprocess(end_proc, aux->contexto);
    insere_final(aux, prioridade);
}

void far volta_dos() {
	disable();
	setvect(8, p_est->int_anterior);
	enable();
	exit(0);
}

void far escalador(){
    p_est->p_origem = d_esc;
    p_est->p_destino = proc_atual->contexto;
    p_est->num_vetor = 8;
    while(1){
        iotransfer();
        disable();
        printf("\nProcesso atual: %s", proc_atual->nome);
        if((proc_atual = procura_prox_ativo()) == NULL){ 
            printf("\nProcesso não encontrado!");
            volta_dos();
        } 
        p_est->p_destino = proc_atual->contexto;
        enable();
    }
}

void far dispara_sistema(){
    PTR_DESC desc_dispara;
    int i;

    for(i = 0; i < MAX_PRIOR-1; i++){
        insere_final(fila[i+1], i);
    }

    for(i = 0; i < MAX_PRIOR; i++){
        printf("Fila %d: %s\t->\t%s\n", i, fila[i]->nome, fila[i]->prox_desc->nome);
    }
    
    proc_atual = fila[MAX_PRIOR-1];

    desc_dispara = cria_desc();
    d_esc = cria_desc();
    newprocess(escalador, d_esc);
    transfer(desc_dispara, d_esc);
}

void far termina_processo(){
    disable();
    proc_atual->estado = terminado;
    enable();
    while(1);
}