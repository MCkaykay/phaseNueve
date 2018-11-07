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
#include "syscalls.h" 

// kernel data are all here:
int cur_pid;                        // current running PID; if -1, none selected
q_t ready_q, avail_q;               // avail PID and those created/ready to run
pcb_t pcb[PROC_MAX];                // Process Control Blocks
char stack[PROC_MAX][STACK_SIZE];   // process runtime stacks
int sys_ticks;                      // OS time (timer ticks), starting 0
unsigned short *video_p;            // PC VGA video pointer, starting HOME_POS
sem_t sem[SEM_MAX];                 // kernel has these semaphores
q_t sem_q;                          // semaphore ID's are intially queued here
int car_sem;                        // to hold a semaphore ID for testing
term_if_t term_if[TERM_MAX];

void TermInit(int index) {
  int i;
  Bzero((char *)&term_if[index], sizeof(term_if_t));// clear the terminal interfaced (that is being indexed)
  // set the 'io' in the interface to either TERM0_IO or TERM1_IO
  // set the 'done' to either TERM0_DONE or TERM1_DONE
  if(index ==0){
    term_if[index].io = TERM0_IO;
    term_if[index].done = TERM0_DONE;
  }
  if(index ==1){
    term_if[index].io = TERM1_IO;
    term_if[index].done = TERM1_DONE;
  }
  outportb(term_if[index].io+CFCR, CFCR_DLAB); // CFCR_DLAB is 0x80
  outportb(term_if[index].io+BAUDLO, LOBYTE(115200/9600)); // period of each 9600 bauds
  outportb(term_if[index].io+BAUDHI, HIBYTE(115200/9600));
  outportb(term_if[index].io+CFCR, CFCR_PEVEN|CFCR_PENAB|CFCR_7BITS);
  outportb(term_if[index].io+IER, 0);
  outportb(term_if[index].io+MCR, MCR_DTR|MCR_RTS|MCR_IENABLE);
  for(i=0; i<LOOP/2; i++) asm("inb $0x80");
  outportb(term_if[index].io+IER, IER_ERXRDY|IER_ETXRDY); // enable TX & RX intr
  for(i=0; i<LOOP/2; i++) asm("inb $0x80");
  inportb(term_if[index].io);                             // clear key cleared PROCOMM screen 
}

void InitKernel(void) {             // init and set up kernel!
   int i;
   struct i386_gate *IVT_p;         // IVT's DRAM location
   
   IVT_p = get_idt_base();          // get IVT location
   fill_gate(&IVT_p[TIMER], (int)TimerEntry, get_cs(), ACC_INTR_GATE, 0); // fill out IVT for timer
   fill_gate(&IVT_p[SYSCALL], (int)SyscallEntry, get_cs(), ACC_INTR_GATE, 0); // fill out IVT for syscall
   fill_gate(&IVT_p[TERM0], (int)Term0Entry, get_cs(), ACC_INTR_GATE, 0); // fill out IVT for terminal 0
   fill_gate(&IVT_p[TERM1], (int)Term1Entry, get_cs(), ACC_INTR_GATE, 0); // fill out IVT for terminal 1
   outportb(PIC_MASK, MASK);        // mask out PIC for timer

   Bzero((char *)&avail_q,sizeof(q_t)); // clear 2 queues
   Bzero((char *)&ready_q,sizeof(q_t));
   for(i=0; i<= PROC_MAX-1; i++){       // add all avail PID's to the queue
     EnQ(i, &avail_q);
     pcb[i].state=AVAIL;
   }
   cur_pid= -1;
   Bzero((char *)&sem_q, sizeof(q_t));
   for(i=0; i<= SEM_MAX-1; i++){
     EnQ(i, &sem_q);
   }
   car_sem = 0;
   video_p = HOME_POS;
   sys_ticks=0;
   TermInit(0);
   TermInit(1);
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
   }
   cur_pid = DeQ(&ready_q);
   pcb[cur_pid].time = 0;              // reset process time
   pcb[cur_pid].state = RUN;           // change its state
}

int main(void) {                       // OS bootstraps
   //initialize the kernal-related stuff
   InitKernel();
   NewProcISR(InitProc);               // create InitProc
   Scheduler();                        // call scheduler to set cur_pid to 1st PID
   Loader(pcb[0].TF_p);                // load proc to run
   return 0;                           // compiler needs it for syntax
}

void TheKernel(TF_t *TF_p) {           // kernel runs
   pcb[cur_pid].TF_p = TF_p;           // save TF address
   
   switch(TF_p->entry){
     case TIMER: TimerISR(); break;
     case WRITE: WriteISR(); break;
     case SLEEP: SleepISR(); break;
     case GETPID: GetPidISR(); break;
     case SETVIDEO: SetVideoISR(); break;
     case SEMINIT: SemInitISR(); break;
     case SEMWAIT: SemWaitISR(); break;
     case SEMPOST: SemPostISR(); break;
     case TERM0: TermISR(0); break;
     case TERM1: TermISR(1); break;
     case READ: ReadISR(); break;
     case SIGNAL: SignalISR(); break;
     case GETPPID: GetPpidISR(); break;
     case FORK: ForkISR(); break;
   }

   if (cons_kbhit()) {                 // if keyboard is pressed
     char key = cons_getchar();
     if (key == 'b') {                 // 'b' for breakpoint
       breakpoint();
     }
     if (key == 'n') {                 // 'n' for new process
       NewProcISR(UserProc);
     }
     if (key == 'c') {                 // 'c' to create CarProc() 
       NewProcISR(CarProc);
     }
     if (key == 't'){
       NewProcISR(TermProc);           // 't' to request outputs to a terminal
     }
   }
   Scheduler();                        //which may pick another proc
   Loader(pcb[cur_pid].TF_p);          //load proc to run!
}
