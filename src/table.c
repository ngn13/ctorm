#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/errors.h"
#include "../include/table.h"
#include "../include/util.h"

void table_init(table_t *t) {
  t->head = NULL;
  t->tail = NULL;
}

bool table_add(table_t *t, char *key, bool alloced) {
  table_node_t *new = malloc(sizeof(table_node_t));
  if (NULL == new) {
    errno = AllocFailed;
    return false;
  }

  new->alloced = alloced;
  new->key     = key;
  new->next    = NULL;
  new->value   = NULL;

  if (NULL == t->head) {
    t->head = new;
    t->tail = new;
    return true;
  }

  t->tail->next = new;
  t->tail       = new;
  return true;
}

bool table_set(table_t *t, char *value) {
  if (NULL == t->tail)
    return false;

  t->tail->value = value;
  return true;
}

char *table_get(table_t *t, char *key) {
  table_node_t *cur = t->head;
  while (cur) {
    if (eq(cur->key, key))
      return cur->value;
    cur = cur->next;
  }
  return NULL;
}

bool table_update(table_t *t, char *key, char *value) {
  table_node_t *cur = t->head;
  while (cur) {
    if (!eq(cur->key, key)) {
      cur = cur->next;
      continue;
    }

    if (cur->alloced)
      free(cur->value);

    cur->value = value;
    return true;
  }

  return false;
}

char **table_next(table_t *t, char **prev) {
  if (NULL == t->head)
    return NULL;

  char **ret = malloc(sizeof(char *) * 2);
  if (NULL == prev) {
    ret[0] = t->head->key;
    ret[1] = t->head->value;
    return ret;
  }

  table_node_t *cur = t->head;
  while (cur) {
    if (!eq(cur->key, prev[0])) {
      cur = cur->next;
      continue;
    }

    if (NULL == cur->next) {
      free(prev);
      return NULL;
    }

    prev[0] = cur->next->key;
    prev[1] = cur->next->value;
    return prev;
  }

  free(prev);
  return NULL;
}

void table_free_node(table_node_t *n){
  if(n->alloced){
    free(n->key);
    free(n->value);
  }
  free(n);
}

void table_free(table_t *t) {
  table_node_t *cur = t->head, *prev;

  while (cur) {
    prev = cur;
    cur  = cur->next;
    table_free_node(prev);
  }
}

bool table_del(table_t *t, char *key) {
  table_node_t *cur = t->head, *prev = NULL;
  while (cur) {
    if (!eq(cur->key, key)){
      prev = cur;
      cur = cur->next;
      continue;
    }

    if(NULL == prev)
      t->head = cur->next;
    else
      prev->next = cur->next;

    table_free_node(cur);
    return true;
  }
  return false;
}
