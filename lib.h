// lib.h, 159

#ifndef __lib_H__
#define __lib_H__

void Bzero(char *p, int types);
int QisFull(q_t *p);
int QisEmpty(q_t *p);
int DeQ(q_t *p);
void EnQ(int to_add, q_t *p);

#endif
