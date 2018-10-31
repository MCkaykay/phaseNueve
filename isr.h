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
void SemInitISR(void);
void SemWaitISR(void);
void SemPostISR(void);
void TermISR(int);
void TermTxISR(int);
void ReadISR(void);
void TermRxISR(int);
void WrapperISR(int, func_p_t);
void SignalISR(void);

#endif
