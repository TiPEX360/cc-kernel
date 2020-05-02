#include "scheduler.h"
#include "hilevel.h"

void dispatch(ctx_t* ctx, pcb_t* prev, pcb_t* next) {
  char prev_pid = '?', next_pid = '?';

  if(NULL != prev) {
    memcpy(&prev->ctx, ctx, sizeof(ctx_t)); // preserve execution context of P_{prev}
    prev_pid = '0' + prev->pid;
  }
  if(NULL != next) {
    memcpy(ctx, &next->ctx, sizeof(ctx_t)); // restore  execution context of P_{next}
    next_pid = '0' + next->pid;
  }

    PL011_putc(UART0, '[', true);
    PL011_putc(UART0, prev_pid, true);
    PL011_putc(UART0, '-', true);
    PL011_putc(UART0, '>', true);
    PL011_putc(UART0, next_pid, true);
    PL011_putc(UART0, ']', true);

    executing = next;                           // update   executing process to P_{next}

  return;
}

extern void lolevel_handler_rst();
void schedule(ctx_t* ctx) { //TERMINATED PROC GETS SCHEDULED!
  //Reset priority for process which has just been executing
  if(executing->priority != executing->initPriority) executing->priority = executing->initPriority;

  pcb_t* next = executing;
  if(next->status == STATUS_TERMINATED) {
    next->priority = 0;
    next->initPriority = 0;
  }

  //Get next priority process
  for (int i = 0; i < MAX_PROCS; i++) {
    if(procTab[i].status == STATUS_READY && procTab[i].pid != executing->pid) {
      if(procTab[i].priority > next->priority) next = &procTab[i];
    }
  }
  if(next->status == STATUS_TERMINATED) lolevel_halt();

  //Dispatch and update status
  pcb_t* prev = executing;
  dispatch(ctx, prev, next);
  if(prev->status == STATUS_EXECUTING) prev->status = STATUS_READY; //CHANGE HERE
  next->status = STATUS_EXECUTING;

  //Adjust priorities for next time
  for(int i = 0; i < MAX_PROCS; i++) {
    if(procTab[i].pid != executing->pid){
      procTab[i].priority++;
    }
  }
  
  return;
}