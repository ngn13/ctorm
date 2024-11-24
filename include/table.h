#pragma once

#include <stdbool.h>
#include <stdint.h>

#define TABLE_SIZE 20

typedef struct table_node {
  struct table_node *next;
  char              *key;
  char              *value;
  bool               alloced;
} table_node_t;

typedef struct {
  uint8_t       _indx;
  table_node_t *_node;
  char         *key;
  char         *value;
} table_entry_t;

typedef int(table_cmp_t)(const char *s1, const char *s2);
typedef uint64_t(table_hash_t)(const char *data);

typedef struct {
  table_node_t *lists[TABLE_SIZE];
  table_cmp_t  *comparer;
  table_hash_t *hasher;
} table_t;

void table_init(table_t *t, table_cmp_t *comparer, table_hash_t *hasher);
void table_free(table_t *t);

bool          table_add(table_t *t, char *key, char *value, bool alloced);
table_node_t *table_get(table_t *t, char *key);
void          table_del(table_t *t, char *key);

#define table_start(e) bzero(e, sizeof(table_entry_t));
bool table_next(table_t *t, table_entry_t *cur);
