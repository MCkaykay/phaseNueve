// syscalls.h, 159

#ifndef __SYSCALLS__
#define __SYSCALLS__

void Sleep(int);
int GetPid(void);
void SetVideo(int, int);
void Write(int, char *);
int SemInit(int);
void SemWait(int);
void SemPost(int);
void Read(int, char *);

#endif
