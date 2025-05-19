#pragma once
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>

#ifndef CTORM_EXPORT

typedef void (*ctorm_work_func_t)(void *arg);
struct ctorm_work {
  ctorm_work_func_t  func;
  void              *arg;
  struct ctorm_work *next;
};

#else

typedef void ctorm_thread_t;

#endif

typedef struct {
  bool     active;  // is the thread pool active
  uint32_t running; // total running thread count

  pthread_mutex_t mutex;     // locked before reading/writing from the pool
  pthread_cond_t  work_cond; // use to wait for work
  pthread_cond_t  exit_cond; // use to wait for threads to exit

  struct ctorm_work *head; // head of the work queue
  struct ctorm_work *tail; // tail (end) of the work queue
} ctorm_pool_t;

#ifndef CTORM_EXPORT

ctorm_pool_t *ctorm_pool_new(uint32_t count);
bool ctorm_pool_add(ctorm_pool_t *pool, ctorm_work_func_t func, void *arg);
void ctorm_pool_free(ctorm_pool_t *pool);

#endif
