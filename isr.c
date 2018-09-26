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
      cons_printf("Kkernel panic: no more process!\n");
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
   outportb(PIC_CONTROL, DONE);                       // notify PIC getting done

   pcb[cur_pid].time++;                               // count up time
   pcb[cur_pid].life++;                               // count up life

   if(pcb[cur_pid].time == TIME_MAX) {              // if runs long enough
      EnQ(cur_pid, &ready_q);                         // move it back to ready_q
      pcb[cur_pid].state=READY;                       // change its state
      cur_pid = -1;                                   // now no running proc
   }
   return;
}

void GetPidISR(void){
  pcb[cur_pid].TF_p->ebx = cur_pid;
}

void SleepISR(void){
  // get sleep second from ebx of the TF of process cur_pid
  // set the wake time in PCB of process cur_pid to cur OS time + sleep second * 100
  // change the state of process cur_pid
  // reset cur_pid to ...
  pcb[cur_pid].wake_time = sys_ticks + 100 * (pcb[cur_pid].TF_p->ebx);
  pcb[cur_pid].state = SLEEPY;
  cur_pid = -1;
}

void SetVideoISR(void){
  //get row from ebx in TF of process cur_pid
  //get col from ecx in TF of process cur_pid
  //set video pointer(video_p) to: HOME_POS + (row-1) * 80 + (col-1)
  int row = pcb[cur_pid].TF_p->ebx;
  int col = pcb[cur_pid].TF_p->ecx;
  *video_p = HOME_POS + (row-1) * 80 + (col-1);
}

void WriteISR(void){
  int device = pcb[cur_pid].TF_p->ebx;
  char *str = pcb[cur_pid].TF_p->ecx;

  if(device == STDOUT) {

  }
}

