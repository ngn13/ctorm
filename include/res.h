#pragma once
#include "http.h"
#include "table.h"
#include <cjson/cJSON.h>

typedef struct res_t {
  unsigned short code;
  char          *version;
  char          *body;
  size_t         bodysize;
  table_t        headers;
} res_t;

void   res_init(res_t *);
void   res_free(res_t *);
size_t res_size(res_t *);
void   res_tostr(res_t *, char *);
void   res_send(res_t *, char *, size_t);
bool   res_sendfile(res_t *, char *);
void   res_set(res_t *, char *, char *);
void   res_del(res_t *, char *);
bool   res_fmt(res_t *, const char *, ...);
bool   res_add(res_t *, const char *, ...);
bool   res_json(res_t *, cJSON *);
void   res_redirect(res_t *, char *);
void   res_clear(res_t *);
