#include "../include/pool.h"
#include <stdlib.h>

work_t *pool_work(func_t func, void *arg) {
  work_t *work = malloc(sizeof(work_t));
  work->next = NULL;
  work->func = func;
  work->arg = arg;
  return work;
}

void pool_free(work_t *work) { free(work); }

work_t *pool_get(pool_t *tp) {
  work_t *work;
  work = tp->first;
  if (NULL == work)
    return NULL;

  tp->first = work->next;
  if (NULL == tp->first) {
    tp->last = NULL;
  }

  return work;
}

void *pool_worker(void *arg) {
  pool_t *tp = arg;
  work_t *work;

  while (true) {
    pthread_mutex_lock(&(tp->mutex));
    while (tp->first == NULL && !tp->stop)
      pthread_cond_wait(&(tp->work_lock), &(tp->mutex));

    if (tp->stop)
      break;

    work = pool_get(tp);
    tp->active++;
    pthread_mutex_unlock(&(tp->mutex));

    if (work != NULL) {
      work->func(work->arg);
      pool_free(work);
    }

    pthread_mutex_lock(&(tp->mutex));
    tp->active--;
    if (!tp->stop && tp->active == 0 && tp->first == NULL)
      pthread_cond_signal(&(tp->thread_lock));
    pthread_mutex_unlock(&(tp->mutex));
  }

  tp->all--;
  pthread_cond_signal(&(tp->thread_lock));
  pthread_mutex_unlock(&(tp->mutex));
  return NULL;
}

pool_t *pool_init(int n) {
  pool_t *tp = calloc(1, sizeof(pool_t));
  tp->all = n;

  pthread_mutex_init(&(tp->mutex), NULL);
  pthread_cond_init(&(tp->work_lock), NULL);
  pthread_cond_init(&(tp->thread_lock), NULL);

  tp->first = NULL;
  tp->last = NULL;

  pthread_t handle;
  for (int i = 0; i < n; i++) {
    pthread_create(&handle, NULL, pool_worker, tp);
    pthread_detach(handle);
  }
  return tp;
}

bool pool_add(pool_t *tp, func_t func, void *arg) {
  work_t *work = pool_work(func, arg);
  if (work == NULL)
    return false;

  pthread_mutex_lock(&(tp->mutex));
  if (tp->first == NULL) {
    tp->first = work;
    tp->last = tp->first;
  } else {
    tp->last->next = work;
    tp->last = work;
  }

  pthread_cond_broadcast(&(tp->work_lock));
  pthread_mutex_unlock(&(tp->mutex));
  return true;
}

void pool_stop(pool_t *tp) {
  pthread_mutex_lock(&(tp->mutex));

  work_t *f = tp->first;
  while (f != NULL) {
    work_t *n = f->next;
    pool_free(n);
    f = n;
  }

  tp->stop = true;
  pthread_cond_broadcast(&(tp->work_lock));
  pthread_mutex_unlock(&(tp->mutex));

  pthread_mutex_destroy(&(tp->mutex));
  pthread_cond_destroy(&(tp->work_lock));
  pthread_cond_destroy(&(tp->thread_lock));
  free(tp);
}
