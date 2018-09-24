// lib.c, 159

#include "include.h"
#include "types.h"
#include "data.h"

// clear DRAM data block, zero-fill it
void Bzero(char *p, int bytes) {
   while(bytes--) *p++ = (char)0;
}

int QisEmpty(q_t *p) { // return 1 if empty, else 0
  if(p->size==0)
    return 1;
  else
    return 0;
}

int QisFull(q_t *p) { // return 1 if full, else 0
   if(p->size==PROC_MAX)
     return 1;
   else
     return 0;
}

// dequeue, 1st integer in queue
// if queue empty, return -1
int DeQ(q_t *p) { // return -1 if q[] is empty
   int pid; 
   if(QisEmpty(p)) {
      return -1;
   }
   pid = p->a[p->head];
   p->size--;
   p->head = (p->head+1) % Q_SIZE;
   return pid;
}

// if not full, enqueue integer to tail slot in queue
void EnQ(int to_add, q_t *p) {
   if(QisFull(p)) {
      cons_printf("Kernel panic: queue is full, cannot EnQ!\n");
      return;
   }
   p->a[p->tail] = to_add;
   p->size = p->size + 1;
   p->tail = (p->tail +1) % Q_SIZE;
}

