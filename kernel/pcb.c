#include "pcb.h"

const int PageSize = (0x00006000 / MAX_PROCS) - ((0x00006000 / MAX_PROCS) % 8) ;

pcb_t procTab[MAX_PROCS]; 
pcb_t *executing = NULL;