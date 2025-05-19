#include "pool.h"
#include "error.h"
#include "log.h"

#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define ctorm_pool_lock()   pthread_mutex_lock(&pool->mutex)
#define ctorm_pool_unlock() pthread_mutex_unlock(&pool->mutex)
#define pool_debug(f, ...)                                                     \
  debug("(thread %p) " f, pthread_self(), ##__VA_ARGS__)

// get the next work in the queue
struct ctorm_work *_ctorm_pool_work(ctorm_pool_t *pool) {
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

  ctorm_pool_lock();
  pool->running++;
  ctorm_pool_unlock();

  while (true) {
    ctorm_pool_lock();

    // hold the thread until we get a work
    while (pool->active && NULL == pool->head) {
      pool_debug("waiting for new work");
      pthread_cond_wait(&pool->work_cond, &pool->mutex);
    }

    if (!pool->active)
      break;

    work = _ctorm_pool_work(pool);
    ctorm_pool_unlock();

    // do the work (if any)
    if (NULL == work)
      continue;

    pool_debug("picked up new work: %p", work->func);
    work->func(work->arg);
    free(work);
  }

  pool->running--;
  pthread_cond_signal(&pool->exit_cond);
  ctorm_pool_unlock();

  pool_debug("returning from the worker");
  return NULL;
}

ctorm_pool_t *ctorm_pool_new(uint32_t count) {
  ctorm_pool_t *pool = calloc(1, sizeof(ctorm_pool_t));
  pthread_t     handle;

  if (NULL == pool)
    return NULL;

  pool->active = true;
  pthread_mutex_init(&pool->mutex, NULL);
  pthread_cond_init(&pool->work_cond, NULL);
  pthread_cond_init(&pool->exit_cond, NULL);

  for (; count > 0; count--) {
    // create a new worker thread
    if (pthread_create(&handle, NULL, _ctorm_pool_worker, pool) != 0) {
      pool_debug("failed to create thread %d: %s", count, ctorm_error());
      ctorm_pool_free(pool);
      return NULL; // errno set by pthread_create()
    }

    // detach thread so the resources will be freed on exit
    if (pthread_detach(handle) != 0) {
      pool_debug("failed to deattach thread %d: %s", count, ctorm_error());
      ctorm_pool_free(pool);
      return NULL; // errno set by pthread_detach()
    }
  }

  return pool;
}

bool ctorm_pool_add(ctorm_pool_t *pool, ctorm_work_func_t func, void *arg) {
  if (NULL == pool || NULL == func || NULL == arg) {
    errno = EINVAL;
    return false;
  }

  struct ctorm_work *work = calloc(1, sizeof(struct ctorm_work));

  if (work == NULL) {
    errno = CTORM_ERR_ALLOC_FAIL;
    return false;
  }

  work->func = func;
  work->arg  = arg;

  ctorm_pool_lock();

  // add work to the queue
  if (NULL == pool->head)
    pool->tail = pool->head = work;
  else
    pool->tail = pool->tail->next = work;

  // we have new work, tell it to the bois
  pthread_cond_broadcast(&pool->work_cond);

  ctorm_pool_unlock();
  return true;
}

void ctorm_pool_free(ctorm_pool_t *pool) {
  ctorm_pool_lock();

  struct ctorm_work *cur = NULL, *next = pool->head;

  pool->head   = NULL;
  pool->active = false;

  // free and empty the work queue
  while (NULL != (cur = next)) {
    next = cur->next;
    free(cur);
  }

  // tell all the bois that work queue has been updated
  pthread_cond_broadcast(&pool->work_cond);
  ctorm_pool_unlock();

  // wait for all the threads to exit
  ctorm_pool_lock();

  while (pool->running > 0) {
    pool_debug("waiting for %d running threads", pool->running);
    pthread_cond_wait(&pool->exit_cond, &pool->mutex);
  }

  pool_debug("done waiting for threads");
  ctorm_pool_unlock();

  // free all the resources
  pthread_mutex_destroy(&pool->mutex);
  pthread_cond_destroy(&pool->work_cond);
  pthread_cond_destroy(&pool->exit_cond);
  free(pool);
}
