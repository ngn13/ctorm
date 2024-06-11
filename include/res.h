#pragma once
#include "http.h"
#include "table.h"

typedef struct res_t {
  unsigned short code;
  char   *version;
  char   *body;
  size_t  bodysize;
  table_t headers;
  table_t render;
} res_t;

void res_init(res_t *);
void res_free(res_t *);
size_t res_size(res_t *);
void res_tostr(res_t *, char *);
void res_send(res_t *, char *, size_t);
bool res_sendfile(res_t *, char *);
void res_set(res_t *, char *, char *);
void res_del(res_t *, char *);
