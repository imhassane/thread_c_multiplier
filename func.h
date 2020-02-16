#ifndef SYMBOL

#define _GNU_SOURCE_
#define __FUNCTION__

#include <sys/mman.h>
#include <sched.h>
#include <pthread.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>

typedef enum
  {
  STATE_WAIT,
  STATE_MULT,
  STATE_ADD,
  STATE_PRINT
  } State;

typedef struct
{
  State state;
  int * pendingMult;
  pthread_cond_t cond;
  pthread_mutex_t mutex;
  size_t nbIterations;
  size_t size;
  double * v1;
  double * v2;
  double * v3;
  double result;
} Product;

Product prod;

void initPendingMult(Product * prod);
int nbPendingMult(Product * prod);
void wasteTime(unsigned long ms);
void * mult(void * data);
void * add(void * data);

#endif
