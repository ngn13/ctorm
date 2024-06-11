#pragma once
#include <stdbool.h>
#include <stddef.h>

// HTTP method enum
typedef enum method_t {
  METHOD_GET     = 0,
  METHOD_HEAD    = 1,
  METHOD_POST    = 2,
  METHOD_PUT     = 3,
  METHOD_DELETE  = 4,
  METHOD_OPTIONS = 5,
} method_t;

// HTTP method map
typedef struct method_map_t {
  method_t code;
  char    *name;
  bool     body;
} method_map_t;

extern method_map_t http_method_map[];

// supported HTTP versions
extern char *http_versions[];

// static values that are only calculated once for optimization
// this calculation is made in http_static_load(), which is
// called by app_new()

// most of these values are max sizes, which are used to allocate
// static buffers on stack for optimization
typedef struct http_static {
  size_t method_count; // stores the count of HTTP methods
  size_t method_max;   // stores the longest HTTP method's length

  size_t version_count; // stores the count of HTTP versions
  size_t version_len;   // stores the HTTP version length

  size_t header_max; // stores the max header size
  size_t path_max;   // stroes the max path size
  size_t body_max;   // stores the max size for HTTP body
} http_static_t;

extern http_static_t http_static;
void                 http_static_load();

// helpers for HTTP methods
int   http_method_id(char *);
char *http_method_name(int);
bool  http_method_has_body(int);

// helpers for HTTP versions
char *http_version_get(char *);
