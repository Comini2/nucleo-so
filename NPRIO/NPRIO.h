#include <System.h>

#define MAX_PRIOR 4

extern void far cria_processo(void far (*end_proc)(), char *nome_proc, int prioridade);

extern void far dispara_sistema();

extern void far termina_processo();