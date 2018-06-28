#include <System.h>

extern void far cria_processo(void far (*end_proc)(), char *nome_proc, int max_tam);

extern void far dispara_sistema();

extern void far termina_processo();

extern int far envia_msg(char *receptor, char *mensagem);

void far recebe_msg(char *nome_emissor, char *mensagem);
