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

   if(pcb[cur_pid].time == TIME_MAX) {              // if runs long enough
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
  // get sleep second from ebx of the TF of process cur_pid
  // set the wake time in PCB of process cur_pid to cur OS time + sleep second * 100
  // change the state of process cur_pid
  // reset cur_pid to ...
  int sleep_sec = pcb[cur_pid].TF_p->ebx;
  pcb[cur_pid].wake_time = sys_ticks + sleep_sec * 100;
  pcb[cur_pid].state = SLEEPY;
  cur_pid = -1;
}

void SetVideoISR(void){
  //get row from ebx in TF of process cur_pid
  //get col from ecx in TF of process cur_pid
  //set video pointer(video_p) to: HOME_POS + (row-1) * 80 + (col-1)
  int row = pcb[cur_pid].TF_p->ebx;
  int col = pcb[cur_pid].TF_p->ecx;
  video_p = (unsigned short*)(HOME_POS + (row-1) * 80 + (col-1));
}

void WriteISR(void){
  int device = (int)pcb[cur_pid].TF_p->ebx;
  char *str = (char *)pcb[cur_pid].TF_p->ecx;
  unsigned short col_pos;
  unsigned short rest;
  int i;
  unsigned short *video_p_copy;
  video_p = video_p_copy;
  if(device == STDOUT) {
    for(i=0; i<=strlen(str); i++){
      //if video_p is reaching END_POS then set back to HOME_POS
      if (video_p >= END_POS) video_p = HOME_POS;
      //if video_p apears at start of line then earse the entrie line
      if ((video_p - HOME_POS)%80 == 0) Bzero((char *)video_p, 160);
      // if 'c' is not '\n' then
      if (str[i] != '\n') {
        // use video_p to write out 'c' and increment video_p
        *video_p = str[i] + VGA_MASK;
        video_p++;
      }
      else {
        // calc column pos of current video_p
        // the 'rest' = 80 - current column pos
        // incr video_p by 'rest'
       col_pos = (video_p - HOME_POS) %80;
       rest = 80 - col_pos;
       video_p = video_p + rest; 
     }
    }
  }
}

