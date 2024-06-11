#pragma once
#include <stdbool.h>

typedef struct table_node_t {
  struct table_node_t *next;
  char                *key;
  char                *value;
  bool                 alloced;
} table_node_t;

typedef struct table_t {
  struct table_node_t *head;
  struct table_node_t *tail;
} table_t;

void   table_init(table_t *);
bool   table_add(table_t *, char *, bool);
bool   table_set(table_t *, char *);
bool   table_update(table_t *, char *, char *);
char **table_next(table_t *, char **);
char  *table_get(table_t *, char *);
void   table_free(table_t *);
bool table_del(table_t *, char *);
