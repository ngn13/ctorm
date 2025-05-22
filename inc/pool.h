#pragma once
#ifndef CTORM_EXPORT

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>

typedef void (*ctorm_pool_func_t)(void *data);

struct ctorm_work {
  ctorm_pool_func_t start; // start function of the work
  ctorm_pool_func_t stop;  // stop function of the work

  void              *data; // data to pass to the functions
  struct ctorm_work *next; // next work
};

typedef struct {
  bool     active;  // is the thread pool active
  uint32_t running; // running thread count
  uint32_t total;   // total thread count
  uint32_t len;     // work queue length

  pthread_mutex_t mutex;     // locked before reading/writing from the pool
  pthread_cond_t  work_cond; // used to wait for new work
  pthread_cond_t  done_cond; // used to wait for threads to complete a work
  pthread_cond_t  exit_cond; // used to wait for threads to exit

  struct ctorm_work **ongoing; // list of ongoing (running) works
  struct ctorm_work  *head;    // head of the work queue
  struct ctorm_work  *tail;    // tail (end) of the work queue
} ctorm_pool_t;

ctorm_pool_t *ctorm_pool_new(uint32_t count);
uint32_t      ctorm_pool_remaining(ctorm_pool_t *pool);
void          ctorm_pool_wait(ctorm_pool_t *pool, uint32_t count);
bool          ctorm_pool_add(ctorm_pool_t *pool, ctorm_pool_func_t start,
             ctorm_pool_func_t stop, void *data);
void          ctorm_pool_free(ctorm_pool_t *pool);

#endif
