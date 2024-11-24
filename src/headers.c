#include "../include/headers.h"

#include <string.h>
#include <stdlib.h>

#define tolower(c) (c | 32)

int headers_cmp(const char *s1, const char *s2) {
  while (*s1 != 0 && *s2 != 0) {
    if (tolower(*s1) != tolower(*s2))
      return -1;
    s1++;
    s2++;
  }

  return (*s1 == 0 && *s2 == 0) ? 0 : -1;
}

uint64_t headers_hasher(const char *data) {
  uint64_t sum = 0;

  for (; *data != 0; data++)
    sum += tolower(*data);

  return sum;
}

bool headers_set(headers_t *h, char *key, char *value, bool dup) {
  table_node_t *node  = NULL;
  char         *valdp = value;

  if (dup)
    valdp = strdup(value);

  if ((node = table_get(h, key)) == NULL)
    return table_add(h, dup ? strdup(key) : key, valdp, true);

  if (node->alloced) {
    free(node->value);
    node->value = valdp;
    return true;
  }

  node->alloced = true;
  node->key     = dup ? strdup(key) : key;
  node->value   = valdp;

  return true;
}

char *headers_get(headers_t *h, char *key) {
  table_node_t *node = table_get(h, key);

  if (NULL == node)
    return NULL;

  return node->value;
}
