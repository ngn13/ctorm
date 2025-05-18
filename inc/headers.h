#pragma once

#include <stdint.h>
#include <stdbool.h>

#define HEADER_TABLE_SIZE 20

struct ctorm_header {
  struct ctorm_header *next;
  char                *name;
  char                *value;
  bool                 allocated;
};

typedef struct {
  uint8_t              _indx;
  struct ctorm_header *_cur;
  char                *name;
  char                *value;
} ctorm_header_pos_t;

typedef struct ctorm_header *ctorm_headers_t[HEADER_TABLE_SIZE];

#ifndef CTORM_EXPORT

#define ctorm_headers_init(headers)                                            \
  memset((headers), 0, sizeof(ctorm_headers_t))
void ctorm_headers_free(ctorm_headers_t headers);

#define ctorm_headers_start(pos) memset((pos), 0, sizeof(ctorm_header_pos_t))
bool ctorm_headers_next(ctorm_headers_t headers, ctorm_header_pos_t *pos);

bool ctorm_headers_cmp(const char *s1, const char *s2);
bool ctorm_headers_set(
    ctorm_headers_t headers, char *name, char *value, bool allocated);
char *ctorm_headers_get(ctorm_headers_t headers, char *name);
void  ctorm_headers_del(ctorm_headers_t headers, char *name);

#endif
