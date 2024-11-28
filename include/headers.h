#pragma once

#include <stdint.h>
#include <stdbool.h>

#define TABLE_SIZE 20

struct header {
  struct header *next;
  char          *key;
  char          *value;
  bool           alloced;
};

typedef struct {
  uint8_t        _indx;
  struct header *_cur;
  char          *key;
  char          *value;
} header_pos_t;

typedef struct header *headers_t[TABLE_SIZE];

bool     headers_cmp(const char *s1, const char *s2);
uint64_t headers_hasher(const char *data);

#define headers_init(h) bzero(h, sizeof(headers_t))
void headers_free(headers_t);

#define headers_start(p) bzero(p, sizeof(header_pos_t))
bool headers_next(headers_t, header_pos_t *);

bool  headers_set(headers_t, char *, char *, bool);
char *headers_get(headers_t, char *);
void  headers_del(headers_t, char *);
