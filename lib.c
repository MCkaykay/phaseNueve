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
   if(p->size==Q_SIZE)
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
   pid = p->q[p->head];
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
   p->q[p->tail] = to_add;
   p->size = p->size + 1;
   p->tail = (p->tail +1) % Q_SIZE;
}

int mystrlen(char *p){
  int count =0;
  while(*p != '\0'){
    count++;
    p++;
  }
  return count;
}

int StrCmp(char *s1, char *s2){
   // returns 1 if two strings are the same otherwise 0
   int match = 1;
   while((*s1 != '\0') && (*s2 != '\0')){
     if(*s1 != *s2){
       match = 0;
     }
     else{
       s1++;
       s2++;
     }
   }
   if(match == 0) return 0;
   else return 1;
}

void MemCpy(char *dst, char *src, int size){
   // copies 'size' bytes from memory location 'src' to location 'dst'
   int i=0;
   for(i=0; i<size; i++){
     dst[i]=src[i];
   }
}

int InQ(int pid, q_t *p){
   int i;
   for (i=0; i < p->size; i++){
     if(pid == p->q[(p->head + i) % Q_SIZE]) return 1;
   }
   return 0;
}

void DelQ(int pid, q_t *p){
   int i;
   q_t tmp_q;
   Bzero((char *)&tmp_q, sizeof(q_t));
   for(i=0; i < p->size; i++){
     if(pid == p->q[(p->head + i) % Q_SIZE]) continue;
     EnQ(DeQ(p), &tmp_q);
   }
   *p = tmp_q;
}

void ReclaimPages(int pid){
   // search thru the memory info array for those
   // currently owned by 'pid' and change the info
   // to 'not owned'
}

int Alloc(int pid, int how_many, int page_index[]){
   // search thru the memory info array for 'how_many'
   // pages currently now owned, and set them to be
   // owned by 'pid' and collect their pages indices
   // into page_index[]
   // return 0 if sucessfully got that many asked
   // otherwise, leave/recover owner info intact and return -1
}
