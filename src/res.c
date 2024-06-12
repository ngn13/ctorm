#include "../include/res.h"
#include "../include/errors.h"
#include "../include/util.h"

#include <cjson/cJSON.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void res_init(res_t *res) {
  table_init(&res->headers);
  table_init(&res->render);

  res->version  = NULL;
  res->bodysize = 0;
  res->body     = NULL;
  res->code     = 200;

  table_add(&res->headers, "Server", false);
  table_set(&res->headers, "ctorm");
  
  table_add(&res->headers, "Connection", false);
  table_set(&res->headers, "close");
  
  table_add(&res->headers, "Content-Length", false);
  table_set(&res->headers, "0");

  struct tm *gmt;
  time_t     raw;

  time(&raw);
  gmt = gmtime(&raw);

  char date[50];
  // https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Date
  strftime(date, 50, "%a, %d %b %Y %H:%M:%S GMT", gmt);
  res_set(res, "Date", date);
}

void res_free(res_t *res) {
  table_free(&res->headers);
  table_free(&res->render);
  free(res->body);
}

void res_set(res_t *res, char *name, char *value) {
  char *valuecp = strdup(value);
  if (table_update(&res->headers, name, valuecp))
    return;

  table_add(&res->headers, strdup(name), true);
  table_set(&res->headers, valuecp);
}

void res_del(res_t *res, char *name) {
  table_del(&res->headers, name);
}

void res_update_size(res_t *res) {
  int  len = digits(res->bodysize) + 1;
  char buf[len];

  snprintf(buf, len, "%lu", res->bodysize);
  res_set(res, "Content-Length", buf);
}

void res_send(res_t *res, char *data, size_t size) {
  free(res->body);
  res->bodysize = 0;

  if (NULL == data) {
    res->body = NULL;
    return;
  }

  if (size <= 0)
    res->bodysize = strlen(data);

  res->body = malloc(res->bodysize);
  memcpy(res->body, data, res->bodysize);
  res_update_size(res);
}

bool res_sendfile(res_t *res, char *path) {
  if (!file_canread(path)) {
    if (errno == ENOENT)
      errno = FileNotExists;
    else
      errno = BadReadPerm;
    return false;
  }

  free(res->body);
  res->body = NULL;

  res->bodysize = file_size(path);
  if (res->bodysize < 0) {
    errno         = SizeFail;
    res->bodysize = 0;
    return false;
  }

  res->body = malloc(res->bodysize);

  if (!file_read(path, res->body, res->bodysize)) {
    errno = CantRead;
    return false;
  }

  if (NULL == res->body)
    return false;

  if (endswith(path, ".html"))
    res_set(res, "Content-Type", "text/html; charset=utf-8");
  else if (endswith(path, ".json"))
    res_set(res, "Content-Type", "application/json; charset=utf-8");
  else if (endswith(path, ".css"))
    res_set(res, "Content-Type", "text/css; charset=utf-8");
  else if (endswith(path, ".js"))
    res_set(res, "Content-Type", "text/javascript; charset=utf-8");
  else
    res_set(res, "Content-Type", "text/plain; charset=utf-8");

  res_update_size(res);
  return true;
}

size_t res_size(res_t *res) {
  size_t size = 0;
  size += http_static.version_len + 1; // "HTTP/1.1 "
  size += 5;                           // "200\r\n"

  char **cur = table_next(&res->headers, NULL);
  while (cur) {
    size += strlen(cur[0]) + 2; // "User-Agent: "
    size += strlen(cur[1]) + 2; // "curl\r\n"
    cur = table_next(&res->headers, cur);
  }

  size += 2; // "\r\n"

  // body
  size += res->bodysize;
  return size;
}

void res_tostr(res_t *res, char *str) {
  char **cur   = table_next(&res->headers, NULL);
  size_t index = 0;

  // fix the HTTP code if its invalid
  if (res->code > 999 || res->code < 100)
    res->code = 200;

  if (NULL == res->version)
    index += sprintf(str, "HTTP/1.1 %d\r\n", res->code);
  else
    index += sprintf(str, "%s %d\r\n", res->version, res->code);

  while (cur) {
    index += sprintf(str + index, "%s: %s\r\n", cur[0], cur[1]);
    cur = table_next(&res->headers, cur);
  }

  index += sprintf(str + index, "\r\n");
  if (res->bodysize > 0)
    memcpy(str + index, res->body, res->bodysize);
}

bool res_fmt(res_t *res, const char *fmt, ...) {
  va_list args, argscp;
  bool    ret = false;

  va_start(args, fmt);
  va_copy(argscp, args);

  free(res->body);
  res->bodysize = 0;

  res->bodysize = vsnprintf(NULL, 0, fmt, args);
  res->body     = malloc(res->bodysize + 1);

  ret = vsnprintf(res->body, res->bodysize, fmt, argscp) > 0;

  res_set(res, "Content-Type", "text/plain; charset=utf-8");
  res_update_size(res);

  va_end(args);
  va_end(argscp);

  return ret;
}

bool res_add(res_t *res, const char *fmt, ...) {
  va_list args, argscp;
  bool    ret   = false;
  int     vsize = 0;

  va_start(args, fmt);
  va_copy(argscp, args);

  if (NULL == res->body || res->bodysize <= 0) {
    res_set(res, "Content-Type", "text/plain; charset=utf-8");
    res->bodysize = vsnprintf(NULL, 0, fmt, args);
    res->body     = malloc(res->bodysize + 1);
  } else {
    res->bodysize += vsize = vsnprintf(NULL, 0, fmt, args);
    res->body              = realloc(res->body, res->bodysize + 1);
  }

  ret = vsnprintf(res->body + vsize, res->bodysize - vsize, fmt, argscp) > 0;
  res_update_size(res);

  va_end(args);
  va_end(argscp);

  return ret;
}

bool res_json(res_t *res, cJSON *json) {
  if (NULL == json)
    return false;

  free(res->body);
  res->bodysize = 0;

  res->body     = cJSON_Print(json);
  res->bodysize = strlen(res->body);

  if (NULL == res->body || res->bodysize <= 0)
    return false;

  res_set(res, "Content-Type", "application/json; charset=utf-8");
  res_update_size(res);

  cJSON_Delete(json);
  return true;
}

void res_redirect(res_t *res, char *url) {
  res->code = 301;
  res_set(res, "Location", url);
}
