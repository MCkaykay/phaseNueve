// types.h, 159

#ifndef __TYPES__
#define __TYPES__

#include "constants.h"

typedef void (*func_p_t)(void); // void-return function pointer type

typedef enum {AVAIL, READY, RUN, SLEEPY, WAIT, ZOMBIE} state_t;

typedef struct {
   unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
   unsigned int entry, eip, cs, efl;
} TF_t;

typedef struct {
   int time;
   int life;
   int wake_time;
   TF_t *TF_p;
   state_t state;
   func_p_t sigint_handler_p;
   func_p_t sigchld_handler_p;
   int ppid;
} pcb_t;                     

typedef struct {             // generic queue type
  int q[Q_SIZE];             // for a circular queue
  int head, tail, size; 
} q_t;

typedef struct {
  int passes;                // max # of processes that can pass
  q_t wait_q;                // blocked proc ID's
} sem_t;

typedef struct {
  int io;                    // I/O # of terminal port (TERM0_IO and TERM1_IO)
  int done;                  // done signal to PIC (TERM0_DONE and TERM1_DONE)
  char *tx_p;                // point to a character in the string
  q_t tx_wait_q;             // TX requester (wait for completion)
  char *rx_p;                // point to inside the buffer
  q_t rx_wait_q;             // for PID waiting for term KB input
} term_if_t;

#endif
