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
  bool active; // is the thread pool active

  uint64_t total_count; // total thread count
  uint64_t busy_count;  // busy thread count

  pthread_mutex_t mutex;
  pthread_cond_t  work_lock;
  pthread_cond_t  thread_lock;

  struct ctorm_work *head; // head of the work queue
  struct ctorm_work *tail; // tail (end) of the work queue
} ctorm_pool_t;

#ifndef CTORM_EXPORT

ctorm_pool_t *ctorm_pool_init(uint64_t count);
bool ctorm_pool_add(ctorm_pool_t *pool, ctorm_work_func_t func, void *arg);
void ctorm_pool_stop(ctorm_pool_t *pool);

#endif
