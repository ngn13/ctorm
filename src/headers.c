#include "headers.h"
#include "error.h"
#include "util.h"
#include "log.h"

#include <string.h>
#include <stdlib.h>

uint64_t _headers_hasher(const char *data) {
  uint64_t sum = 0;

  for (; *data != 0; data++)
    sum += cu_lower(*data);

  return sum;
}

#define headers_hash(k) (_headers_hasher(k) % HEADER_TABLE_SIZE)
#define headers_list(k) (&(headers[headers_hash(k)]))

void _headers_free_single(struct ctorm_header *header) {
  if (header->allocated) {
    free(header->name);
    free(header->value);
  }

  free(header);
}

void _headers_free_list(struct ctorm_header *cur) {
  struct ctorm_header *next = cur;

  while (NULL != (cur = next)) {
    next = cur->next;
    _headers_free_single(cur);
  }
}

bool ctorm_headers_cmp(const char *s1, const char *s2) {
  while (*s1 != 0 && *s2 != 0) {
    if (cu_lower(*s1) != cu_lower(*s2))
      return false;
    s1++;
    s2++;
  }

  return *s1 == 0 && *s2 == 0;
}

void ctorm_headers_free(ctorm_headers_t headers) {
  for (uint8_t i = 0; i < HEADER_TABLE_SIZE; i++)
    _headers_free_list(headers[i]);
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

bool ctorm_headers_set(
    ctorm_headers_t headers, char *name, char *value, bool allocated) {
  struct ctorm_header *new = NULL, **head = NULL;

  if (NULL == (new = malloc(sizeof(struct ctorm_header)))) {
    errno = CTORM_ERR_ALLOC_FAIL;
    return false;
  }

  new->allocated = allocated;
  new->next      = NULL;
  new->name      = name;
  new->value     = value;

  if (NULL != *(head = headers_list(name)))
    new->next = *head;

  *head = new;
  return true;
}

char *ctorm_headers_get(ctorm_headers_t headers, char *name) {
  struct ctorm_header **head = headers_list(name), *cur = NULL;

  for (cur = *head; cur != NULL; cur = cur->next) {
    if (ctorm_headers_cmp(cur->name, name))
      return cur->value;
  }

  return NULL;
}

void ctorm_headers_del(ctorm_headers_t headers, char *name) {
  struct ctorm_header **head = headers_list(name), *cur = NULL, *pre = NULL;

  if (NULL == (cur = *head))
    return;

  while (cur != NULL) {
    if (ctorm_headers_cmp(cur->name, name))
      break;

    pre = cur;
    cur = cur->next;
  }

  if (NULL == pre)
    *head = cur->next;
  else
    pre->next = cur->next;

  _headers_free_single(cur);
}
