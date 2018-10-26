// proc.h, 159

#ifndef __PROC_H__
#define __PROC_H__

void InitProc(void);
void UserProc(void);
void CarProc(void);
void TermProc(void);
void Ouch(int);
void Wrapper(func_p_t handler_p);

#endif

