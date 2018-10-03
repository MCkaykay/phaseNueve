// isr.c, 159

#include "include.h"
#include "types.h"
#include "data.h"
#include "isr.h"
#include "lib.h"
#include "proc.h"

// to create a process: alloc PID, PCB, and process stack
// build trapframe, initialize PCB, record PID to ready_q
void NewProcISR(func_p_t p) {  // arg: where process code starts
   int pid;

   if(QisEmpty(&avail_q)) {    // may occur if too many been created
      cons_printf("Kernel panic: no more process!\n");
      breakpoint();     // cannot continue, alternative: breakpoint();
   }

   pid = DeQ(&avail_q);                         // alloc PID (1st is 0)
   Bzero((char*)&pcb[pid],sizeof(pcb_t));       // clear PCB
   Bzero((char*)&stack[pid],STACK_SIZE);        // clear stack
   pcb[pid].state= READY;                       // change process state

   EnQ(pid, &ready_q);                          // queue it

// point TF_p to stack & fill it out
   pcb[pid].TF_p = (TF_t *)&stack[pid][STACK_SIZE];                               
   pcb[pid].TF_p--;
   pcb[pid].TF_p->efl = EF_DEFAULT_VALUE|EF_INTR; // enables intr
   pcb[pid].TF_p->cs = get_cs();                  // duplicate from CPU
   pcb[pid].TF_p->eip = (int) p;                  // set to code
}

// count run time and switch if hitting time limit
void TimerISR(void) {
   int i;
   outportb(PIC_CONTROL, DONE);                       // notify PIC getting done

   pcb[cur_pid].time++;                               // count up time
   pcb[cur_pid].life++;                               // count up life

   if(pcb[cur_pid].time == TIME_MAX) {                // if runs long enough
      EnQ(cur_pid, &ready_q);                         // move it back to ready_q
      pcb[cur_pid].state=READY;                       // change its state
      cur_pid = -1;                                   // now no running proc
   }
   sys_ticks++;
   for(i=0; i< PROC_MAX; i++){
     if(pcb[i].state==SLEEPY && pcb[i].wake_time==sys_ticks){
       EnQ(i, &ready_q);
       pcb[i].state=READY;
     }
   }
}

void GetPidISR(void){
  pcb[cur_pid].TF_p->ebx = cur_pid;
}

void SleepISR(void){
  int sleep_sec = pcb[cur_pid].TF_p->ebx;       // get sleep second from ebx of the TF of process cur_pid
  pcb[cur_pid].wake_time = sys_ticks + sleep_sec * 100;  // set the wake time in PCB of process cur_pid
  pcb[cur_pid].state = SLEEPY;                  // change the state of process cur_pid
  cur_pid = -1;                                 // reset cur_pid
}

void SetVideoISR(void){
  int row = pcb[cur_pid].TF_p->ebx;             // get row from ebx in TF of process cur_pid
  int col = pcb[cur_pid].TF_p->ecx;             // get col from ecx in TF of process cur_pid
  video_p = HOME_POS + (row-1) * 80 + (col-1);
}

void WriteISR(void){
  int device = pcb[cur_pid].TF_p->ebx;
  char *str = (char *)pcb[cur_pid].TF_p->ecx;
  unsigned short col_pos;
  unsigned short rest;
  int i;
  if(device == STDOUT) {
    for(i=0; i<=strlen(str); i++){
      //if video_p is reaching END_POS then set back to HOME_POS
      if (video_p >= END_POS) video_p = HOME_POS;
      //if video_p apears at start of line then earse the entrie line
      if ((video_p - HOME_POS) % 80 == 0) Bzero((char *)video_p, 160);
      // if 'c' is not '\n' then
      if (str[i] != '\n') {
        // use video_p to write out 'c' and increment video_p
        *video_p = str[i] + VGA_MASK;
        video_p++;
      }
      else {
       col_pos = (video_p - HOME_POS) % 80; // calc column pos of current video_p
       rest = 80 - col_pos;                 // the 'rest' = 80 - current column pos
       video_p = video_p + rest;            // incr video_p by 'rest'
     }
    }
  }
}

void SemInitISR(void){
   // allocates a semaphore from the OS semaphore queue, empty it, and set the passes
   // retrun the semaphore ID via the trapframe
}

void SemWaitISR(void){
   // decrement the passes in the semaphore by 1 if it has any left (greater than 0)
   // otherwise, the calling process will be blocked: its PID is queued to the wait queue in the semaphore, state changed to WAIT, and OS current pid is reset

}

void SemPostISR(void){
   // checks if there is any blocked process in the wait queue of the semaphore
   int sem_id = pcb[cur_pid].TF_p->ebx;
   if (QisEmpty(&(sem[sem_id].wait_q))){
     sem[sem_id].passes++;
   }
   else{
   
   }

   // if none, increment the semaphore passes by 1
   // otherwise, move the 1st PID in the wait queue of the semaphore to the ready-to-run PID queue, and update its state
    
}
