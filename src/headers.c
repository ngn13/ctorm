#include "headers.h"
#include "errors.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define __tolower(c) (c | 32)

bool headers_cmp(const char *s1, const char *s2) {
  while (*s1 != 0 && *s2 != 0) {
    if (__tolower(*s1) != __tolower(*s2))
      return false;
    s1++;
    s2++;
  }

  return *s1 == 0 && *s2 == 0;
}

uint64_t headers_hasher(const char *data) {
  uint64_t sum = 0;

  for (; *data != 0; data++)
    sum += __tolower(*data);

  return sum;
}

#define headers_hash(k) (headers_hasher(k) % HEADER_TABLE_SIZE)
#define headers_list(k) (&(h[headers_hash(k)]))

void __headers_free_single(struct header *h) {
  if (h->alloced) {
    free(h->key);
    free(h->value);
  }

  free(h);
}

void __headers_free_list(struct header *cur) {
  struct header *pre = NULL;

  while (cur != NULL) {
    pre = cur;
    cur = cur->next;

    __headers_free_single(pre);
  }
}

void headers_free(headers_t h) {
  for (uint8_t i = 0; i < HEADER_TABLE_SIZE; i++)
    __headers_free_list(h[i]);
}

bool headers_next(headers_t h, header_pos_t *pos) {
  if (NULL == pos || NULL == h)
    return false;

next_node:
  if (pos->_indx >= HEADER_TABLE_SIZE) {
    return false;
  }

  if (NULL == pos->_cur)
    pos->_cur = h[pos->_indx];
  else
    pos->_cur = pos->_cur->next;

  if (NULL == pos->_cur) {
    pos->_indx++;
    goto next_node;
  }

  pos->key   = pos->_cur->key;
  pos->value = pos->_cur->value;

  return true;
}

bool __headers_add(headers_t h, char *key, char *val, bool alloced) {
  struct header *new = NULL, *cur = NULL, **head = NULL;

  if (NULL == (new = malloc(sizeof(struct header)))) {
    errno = AllocFailed;
    return false;
  }

  new->alloced = alloced;
  new->next    = NULL;
  new->value   = val;
  new->key     = key;

  if (NULL == (cur = *(head = headers_list(key)))) {
    *head = new;
    return true;
  }

  while (NULL != cur->next)
    cur = cur->next;
  cur->next = new;

  return true;
}

bool headers_set(headers_t h, char *key, char *val, bool alloced) {
  struct header **header = NULL;

  if (*(header = headers_list(key)) == NULL)
    return __headers_add(h, key, val, alloced);

  if ((*header)->alloced) {
    free((*header)->value);
    free((*header)->key);
  }

  (*header)->alloced = alloced;
  (*header)->value   = val;
  (*header)->key     = key;

  return true;
}

char *headers_get(headers_t h, char *key) {
  struct header **cur = headers_list(key);
  return NULL == *cur ? NULL : (*cur)->value;
}

void headers_del(headers_t h, char *key) {
  struct header **head = headers_list(key), *cur = NULL, *pre = NULL;

  if (NULL == (cur = *head))
    return;

  while (cur != NULL) {
    if (headers_cmp(key, cur->key))
      break;

    pre = cur;
    cur = cur->next;
  }

  if (NULL == pre)
    *head = cur->next;
  else
    pre->next = cur->next;

  __headers_free_single(cur);
}
