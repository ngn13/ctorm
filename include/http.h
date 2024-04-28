#pragma once
#include <stdbool.h>
#include <stddef.h>

typedef enum method_t{
  METHOD_GET = 0,
  METHOD_HEAD = 1,
  METHOD_POST = 2,
  METHOD_PUT = 3,
  METHOD_DELETE = 4,
  METHOD_OPTIONS = 5,
} method_t;

typedef struct method_map_t {
  method_t code;
  char *name;
  bool body;
} method_map_t;

extern method_map_t http_method_map[];
extern size_t http_method_sz;

int http_methodid(char *);
