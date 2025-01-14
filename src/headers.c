#include "headers.h"
#include "errors.h"
#include "util.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>

uint64_t __ctorm_headers_hasher(const char *data) {
  uint64_t sum = 0;

  for (; *data != 0; data++)
    sum += cu_lower(*data);

  return sum;
}

#define __ctorm_headers_hash(k) (__ctorm_headers_hasher(k) % HEADER_TABLE_SIZE)
#define __ctorm_headers_list(k) (&(headers[__ctorm_headers_hash(k)]))

bool ctorm_headers_cmp(const char *s1, const char *s2) {
  while (*s1 != 0 && *s2 != 0) {
    if (cu_lower(*s1) != cu_lower(*s2))
      return false;
    s1++;
    s2++;
  }

  return *s1 == 0 && *s2 == 0;
}

void __ctorm_headers_free_single(struct ctorm_header *header) {
  if (header->alloced) {
    free(header->name);
    free(header->value);
  }

  free(header);
}

void __ctorm_headers_free_list(struct ctorm_header *cur) {
  struct ctorm_header *next = cur;

  while (NULL != (cur = next)) {
    next = cur->next;
    __ctorm_headers_free_single(cur);
  }
}

void ctorm_headers_free(ctorm_headers_t headers) {
  for (uint8_t i = 0; i < HEADER_TABLE_SIZE; i++)
    __ctorm_headers_free_list(headers[i]);
}

bool ctorm_headers_next(ctorm_headers_t headers, ctorm_header_pos_t *pos) {
  if (NULL == pos || NULL == headers)
    return false;

next_node:
  if (pos->_indx >= HEADER_TABLE_SIZE) {
    return false;
  }

  if (NULL == pos->_cur)
    pos->_cur = headers[pos->_indx];
  else
    pos->_cur = pos->_cur->next;

  if (NULL == pos->_cur) {
    pos->_indx++;
    goto next_node;
  }

  pos->name  = pos->_cur->name;
  pos->value = pos->_cur->value;

  return true;
}

bool __ctorm_headers_add(ctorm_headers_t headers, char *name, char *value, bool alloced) {
  struct ctorm_header *new = NULL, *cur = NULL, **head = NULL;

  if (NULL == (new = malloc(sizeof(struct ctorm_header)))) {
    errno = AllocFailed;
    return false;
  }

  new->alloced = alloced;
  new->next    = NULL;
  new->name    = name;
  new->value   = value;

  if (NULL == (cur = *(head = __ctorm_headers_list(name)))) {
    *head = new;
    return true;
  }

  while (NULL != cur->next)
    cur = cur->next;
  cur->next = new;

  return true;
}

bool ctorm_headers_set(ctorm_headers_t headers, char *name, char *value, bool alloced) {
  struct ctorm_header **header = NULL;

  if (*(header = __ctorm_headers_list(name)) == NULL)
    return __ctorm_headers_add(headers, name, value, alloced);

  if ((*header)->alloced) {
    free((*header)->value);
    free((*header)->name);
  }

  (*header)->alloced = alloced;
  (*header)->value   = value;
  (*header)->name    = name;

  return true;
}

char *ctorm_headers_get(ctorm_headers_t headers, char *name) {
  struct ctorm_header **cur = __ctorm_headers_list(name);
  return NULL == *cur ? NULL : (*cur)->value;
}

void ctorm_headers_del(ctorm_headers_t headers, char *key) {
  struct ctorm_header **head = __ctorm_headers_list(key), *cur = NULL, *pre = NULL;

  if (NULL == (cur = *head))
    return;

  while (cur != NULL) {
    if (ctorm_headers_cmp(key, cur->name))
      break;

    pre = cur;
    cur = cur->next;
  }

  if (NULL == pre)
    *head = cur->next;
  else
    pre->next = cur->next;

  __ctorm_headers_free_single(cur);
}
