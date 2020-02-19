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

#include "my_types.h"

typedef enum
  {
  STATE_WAIT,
  STATE_MULT,
  STATE_ADD,
  STATE_SAVE,
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
  matrice_t * mat_1;
  matrice_t * mat_2;
  matrice_t * mat_result;
  double result;
} Product;

Product prod;

void initPendingMult(Product * prod);
int nbPendingMult(Product * prod);
void wasteTime(unsigned long ms);
void * mult(void * data);
void * add(void * data);

#endif
