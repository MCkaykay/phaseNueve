// proc.c, 159
// all processes are coded here
// processes do not access kernel data, must use syscalls

#include "constants.h" // include only these 2 files
#include "syscalls.h"
#include "data.h"

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

void TermInit(int index) {
  // clear the terminal interfaced (that is being indexed)
  // set the 'io' in the interface to either TERM_IO or TERM1_IO
  // set the 'done' to either TERM0_DONE or TERM1_DONE
  outportb(term_if[which].io+CFCR, CFCR_DLAB); // CFCR_DLAB is 0x80
  outportb(term_if[which].io+BAUDLO, LOBYTE(115200/9600)); // period of each 9600 bauds
  outportb(term_if[which].io+BAUDHI, HIBYTE(115200/9600));
  outportb(term_if[which].io+CFCR, CFCR_PEVEN|CFCR_PENAB|CFCR_7BITS);
  outportb(term_if[which].io+IER, 0);
  outportb(term_if[which].io+MCR, MCR_DTR|MCR_RTS|MCR_IENABLE);
  for(i=0; i<LOOP/2; i++) asm("inb $0x80");
  outportb(term_if[which].io+IER, IER_ERXRDY|IER_ETXRDY); // enable TX & RX intr
  for(i=0; i<LOOP/2; i++) asm("inb $0x80");
  inportb(term_if[which].io);                             // clear key cleared PROCOMM screen 
}

void TermProc(void){
  int my_pid, device;
  char str[3];

  my_pid = GetPid();
  str[0] = my_pid / 10 + '0';
  str[1] = my_pid % 10 + '0';
  str[2] = '\0';

  // determine what my 'device' should be (even PID TERM0, odd TERM1)
  
  while(1){
    // every 5 lines Write() to my device some special symbols (help viewing)
    // Write() 'str' to my device
    // Write() a lengthier message (see demo) to my device
    Sleep(3) // sleep for 3 seconds
  }
}
