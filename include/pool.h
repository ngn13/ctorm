#pragma once
#include <pthread.h>
#include <stdbool.h>

typedef void (*func_t)(void *arg);
typedef struct work_t {
  func_t         func;
  void          *arg;
  struct work_t *next;
} work_t;

typedef struct pool_t {
  pthread_mutex_t mutex;

  pthread_cond_t work_lock;
  pthread_cond_t thread_lock;

  size_t active;
  size_t all;

  work_t *first;
  work_t *last;
  bool    stop;
} pool_t;

pool_t *pool_init(int);
bool    pool_add(pool_t *, func_t, void *);
void    pool_stop(pool_t *);
