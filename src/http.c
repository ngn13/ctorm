#include "../include/http.h"
#include "../include/util.h"

#include <stdlib.h>
#include <string.h>

size_t http_method_sz = 6;
method_map_t http_method_map[] = {
    {.code = METHOD_GET, .name = "GET", .body = false},
    {.code = METHOD_HEAD, .name = "HEAD", .body = false},
    {.code = METHOD_POST, .name = "POST", .body = true},
    {.code = METHOD_PUT, .name = "PUT", .body = true},
    {.code = METHOD_DELETE, .name = "DELETE", .body = true},
    {.code = METHOD_OPTIONS, .name = "OPTIONS", .body = false},
};

int http_methodid(char *name) {
  for (int i = 0; i < http_method_sz; i++)
    if (eq(http_method_map[i].name, name))
      return http_method_map[i].code;
  return -1;
}
