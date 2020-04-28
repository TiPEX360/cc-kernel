#include "svc.h"
#include "hilevel.h"

extern uint32_t tos_usr;

void svc_handler_exec(ctx_t*ctx, uint32_t pc) {
  ctx->pc = pc; 
  ctx->sp = executing->tos;
  return;
}

void svc_handler_fork(ctx_t* ctx) {
  int pid = NULL;
  for(int i = 0; i < MAX_PROCS && pid == NULL; i++) {
    if(procTab[i].status == STATUS_INVALID || procTab[i].status == STATUS_TERMINATED) pid = i + 1;
  }

  if(pid == NULL) {
    ctx->gpr[0] = -1; 
    return;
  }

  memset(&procTab[pid - 1], 0, sizeof(pcb_t));
  procTab[pid - 1].pid = pid;
  procTab[pid - 1].status = STATUS_READY;
  procTab[pid - 1].tos = (uint32_t)(&tos_usr) - PageSize * (pid - 1); //Give it own stack?
  procTab[pid - 1].priority = 3;
  procTab[pid - 1].initPriority = 3;

  procTab[pid - 1].ctx.cpsr = 0x50;
  procTab[pid - 1].ctx.pc = ctx->pc;
  procTab[pid - 1].ctx.gpr[0] = 0; 
  procTab[pid - 1].ctx.sp = procTab[pid - 1].tos;
  procTab[pid - 1].ctx.lr = ctx->lr;

  for(int i = 1; i < 13; i++) procTab[pid - 1].ctx.gpr[i] = ctx->gpr[i];

  memcpy(procTab[pid - 1].tos, executing->tos, PageSize); 

  //Parent process return 1
  ctx->gpr[0] = pid; return; //Is it done?
};

void svc_handler_exit(ctx_t* ctx, status_t s) { // THIS NOT DONE PROPERLY PLS FIX
  executing->status = STATUS_TERMINATED;
  schedule(ctx);
}