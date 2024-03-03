#pragma once
#include "http.h"

typedef struct res_t {
  int code;
  char* version;
  char* body;
  pairs_t headers;
  pairs_t render;
} res_t;

void res_init(res_t*);
void res_free(res_t*);
int res_size(res_t*);
void res_tostr(res_t*, char*);
void res_send(res_t*, char*);
void res_render_add(res_t*, char*, char*);
bool res_render(res_t*, char*);
bool res_sendfile(res_t*, char*);
void res_set(res_t*, char*, char*);
void res_set_version(res_t*, char*);
