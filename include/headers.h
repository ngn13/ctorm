#pragma once

#include "table.h"

typedef table_t       headers_t;
typedef table_entry_t header_t;

int      headers_cmp(const char *s1, const char *s2);
uint64_t headers_hasher(const char *data);

#define headers_init(h) table_init(h, headers_cmp, headers_hasher)
#define headers_free(h) table_free(h)

#define headers_start(e)   table_start(e)
#define headers_next(h, e) table_next(h, e)

#define headers_add(h, k, v, a) table_add(h, k, v, a);
bool  headers_set(headers_t *h, char *key, char *value, bool dup);
char *headers_get(headers_t *h, char *key);
#define headers_del(h, k) table_del(h, k)
