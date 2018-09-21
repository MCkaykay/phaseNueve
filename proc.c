// proc.c, 159
// all user processes are coded here
// processes do not R/W kernel data or code, must use syscalls

#include "include.h"
#include "data.h"
#include "proc.h"

void InitProc(void) {
   int i;
   unsigned short *p;

   //point p to 0xb8000; // upper-left corner of display
   p = (unsigned short *)0xb8000;
   while(1) {
      *p = '.' + VGA_MASK;
      for(int i=0; i<= (LOOP/2); i++){
        asm("inb $0x80");
      }
      *p = ' ' + VGA_MASK;
      for(int i=0; i<=(LOOP/2); i++){
        asm("inb $0x80");
      }
   }
}

void UserProc(void) {
   int i;
   unsigned short *p;

   while(1) {
      p = (unsigned short *)(0xb8000 + cur_pid);   //point p to (0xb8000 + offset according to PID)
      cons_printf("%d", cur_pid);                  //show 1st digit of its PID
      p++;                                         //move p to next column
      cons_printf("%d ", cur_pid);                 //show 2nd digit of its PID
      for(int i=0; i<=(LOOP/2); i++){
        asm("inb $0x80");
      }
      cons_printf("      ");                       //erase above writing
      for (int i=0; i<=(LOOP/2); i++){
        asm("inb $0x80");
      }
   }
}
