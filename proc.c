// proc.c, 159
// all processes are coded here
// processes do not access kernel data, must use syscalls

#include "constants.h" // include only these 2 files
#include "syscalls.h"

void InitProc(void) {
   int i;

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
   //str = (my_pid + '0') + '\n';
   str[0]=my_pid/10;
   str[1]=my_pid%10;
   str[2]='\n';
   SetVideo(my_pid+1,1); // set video cursor to beginning of my row
   Write(STDOUT, "Print this big ass sentence to ensure that this test line wraps around the screen and then check for erasure! :)");
   Sleep(2); // sleep for 2 seconds

   while(1) {
      SetVideo(my_pid+1,1);         //call service to set video cursor to beginning of my row
      Write(STDOUT, str);           //call service to write out my PID str
      Sleep(2);

      SetVideo(my_pid+1,1);         //call service to set video cursor to beginning of my row
      Write(STDOUT, "--");          //call service to erase my PID str (with "--")
      Sleep(2);
   }
}
