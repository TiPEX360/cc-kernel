#ifndef __SCHEDULER_H
#define __SCHEDULER_H

#include "pcb.h"

void dispatch(ctx_t* ctx, pcb_t* prev, pcb_t* next);

void schedule(ctx_t* ctx);

#endif