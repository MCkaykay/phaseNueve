// lib.h, 159

#ifndef __lib_H__
#define __lib_H__

#include "types.h" 

void Bzero(char *, int);
int QisFull(q_t *);
int QisEmpty(q_t *);
int DeQ(q_t *);
void EnQ(int, q_t *);
int mystrlen(char *);
int StrCmp(char *, char *);
void MemCpy(char *, char *, int);
int InQ(int, q_t *);
void DelQ(int, q_t *);
void ReclaimPages(int);
int Alloc(int, int, int);

#endif
