.global sem_dec
.global sem_inc

@ ; sem_dec
@ ; Declare for use from C as extern void sem_dec(void * semaphore);
@     EXPORT sem_dec
@ sem_dec PROC
@ 1   LDREX   r1, [r0]
@     CMP		    r1, #0        ; Test if semaphore holds the value 0
@     BEQ     %f2           ; If it does, block before retrying
@     SUB     r1, #1        ; If not, decrement temporary copy
@     STREX   r2, r1, [r0]  ; Attempt Store-Exclusive
@     CMP     r2, #0        ; Check if Store-Exclusive succeeded
@     BNE     %b1           ; If Store-Exclusive failed, retry from start
@     DMB                   ; Required before accessing protected resource
@     BX      lr

sem_dec: ldrex r1, [r0]             @ atomic load semaphore value //initialize monitor
         cmp   r1, #0               @ check is sem == 0
         beq   wait_for_update      @ wait for update if == 0
         sub   r1, #1               @ decrement semaphore
         strex r2, r1, [r0]         @ attempt atomic store r1->[r0]. result held in r2.
         cmp   r2, #0               @ check success
         bne   wait_for_update      @ wait for update if failed
         dmb                        @ ensure atomic instrs. happen before this point is passed
         mov   pc, lr 

wait_for_update: svc #0x08             @ HOW DO WE GET BACK??
                 b sem_dec

sem_inc: ldrex r1, [r0]             @atomic load
         add   r1, #1               @add 1 to sem
         strex r2, r1, [r0]         @atomic store
         cmp   r2, #0               @check if store failed
         bne   sem_inc              @wait and retry if it did
         cmp   r0, #1
         dmb
         bge  unwait_all
         mov  pc, lr


unwait_all: svc #0x09              @ HOW DO WE GET BACK??
            mov pc, lr


@ 2   ; Take appropriate action while waiting for semaphore to be incremented
@     WAIT_FOR_UPDATE       ; Wait for signal to retry
@     B       %b1
@     ENDP


@ ; sem_inc
@ ; Declare for use from C as extern void sem_inc(void * semaphore);
@     EXPORT sem_inc
@ sem_inc PROC
@ 1   LDREX   r1, [r0]
@     ADD     r1, #1        ; Increment temporary copy
@     STREX   r2, r1, [r0]  ; Attempt Store-Exclusive
@     CMP     r2, #0        ; Check if Store-Exclusive succeeded
@     BNE     %b1           ; Store failed - retry immediately
@     CMP     	r0, #1        ; Store successful - test if incremented from zero
@     DMB                   ; Required before releasing protected resource
@     BGE     	%f2           ; If initial value was 0, signal update
@     BX      lr

@ 2   ; Signal waiting processors or processes
@     SIGNAL_UPDATE
@     BX      lr
@     ENDP
