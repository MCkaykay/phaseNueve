// proc.c, 159
// all processes are coded here
// processes do not access kernel data, must use syscalls

#include "constants.h" // include only these 2 files
#include "syscalls.h"
#include "data.h"
#include "types.h"
#include "lib.h"

void InitProc(void) {
   int i;
   car_sem = SemInit(3);
   while(1) {
      SetVideo(1, 1);         // pos video
      Write(STDOUT, ".");
      for(i=0; i<LOOP/2; i++)asm("inb $0x80"); // busy CPU

      SetVideo(1, 1);         // pos video
      Write(STDOUT, " ");
      for(i=0; i<LOOP/2; i++)asm("inb $0x80"); // busy CPU
   }
}

void UserProc(void) {
   int my_pid;
   char str[3];

   // get my PID and make a string from it (null-delimited)
   my_pid = GetPid();
   str[0]=my_pid / 10 + '0';
   str[1]=my_pid % 10 + '0';
   str[2]='\0';
   SetVideo(my_pid + 1, 1);            // set video cursor to beginning of my row
   Write(STDOUT, "Print this big ass sentence to ensure that this test line wraps around the screen and then check for erasure! :)");
   Sleep(2);                           // sleep for 2 seconds

   while(1) {
      SetVideo(my_pid + 1, 1);         //call service to set video cursor to beginning of my row
      Write(STDOUT, str);              //call service to write out my PID str
      Sleep(2);

      SetVideo(my_pid + 1, 1);         //call service to set video cursor to beginning of my row
      Write(STDOUT, "--");             //call service to erase my PID str (with "--")
      Sleep(2);
   }
}

void CarProc(void){
   // get my pid and build a str
   int my_pid;
   char str[3];
   my_pid = GetPid();
   str[0] = my_pid / 10 + '0';
   str[1] = my_pid % 10 + '0';
   str[2] = '\0';
   
   SetVideo(my_pid + 1, 1);           // show pid on beginning of 'my' row
   Write(STDOUT, str);

   while(1){
     SetVideo(my_pid + 1, 10);
     Write(STDOUT, "I'm off ...       "); // show: I'm off ... on my row (skip my PID, don't overwrite it)
     Sleep(2);                        // sleep for 2 seconds 

     SemWait(car_sem);                // semaphore-wait on the car semaphore
     
     SetVideo(my_pid + 1, 10);
     Write(STDOUT, "I'm on the bridge!"); // show: I'm on the bridge! (on the same location to overwrite above)
     Sleep(2);                        // sleep for 2 seconds
     SemPost(car_sem);                // semaphore-post on the car semaphore
   }
}

void Ouch(void){
  int device, pid;
  pid = GetPid();
  if(pid%2 == 0) device = TERM0;
  else device = TERM1;
  Write(device, "Ouch, don't touch that!\n\r");
}

void Wrapper(func_p_t handler_p){
   asm("pushal");         // save registers
   handler_p();           // call user's signal handler
   asm("popal");          // restore registers
   asm("movl %%ebp, %%esp; popl %%ebp; ret $4"::); // skip Ouch addr
}

void ChildCode(void){
   int my_pid, ppid, device;
   char str[3];
   my_pid = GetPid();         // get my PID
   ppid = GetPpid();          // get parent PID
   // select device from parent PID
   if(ppid % 2 == 0) device = TERM0;
   else device = TERM1;
   // build a string based on my PID
   str[0] = my_pid / 10 + '0';
   str[1] = my_pid % 10 + '0';
   str[2] = '\0';
   Write(device, "I am child PID ");
   Write(device, str);
   Write(device, "\n\r");
   Sleep(my_pid);
   Exit(my_pid * 5);          // exiting with a special exit number for parent to get
}

void ChldHandler(void){
   int pid, cpid, device, ec;
   char str[3],cstr[3];
   pid = GetPid();
   cpid = Wait(&ec);
   str[0] = pid / 10 + '0';
   str[1] = pid % 10 + '0';
   str[2] = '\0';
   cstr[0] = cpid / 10 + '0';
   cstr[1] = cpid % 10 + '0';
   cstr[2] = '\0';
   if(pid % 2 == 0) device = TERM0;
   else device = TERM1;
   // issue several Write() calls to print info from Wait() 
   Write(device, "my_pid ");
   Write(device, str);
   Write(device, ", cpid ");
   Write(device, cstr);
   Write(device, ", ec ");
   str[0] = ec / 10 + '0';
   str[1] = ec % 10 + '0';
   str[2] = '\0';
   Write(device, str);
   Write(device, "\r\n");
   Signal(SIGCHLD, (func_p_t)0); // issue Signal() call to cancel ChldHandler
}

void TermProc(void){
  int my_pid, device, fg, cpid, ec;
  char str[3], cstr[3], estr[3];
  char buff[BUFF_SIZE];
  my_pid = GetPid();
  str[0] = my_pid / 10 + '0';
  str[1] = my_pid % 10 + '0';
  str[2] = '\0';
  // determine what my 'device' should be (even PID TERM0, odd TERM1)
  if(my_pid % 2 == 0) device = TERM0;
  else device = TERM1;
  Signal(SIGINT, Ouch);         // call syscall to register Ouch() as its handler
  while(1){
    Write(device, str);         // Write() 'str' to my device 
    Write(device, ": enter > ");
    Read(device, buff); // read whats entered from terminal KB
    Write(device, "\n\rentered: ");
    Write(device, buff);
    Write(device, "\n\r");
    if(StrCmp(buff, "fork")) fg = 1;
    else if(StrCmp(buff, "fork&")) fg = 0;
    else continue;
    
    if(!fg) Signal(SIGCHLD, ChldHandler);     // child runs in the background

    switch(Fork()){
      case -1: Write(device, "OS failed to fork!"); break;
      case 0: 
        if(-1 == Exec((func_p_t2)ChildCode, device)) {
           Write(device, "OS failed to Exec()!\n\r");
           Exit(-1);
         }
      default: 
        Sleep(my_pid * 2);
        if(fg){
           cpid = Wait(&ec);
           cstr[0] = cpid / 10 + '0';
           cstr[1] = cpid % 10 + '0';
           cstr[2] = '\0';
           estr[0] = ec/10 + '0';
           estr[1] = ec%10 + '0';
           estr[2] = '\0';
           Write(device, "my_pid ");
           Write(device, str);
           Write(device, ", cpid ");
           Write(device, cstr);
           Write(device, ", ec ");
           Write(device, estr);
           Write(device, "\n\r");
       }
    }
  }
}
