// isr.h, 159

#ifndef __ISR_H__
#define __ISR_H__

#include "types.h"

void NewProcISR(func_p_t);
void TimerISR(void);
void GetPidISR(void);
void SleepISR(void);
void SetVideoISR(void);
void WriteISR(void);

#endif
