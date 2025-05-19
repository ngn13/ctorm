#include "pair.h"
#include "error.h"
#include "util.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>

ctorm_pair_t *ctorm_pair_add(ctorm_pair_t **head, char *key, char *value) {
  if (NULL == head || NULL == key) {
    errno = EINVAL;
    return NULL;
  }

  ctorm_pair_t *new = calloc(1, sizeof(ctorm_pair_t));

  if (NULL == new) {
    errno = CTORM_ERR_ALLOC_FAIL;
    return NULL;
  }

  new->key   = key;
  new->value = value;
  new->next  = *head;

  return *head = new;
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
