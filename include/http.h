#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

// HTTP method enum
enum {
  METHOD_GET     = 0,
  METHOD_HEAD    = 1,
  METHOD_POST    = 2,
  METHOD_PUT     = 3,
  METHOD_DELETE  = 4,
  METHOD_OPTIONS = 5,
};

typedef int8_t method_t;

// HTTP method map
typedef struct {
  method_t code;
  char    *name;
  bool     body;
} method_map_t;

extern method_map_t http_method_map[];

// supported HTTP versions
extern const char *http_versions[];

/*

 * static values that are only calculated once for optimization
 * this calculation is made in http_static_load(), which is
 * called by app_new()

 * most of these values are max sizes, which are used to allocate
 * static buffers on stack for optimization

*/
typedef struct {
  uint8_t method_count; // stores the count of HTTP methods
  uint8_t method_max;   // stores the longest HTTP method's length

  uint8_t version_count; // stores the count of HTTP versions
  uint8_t version_len;   // stores the HTTP version length

  uint64_t header_max; // stores the max header size
  uint64_t path_max;   // stroes the max path size
  uint64_t body_max;   // stores the max size for HTTP body

  uint16_t res_code_min; // stores the minimum HTTP response code value
  uint16_t res_code_max; // stores the maximum HTTP response code value
} http_static_t;

extern http_static_t http_static;
void                 http_static_load();

method_t http_method_id(char *);
char    *http_method_name(int);
bool     http_method_has_body(int);

#define http_is_valid_header_char(c) (is_digit(c) || is_letter(c) || contains("_ :;.,\\/\"'?!(){}[]@<>=-+*#$&`|~^%", c))
#define http_is_valid_path_char(c)   (is_digit(c) || is_letter(c) || contains("-._~:/?#[]@!$&'()*+,;%=", c))

const char *http_version_get(char *);
