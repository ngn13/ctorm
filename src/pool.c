#include "pool.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define ctorm_pool_lock(pool)   pthread_mutex_lock(&pool->mutex)
#define ctorm_pool_unlock(pool) pthread_mutex_unlock(&pool->mutex)

// create a new work structure
struct ctorm_work *_ctorm_work_new(ctorm_work_func_t func, void *arg) {
  struct ctorm_work *work = calloc(1, sizeof(struct ctorm_work));

  work->func = func;
  work->arg  = arg;

  return work;
}

// get the next work in the queue
struct ctorm_work *_ctorm_work(ctorm_pool_t *pool) {
  struct ctorm_work *work = pool->head;

  if (NULL == work)
    return NULL;

  if (NULL == (pool->head = work->next))
    pool->tail = NULL;

  return work;
}

// the worker function
void *_ctorm_pool_worker(void *_pool) {
  ctorm_pool_t      *pool = _pool;
  struct ctorm_work *work = NULL;

  while (true) {
    ctorm_pool_lock(pool);

    // hold the thread until we get a work
    while (NULL == pool->head && pool->active)
      pthread_cond_wait(&pool->work_lock, &pool->mutex);

    if (pool->active)
      break;

    work = _ctorm_work(pool);
    pool->busy_count++;

    ctorm_pool_unlock(pool);

    // do the work (if any)
    if (NULL != work) {
      work->func(work->arg);
      free(work);
      work = NULL;
    }

    ctorm_pool_lock(pool);

    pool->busy_count--;

    if (pool->active && pool->busy_count == 0 && NULL == pool->head)
      pthread_cond_signal(&pool->thread_lock);

    ctorm_pool_unlock(pool);
  }

  pool->total_count--;

  pthread_cond_signal(&pool->thread_lock);
  ctorm_pool_unlock(pool);

  return NULL;
}

ctorm_pool_t *ctorm_pool_init(uint64_t count) {
  ctorm_pool_t *pool = calloc(1, sizeof(ctorm_pool_t));
  pthread_t     handle;

  pool->active      = true;
  pool->total_count = count;
  pthread_mutex_init(&pool->mutex, NULL);
  pthread_cond_init(&pool->work_lock, NULL);
  pthread_cond_init(&pool->thread_lock, NULL);

  for (; count > 0; count--) {
    pthread_create(&handle, NULL, _ctorm_pool_worker, pool);
    pthread_detach(handle);
  }

  return pool;
}

bool ctorm_pool_add(ctorm_pool_t *pool, ctorm_work_func_t func, void *arg) {
  if (NULL == pool || NULL == func || NULL == arg) {
    errno = EINVAL;
    return false;
  }

  struct ctorm_work *work = _ctorm_work_new(func, arg);

  if (work == NULL)
    return false;

  ctorm_pool_lock(pool);

  // add work to the queue
  if (NULL == pool->head)
    pool->tail = pool->head = work;
  else
    pool->tail = pool->tail->next = work;

  // we have new work, tell it to the bois
  pthread_cond_broadcast(&pool->work_lock);

  ctorm_pool_unlock(pool);
  return true;
}

void ctorm_pool_stop(ctorm_pool_t *pool) {
  ctorm_pool_lock(pool);

  struct ctorm_work *cur = NULL, *next = pool->head;

  // free and empty the work queue
  while (NULL != (cur = next)) {
    next = cur->next;
    free(cur);
  }

  pool->active = false;
  pthread_cond_broadcast(&pool->work_lock);

  ctorm_pool_unlock(pool);

  pthread_mutex_destroy(&pool->mutex);
  pthread_cond_destroy(&pool->work_lock);
  pthread_cond_destroy(&pool->thread_lock);

  free(pool);
}
