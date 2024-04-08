#pragma once
#include <stdbool.h>
#include <stddef.h>

typedef enum t_method {
  METHOD_GET = 0,
  METHOD_HEAD = 1,
  METHOD_POST = 2,
  METHOD_PUT = 3,
  METHOD_DELETE = 4,
  METHOD_OPTIONS = 5,
} t_method;

typedef struct t_method_map {
  t_method code;
  char *name;
  bool body;
} t_method_map;

extern t_method_map http_method_map[];
extern size_t http_method_sz;
int http_methodid(char *);
