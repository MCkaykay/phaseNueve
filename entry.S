// entry.S, entries to kernel services, build process trapframe

#include "constants.h"
#include <spede/machine/asmacros.h> // do not use/include "spede.h"
#define K_STACK_SIZE (16*2^10)      // kernel has 16KB stack

.comm k_stack, K_STACK_SIZE, 1      // declare kernel stack space (16K x 1B)
.text

// loads proc TF to run it
ENTRY(Loader)
   movl 4(%esp), %ecx
   movl %ecx, %esp
   popa
   iret

// build proc frame, and go into kernel
ENTRY(TimerEntry)                  // will have efl, cs, eip pushed by hardware
   pusha
   movl %esp, %ecx
   movw $0x10, %ax                 // set kernel data segment registers
   mov %ax, %ds                    // set kernel data segment registers
   mov %ax, %es                    // set kernel data segment registers
   leal k_stack + K_STACK_SIZE, %esp  // set esp to top of kernel stack
   pushl ecx
   cld
   call CNAME(TheKernal)

