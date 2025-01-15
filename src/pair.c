#include "pair.h"
#include "errors.h"
#include "util.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>

ctorm_pair_t *ctorm_pair_add(ctorm_pair_t **head, char *key, char *value) {
  if (NULL == key) {
    errno = EINVAL;
    return NULL;
  }

  ctorm_pair_t *new = malloc(sizeof(ctorm_pair_t));

  if (NULL == new) {
    errno = AllocFailed;
    return NULL;
  }

  bzero(new, sizeof(ctorm_pair_t));

  if (NULL != head) {
    new->next = *head;
    *head     = new;
  }

  new->key   = key;
  new->value = value;

  return new;
}

ctorm_pair_t *ctorm_pair_find(ctorm_pair_t *head, char *key) {
  if (NULL == head || NULL == key) {
    errno = EINVAL;
    return NULL;
  }

  ctorm_pair_next(head, cur) {
    if (cu_streq(cur->key, key))
      return cur;
  }

  errno = EFAULT;
  return NULL;
}

void ctorm_pair_free(ctorm_pair_t *head) {
  ctorm_pair_t *cur = NULL, *next = head;

  while (NULL != (cur = next)) {
    next = cur->next;
    free(cur);
  }
}
