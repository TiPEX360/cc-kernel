#ifndef __SVC_H
#define __SVC_H

#include "pcb.h"

void svc_handler_exec(ctx_t*ctx, uint32_t pc);

void svc_handler_fork(ctx_t* ctx);

void svc_handler_exit(ctx_t* ctx, status_t s);

#endif