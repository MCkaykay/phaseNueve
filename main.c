// main.c, 159
// OS bootstrap and The Kernel for OS phase 1
//
// Team Name: null  
// Members: Mckayla Glaves, Javanika Naik

#include "include.h"  // given SPEDE stuff
#include "types.h"    // kernle data types
#include "lib.h"      // small handly functions
#include "proc.h"     // all user process code here
#include "isr.h"      // kernel ISR code
#include "entry.h"    // entries to kernel (TimerEntry, etc.)

// kernel data are all here:
int cur_pid;                        // current running PID; if -1, none selected
q_t ready_q, avail_q;               // avail PID and those created/ready to run
pcb_t pcb[PROC_MAX];                // Process Control Blocks
char stack[PROC_MAX][STACK_SIZE];   // process runtime stacks

void InitKernel(void) {             // init and set up kernel!
   int i;
   struct i386_gate *IVT_p;         // IVT's DRAM location

   IVT_p = get_idt_base();          // get IVT location
   fill_gate(&IVT_p[TIMER], (int)TimerEntry, get_cs(), ACC_INTR_GATE, 0); // fill out IVT for timer
   outportb(PIC_MASK, MASK);                   // mask out PIC for timer

   Bzero((char *)&avail_q,sizeof(q_t));                      // clear 2 queues
   Bzero((char *)&ready_q,sizeof(q_t));
   for(i=0; i<= PROC_MAX-1; i++){                 // add all avail PID's to the queue
     EnQ(i, &ready_q);
   }
}

void Scheduler(void) {                         // choose a cur_pid to run
   if (cur_pid > 0) return;                    // a user PID is already picked
   if (QisEmpty(&ready_q) && cur_pid == 0) return;
   if (QisEmpty(&ready_q) && cur_pid == -1) {
      cons_printf("Kernel panic: no process to run!\n");
      breakpoint();                                  // to GDB we go
   }

   //if cur_pid is not -1, then append cur_pid to ready_q; // suspend cur_pid
   //replace cur_pid with the 1st one in ready_q; // pick a user proc
   if (cur_pid != -1) {
     EnQ(cur_pid, &ready_q);
     pcb[cur_pid].state = READY;
     cur_pid = ready_q.a[0];
   }
   pcb[cur_pid].time = 0;
   pcb[cur_pid].state = AVAIL;
   // reset process time
   // change its state
}

int main(void) {                       // OS bootstraps
   //initialize the kernal-related stuff
   InitKernel();
   NewProcISR(InitProc);                         // create InitProc
   Scheduler();                        // call scheduler to set cur_pid to 1st PID
   Loader(pcb[cur_pid].TF_p);                // load proc to run
   return 0;                           // compiler needs it for syntax
}

void TheKernel(TF_t *TF_p) {           // kernel runs
   pcb[cur_pid].TF_p = TF_p;           // save TF address
   TimerISR();                         // handle timer event

   if (cons_kbhit()) {                 // if keyboard is pressed
     char key = cons_getchar();
     if (key == 'b') {                 // 'b' for breakpoint
      breakpoint();
     }
     if (key == 'n') {                 // 'n' for new process
      cons_printf("n was pressed for user proc");
      NewProcISR(UserProc);
     }
   }
   Scheduler();                        //which may pick another proc
   Loader(pcb[cur_pid].TF_p);          //load proc to run!
}

