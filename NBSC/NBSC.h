#include <System.h>

extern void far cria_processo(void far (*end_proc)(), char *nome_proc);

extern void far dispara_sistema();

extern void far termina_processo();