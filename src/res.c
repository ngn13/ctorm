#include "../include/errors.h"
#include "../include/log.h"
#include "../include/util.h"
#include "../include/res.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <stdio.h>

#include <time.h>

void res_init(res_t *res) {
  headers_init(&res->headers);

  res->version  = NULL;
  res->bodysize = 0;
  res->body     = NULL;
  res->code     = 200;

  headers_add(&res->headers, "Server", "ctorm", false);
  headers_add(&res->headers, "Connection", "close", false);
  headers_add(&res->headers, "Content-Length", "0", false);

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
  headers_free(&res->headers);
  free(res->body);
}

void res_set(res_t *res, char *name, char *value) {
  if (NULL == name || NULL == value)
    errno = BadHeaderPointer;
  else
    headers_set(&res->headers, name, value, true);
}

void res_del(res_t *res, char *name) {
  if (NULL == name) {
    errno = BadHeaderPointer;
    return;
  }

  headers_del(&res->headers, name);
}

void res_update_size(res_t *res) {
  int  len = digits(res->bodysize) + 1;
  char buf[len];

  snprintf(buf, len, "%lu", res->bodysize);
  res_set(res, "Content-Length", buf);
}

void res_clear(res_t *res) {
  free(res->body);
  res->body     = NULL;
  res->bodysize = 0;
}

void res_send(res_t *res, char *data, size_t size) {
  if (NULL == data) {
    errno = BadDataPointer;
    return;
  }

  res_clear(res);
  if (size <= 0)
    res->bodysize = strlen(data);

  res->body = malloc(res->bodysize);
  memcpy(res->body, data, res->bodysize);
  res_update_size(res);
}

bool res_sendfile(res_t *res, char *path) {
  if (NULL == path) {
    errno = BadPathPointer;
    return false;
  }

  if (!file_canread(path)) {
    if (errno == ENOENT)
      errno = FileNotExists;
    else
      errno = BadReadPerm;
    return false;
  }

  res_clear(res);

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
  size_t   size = 0;
  header_t cur;

  size += http_static.version_len + 1; // "HTTP/1.1 "
  size += 5;                           // "200\r\n"

  headers_start(&cur);

  while (headers_next(&res->headers, &cur)) {
    size += strlen(cur.key) + 2;   // "User-Agent: "
    size += strlen(cur.value) + 2; // "curl\r\n"
  }

  size += 2; // "\r\n"

  // body
  size += res->bodysize;
  return size;
}

void res_tostr(res_t *res, char *str) {
  size_t   index = 0;
  header_t cur;

  // fix the HTTP code if its invalid
  if (res->code > 999 || res->code < 100)
    res->code = 200;

  if (NULL == res->version)
    index += sprintf(str, "HTTP/1.1 %d\r\n", res->code);
  else
    index += sprintf(str, "%s %d\r\n", res->version, res->code);

  headers_start(&cur);

  while (headers_next(&res->headers, &cur))
    index += sprintf(str + index, "%s: %s\r\n", cur.key, cur.value);

  index += sprintf(str + index, "\r\n");

  if (res->bodysize > 0)
    memcpy(str + index, res->body, res->bodysize);
}

bool res_fmt(res_t *res, const char *fmt, ...) {
  if (NULL == fmt) {
    errno = BadFmtPointer;
    return false;
  }

  va_list args, argscp;
  bool    ret = false;

  va_start(args, fmt);
  va_copy(argscp, args);

  res_clear(res);

  res->bodysize = vsnprintf(NULL, 0, fmt, args);
  res->body     = malloc(res->bodysize + 1);

  ret = vsnprintf(res->body, res->bodysize + 1, fmt, argscp) > 0;

  res_set(res, "Content-Type", "text/plain; charset=utf-8");
  res_update_size(res);

  va_end(args);
  va_end(argscp);

  return ret;
}

bool res_add(res_t *res, const char *fmt, ...) {
  if (NULL == fmt) {
    errno = BadFmtPointer;
    return false;
  }

  va_list args, argscp;
  bool    ret   = false;
  int     vsize = 0;

  va_start(args, fmt);
  va_copy(argscp, args);

  if (NULL == res->body || res->bodysize <= 0) {
    res_set(res, "Content-Type", "text/plain; charset=utf-8");
    vsize     = vsnprintf(NULL, 0, fmt, args);
    res->body = malloc(res->bodysize + vsize + 1);
  } else {
    vsize     = vsnprintf(NULL, 0, fmt, args);
    res->body = realloc(res->body, res->bodysize + vsize);
  }

  ret = vsnprintf(res->body + res->bodysize, (res->bodysize + 1) + vsize, fmt, argscp) > 0;
  res->bodysize += vsize;
  res_update_size(res);

  va_end(args);
  va_end(argscp);

  return ret;
}

bool res_json(res_t *res, cJSON *json) {
  if (NULL == json) {
    errno = BadJsonPointer;
    return false;
  }

  res_clear(res);

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
  if (NULL == url) {
    errno = BadUrlPointer;
    return;
  }

  res->code = 301;
  res_set(res, "Location", url);
}
