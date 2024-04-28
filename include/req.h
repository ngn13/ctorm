#pragma once
#include "http.h"
#include "table.h"

typedef struct req_t {
  method_t method;
  char *fullpath;
  char *encpath;
  char *path;
  char *version;
  char *body;
  size_t bodysz;
  table_t headers;
  table_t query;
} req_t;

typedef table_t body_t;

void req_init(req_t *);
void req_free(req_t *);
int req_size(req_t *);
void req_tostr(req_t *, char *);

bool req_has_body(req_t *);
bool req_can_have_body(req_t *);
int req_body_size(req_t *);

char *req_method(req_t *);
char *req_body(req_t *);
char *req_query(req_t *, char *);
char *req_header(req_t *, char *);

body_t *req_body_parse(req_t *);
char *req_body_get(body_t *, char *);
void req_body_free(body_t *);

void req_add_header(req_t *, char *);
void req_add_header_value(req_t *, char *);
