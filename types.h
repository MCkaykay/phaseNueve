// types.h, 159

#ifndef __TYPES__
#define __TYPES__

#include "constants.h"

typedef void (*func_p_t)(void); // void-return function pointer type

typedef enum {AVAIL, READY, RUN, SLEEPY} state_t;

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
} pcb_t;                     

typedef struct {             // generic queue type
  int q[Q_SIZE];             // for a circular queue
  int head, tail, size; 
} q_t;

#endif
