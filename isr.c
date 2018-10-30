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
  int i, arr_index;
  if(*str == '\0') return;
  if((device == TERM0) || (device == TERM1)){
    if(device == TERM0) arr_index=0;
    if(device == TERM1) arr_index=1;
    outportb(term_if[arr_index].io, *str);
    str++;
    term_if[arr_index].tx_p = str;
    EnQ(cur_pid, &term_if[arr_index].tx_wait_q);
    pcb[cur_pid].state = WAIT;
    cur_pid=-1;
  }
  if(device == STDOUT) {
    for(i=0; i<=mystrlen(str); i++){
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
   unsigned short * p;
   int sem_id, passes;
   sem_id = DeQ(&sem_q);
   if (sem_id == -1) {
     pcb[cur_pid].TF_p->ecx = sem_id;
     cons_printf("Kernal Panic: no more semaphores\n");
     breakpoint();
   }
   passes = pcb[cur_pid].TF_p->ebx;
   pcb[cur_pid].TF_p->ecx = sem_id;
   Bzero((char *)&sem[sem_id], sizeof(sem_t));
   sem[sem_id].passes = passes;
   p= HOME_POS + 21 * 80;
   *p = sem[sem_id].passes + '0' + VGA_MASK;
}

void SemWaitISR(void){
   unsigned short *p;
   int sem_id = pcb[cur_pid].TF_p->ebx;
   if (sem[sem_id].passes > 0){       // decrement the passes in the semaphore by 1 if it has any left (greater than 0)
    sem[sem_id].passes--;
   }
   else{
    EnQ(cur_pid, &sem[sem_id].wait_q);
    pcb[cur_pid].state = WAIT;
    cur_pid = -1;
   }
   p = HOME_POS + 21 * 80;
   *p = sem[sem_id].passes + '0' + VGA_MASK;
}

void SemPostISR(void){
   unsigned short *p;
   int pid, sem_id;
   sem_id = pcb[cur_pid].TF_p->ebx; 
   if (QisEmpty(&(sem[sem_id].wait_q))){          // check if there is any blocked process in the wait queue of the sem
     sem[sem_id].passes++;                        // if none, increment the semaphore passes by 1
   }
   else{                                          // otherwise, move the 1st PID in the wait queue of the sem to the ready-to-run PID queue
     pid = DeQ(&sem[sem_id].wait_q);
     EnQ(pid, &ready_q);
     pcb[pid].state = READY;                  // update the state
   }
   p = HOME_POS + 21 * 80;
   *p = sem[sem_id].passes + '0' + VGA_MASK;
}

void TermISR(int index) {
   // read 'event' by inportb(), via the terminal 'io' plus the offset 'IIR'
   unsigned int event = inportb(term_if[index].io + IIR);
   // if event read is IIR_TXRDY, call TermTxISR() with the array index
   if(event == IIR_TXRDY) TermTxISR(index);
   // if event read is IIR_RXRDY, just cons_printf() an asterisk on target PC
   if(event == IIR_RXRDY) TermRxISR(index); 
   outportb(PIC_CONTROL, term_if[index].done);
}

void TermTxISR(int index){
   int pid;
   // return if tx_wait_q in the terminal interface is empty
   if (QisEmpty(&term_if[index].tx_wait_q)) return;
   // if tx_p in the terminal interface points to null charater:
   // 1-2-3. release the 1st process from tx_wait_q (the 3 steps)
   if (*term_if[index].tx_p == '\0'){
     pid = DeQ(&term_if[index].tx_wait_q);
     EnQ(pid, &ready_q);
     pcb[pid].state = READY;
   }
   else{  
     outportb(term_if[index].io, *term_if[index].tx_p);
     term_if[index].tx_p++;
     return;
   }
}

void ReadISR(void){
   int device, term_interface;
   char *buff = (char *)pcb[cur_pid].TF_p->ecx;
   device = pcb[cur_pid].TF_p->ebx;
   // determine which terminal interface to use
   if(device == TERM0) term_interface=0;
   if(device == TERM1) term_interface=1;
   // set the RX pointer of the interface to 'buff'
   term_if[term_interface].rx_p = buff;
   // "block" the current process to the RX wait queue of the interface
   EnQ(cur_pid, &term_if[term_interface].rx_wait_q);
   pcb[cur_pid].state = WAIT;
   cur_pid = -1;
}

void TermRxISR(int interface_num) {
   char ch;
   int pid;
   ch = inportb(term_if[interface_num].io);
   if(ch == SIGINT) {
      if(QisEmpty(&term_if[interface_num].rx_wait_q)){
         return;
      }
      pid = DeQ(&term_if[interface_num].rx_wait_q);
      EnQ(pid, &ready_q);
      pcb[pid].state=READY;
      *term_if[interface_num].rx_p = '\0';
      if(!QisEmpty(&pcb[pid].sigint_handler_p)){
         WrapperISR(pid, &pcb[pid].sigint_handler_p);
      }
      return;
   }
   if(!(ch == '\n' || ch == '\r')){
     outportb(term_if[interface_num].io, ch);
     if(!QisEmpty(&term_if[interface_num].rx_wait_q)){
       *term_if[interface_num].rx_p = ch; // using the RX pointer of the interface to append it to buff
       term_if[interface_num].rx_p++;  // advance the RX pointer
     }
     return;
   }
   if(!QisEmpty(&term_if[interface_num].rx_wait_q)){
     *term_if[interface_num].rx_p = '\0'; // delimit 'buff' with a null character
     pid = DeQ(&term_if[interface_num].rx_wait_q);
     EnQ(pid, &ready_q);
     pcb[pid].state = READY;
   }
}

void SignalISR(void){
   // will register the handler address to the PCB
   pcb[cur_pid].sigint_handler_p = (func_p_t)pcb[cur_pid].TF_p->ebx;
}

void WrapperISR(int pid, func_p_t handler_p){
   int tmp;
   tmp = *pcb[pid].TF_p; // copy process trapframe to a temporary trapframe (local)
   (int)pcb[pid].TF_p -= 8; // lower the trapframe location info (in PCB) by 8 bytes
   *pcb[pid].TF_p = tmp; // copy temporary trapframe to the new lowered location
   // the vacated 8 bytes: put 'handler_p,' and 'eip' of the old trapframe there
   // change 'eip' in the moved trapframe to Wrapper() address
}
