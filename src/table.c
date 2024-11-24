#include "../include/errors.h"
#include "../include/table.h"
#include "../include/util.h"
#include "../include/log.h"

#include <stdint.h>
#include <stdlib.h>

#include <stdio.h>
#include <errno.h>

uint64_t __table_basic_hasher(const char *data) {
  uint64_t sum = 0;

  for (; *data != 0; data++)
    sum += *data;

  return sum;
}

void table_init(table_t *t, table_cmp_t *comparer, table_hash_t *hasher) {
  bzero(t, sizeof(table_t));
  t->comparer = comparer == NULL ? strcmp : comparer;
  t->hasher   = hasher == NULL ? __table_basic_hasher : hasher;
}

#define table_hash(d)     (t->hasher(d) % TABLE_SIZE)
#define table_list(k)     (&(t->lists[table_hash(k)]))
#define table_cmp(k1, k2) (t->comparer(k1, k2) == 0)

bool table_add(table_t *t, char *key, char *value, bool alloced) {
  table_node_t *new = NULL, *cur = NULL, **head = NULL;

  if (NULL == (new = malloc(sizeof(table_node_t)))) {
    errno = AllocFailed;
    return false;
  }

  new->alloced = alloced;
  new->value   = value;
  new->key     = key;
  new->next    = NULL;

  if (NULL == (cur = *(head = table_list(key)))) {
    *head = new;
    return true;
  }

  while (NULL != cur->next)
    cur = cur->next;

  cur->next = new;

  return true;
}

table_node_t *table_get(table_t *t, char *key) {
  table_node_t **head = NULL, *cur = NULL;
  head = table_list(key);

  for (cur = *head; NULL != cur; cur = cur->next) {
    if (table_cmp(key, cur->key))
      return cur;
  }

  return NULL;
}

bool table_next(table_t *t, table_entry_t *cur) {
  if (NULL == cur || NULL == t)
    return false;

next_node:
  if (cur->_indx >= TABLE_SIZE) {
    return false;
  }

  if (NULL == cur->_node)
    cur->_node = t->lists[cur->_indx];
  else
    cur->_node = cur->_node->next;

  if (NULL == cur->_node) {
    cur->_indx++;
    goto next_node;
  }

  cur->key   = cur->_node->key;
  cur->value = cur->_node->value;

  return true;
}

void __table_single_node_free(table_node_t *n) {
  if (n->alloced) {
    free(n->key);
    free(n->value);
  }

  free(n);
}

void __table_node_free(table_node_t *node) {
  table_node_t *pre = NULL;

  while (node != NULL) {
    pre  = node;
    node = node->next;

    __table_single_node_free(pre);
  }
}

void table_free(table_t *t) {
  for (uint8_t i = 0; i < TABLE_SIZE; i++)
    __table_node_free(t->lists[i]);
}

void table_del(table_t *t, char *key) {
  table_node_t **head = NULL, *cur = NULL, *pre = NULL;
  head = table_list(key);

  if (NULL == (cur = *head))
    return;

  while (cur != NULL) {
    if (table_cmp(key, cur->key))
      break;

    pre = cur;
    cur = cur->next;
  }

  if (NULL == pre)
    *head = cur->next;
  else
    pre->next = cur->next;

  __table_single_node_free(cur);
}
