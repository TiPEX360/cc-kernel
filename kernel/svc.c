#include "svc.h"
#include "hilevel.h"
#include "pipe.h"

extern uint32_t tos_usr;

void svc_handler_exec(ctx_t*ctx, uint32_t pc) {
  ctx->pc = pc; 
  ctx->sp = executing->tos;
  return;
}

void svc_handler_fork(ctx_t* ctx) {
  int pid = 0;
  for(int i = 0; i < MAX_PROCS && pid == 0; i++) {
    if(procTab[i].status == STATUS_INVALID || procTab[i].status == STATUS_TERMINATED) pid = i + 1;
  }

  if(pid == 0) {
    svc_handler_write(ctx, 0x01, "ERROR: Max procs reached! Execution may be unpredictable beyong this point. \n", 77);
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

  memcpy(procTab[pid - 1].tos + 4 - PageSize, executing->tos + 4 - PageSize, PageSize); //WRONG! do this descending, not ascending

  //Parent process return 1
  ctx->gpr[0] = pid; return; //Is it done?
};

void svc_handler_exit(ctx_t* ctx, status_t s) { // THIS NOT DONE PROPERLY PLS FIX
  executing->status = STATUS_TERMINATED;
  schedule(ctx);
}

void svc_handler_write(ctx_t* ctx, int fd, char *x, int n) {
  if(fd > 2) {
    pipe_t *pipe = NULL;
    for(int i = 0; i < MAX_PIPES; i++) {
      if(pipeTab[i].fd[1] == fd) pipe = &pipeTab[i];
    }
    if(pipe == NULL) {
      ctx->gpr[0] = 0;
      return;
    }

    int sent = 0;
    for(int i = 0; i < n; i++) {
      if ((pipe->tail + 1) != pipe->head) {
        pipe->data[pipe->tail] = x[i];
        pipe->tail = (pipe->tail + 1) % 16;
        sent++;
      }
    }

    ctx->gpr[0] = sent;
  }
  else {
    for( int i = 0; i < n; i++ ) {
      PL011_putc( UART0, *x++, true );
    }
    ctx->gpr[0] = n;
  }
}

void svc_handler_read(ctx_t* ctx, int fd, char *x, int n) {
  if(fd > 2) {
    pipe_t *pipe = NULL;
    for(int i = 0; i < MAX_PIPES && pipe == NULL; i++) {
      if(pipeTab[i].fd[0] == fd) pipe = &pipeTab[i];
    }
    if(pipe == NULL) {
      ctx->gpr[0] = 0;
      return;
    }

    int i = 0;
    while(i < n && (pipe->head != pipe->tail)) {
      x[i] = pipe->data[i];
      pipe->head = (pipe->head + 1) % 16;
      
      i++;
    }
    ctx->gpr[0] = i;
    return;
  }
}