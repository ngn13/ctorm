#include "pool.h"

#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

ctorm_thread_t *__ctorm_thread_new(func_t func, void *arg) {
  ctorm_thread_t *thread = malloc(sizeof(ctorm_thread_t));

  thread->next = NULL;
  thread->func = func;
  thread->arg  = arg;

  return thread;
}

#define __ctorm_thread_free(thread)                                            \
  do {                                                                         \
    free(thread);                                                              \
    thread = NULL;                                                             \
  } while (0);

#define __ctorm_pool_lock(pool)   pthread_mutex_lock(&pool->mutex)
#define __ctorm_pool_unlock(pool) pthread_mutex_unlock(&pool->mutex)

ctorm_thread_t *__ctorm_pool_get(ctorm_pool_t *pool) {
  ctorm_thread_t *thread;

  if (NULL == (thread = pool->first))
    return NULL;

  if (NULL == (pool->first = thread->next))
    pool->last = NULL;

  return thread;
}

void *__ctorm_pool_worker(void *_pool) {
  ctorm_pool_t   *pool = _pool;
  ctorm_thread_t *thread;

  while (true) {
    __ctorm_pool_lock(pool);

    while (pool->first == NULL && !pool->stop)
      pthread_cond_wait(&pool->work_lock, &pool->mutex);

    if (pool->stop)
      break;

    thread = __ctorm_pool_get(pool);
    pool->active++;

    __ctorm_pool_unlock(pool);

    if (thread != NULL) {
      thread->func(thread->arg);
      __ctorm_thread_free(thread);
    }

    __ctorm_pool_lock(pool);

    pool->active--;
    if (!pool->stop && pool->active == 0 && pool->first == NULL)
      pthread_cond_signal(&pool->thread_lock);

    __ctorm_pool_unlock(pool);
  }

  pool->all--;

  pthread_cond_signal(&pool->thread_lock);
  __ctorm_pool_unlock(pool);

  return NULL;
}

ctorm_pool_t *ctorm_pool_init(uint64_t pool_size) {
  ctorm_pool_t *pool = calloc(1, sizeof(ctorm_pool_t));
  bzero(pool, sizeof(ctorm_pool_t));

  pthread_t handle;
  pool->all = pool_size;

  pthread_mutex_init(&pool->mutex, NULL);
  pthread_cond_init(&pool->work_lock, NULL);
  pthread_cond_init(&pool->thread_lock, NULL);

  for (; pool_size > 0; pool_size--) {
    pthread_create(&handle, NULL, __ctorm_pool_worker, pool);
    pthread_detach(handle);
  }

  return pool;
}

bool ctorm_pool_add(ctorm_pool_t *pool, func_t func, void *arg) {
  ctorm_thread_t *thread = __ctorm_thread_new(func, arg);

  if (thread == NULL)
    return false;

  __ctorm_pool_lock(pool);

  if (pool->first == NULL)
    pool->last = pool->first = thread;

  else {
    pool->last->next = thread;
    pool->last       = thread;
  }

  pthread_cond_broadcast(&pool->work_lock);
  __ctorm_pool_unlock(pool);

  return true;
}

void ctorm_pool_stop(ctorm_pool_t *pool) {
  __ctorm_pool_lock(pool);
  ctorm_thread_t *cur = NULL, *next = pool->first;

  while (NULL != (cur = next)) {
    next = cur->next;
    __ctorm_thread_free(cur);
  }

  pool->stop = true;
  pthread_cond_broadcast(&pool->work_lock);
  __ctorm_pool_unlock(pool);

  pthread_mutex_destroy(&pool->mutex);
  pthread_cond_destroy(&pool->work_lock);
  pthread_cond_destroy(&pool->thread_lock);

  free(pool);
}
