#include "../include/http.h"
#include "../include/util.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

method_map_t http_method_map[] = {
    {.code = METHOD_GET,     .name = "GET",     .body = false},
    {.code = METHOD_HEAD,    .name = "HEAD",    .body = false},
    {.code = METHOD_POST,    .name = "POST",    .body = true },
    {.code = METHOD_PUT,     .name = "PUT",     .body = true },
    {.code = METHOD_DELETE,  .name = "DELETE",  .body = true },
    {.code = METHOD_OPTIONS, .name = "OPTIONS", .body = false},
};

const char *http_versions[] = {"HTTP/1.1", "HTTP/1.0"};

http_static_t http_static;

void http_static_load() {
  http_static.method_count = sizeof(http_method_map) / sizeof(http_method_map[0]);
  http_static.method_max   = strlen(http_method_map[0].name);

  http_static.version_count = sizeof(http_versions) / sizeof(char *);
  http_static.version_len   = strlen(http_versions[0]);

  http_static.header_max = getpagesize();
  http_static.body_max   = getpagesize();
  http_static.path_max   = 2000;

  http_static.res_code_min = 100; // 100 Continue
  http_static.res_code_max = 511; // 511 Network Authentication Required

  for (int i = 1; i < http_static.method_count; i++) {
    size_t cur_len = strlen(http_method_map[i].name);
    if (http_static.method_max < cur_len)
      http_static.method_max = cur_len;
  }
}

method_t http_method_id(char *name) {
  for (int i = 0; i < http_static.method_count; i++)
    if (eq(http_method_map[i].name, name))
      return http_method_map[i].code;
  return -1;
}

char *http_method_name(int code) {
  for (int i = 0; i < http_static.method_count; i++)
    if (http_method_map[i].code == code)
      return http_method_map[i].name;
  return NULL;
}

bool http_method_has_body(int code) {
  for (int i = 0; i < http_static.method_count; i++)
    if (http_method_map[i].code == code)
      return http_method_map[i].body;
  return false;
}

const char *http_version_get(char *version) {
  for (int i = 0; i < http_static.version_count; i++)
    if (eq(http_versions[i], version))
      return http_versions[i];
  return NULL;
}
