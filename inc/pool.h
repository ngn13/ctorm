#pragma once
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>

#ifndef CTORM_EXPORT

typedef void (*func_t)(void *arg);
typedef struct ctorm_thread {
  func_t               func;
  void                *arg;
  struct ctorm_thread *next;
} ctorm_thread_t;

#else

typedef void ctorm_thread_t;

#endif

typedef struct {
  pthread_mutex_t mutex;

  pthread_cond_t work_lock;
  pthread_cond_t thread_lock;

  size_t active;
  size_t all;

  ctorm_thread_t *first;
  ctorm_thread_t *last;
  bool            stop;
} ctorm_pool_t;

#ifndef CTORM_EXPORT

ctorm_pool_t *ctorm_pool_init(uint64_t pool_size);
bool          ctorm_pool_add(ctorm_pool_t *pool, func_t func, void *arg);
void          ctorm_pool_stop(ctorm_pool_t *pool);

#endif
