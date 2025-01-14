#include "headers.h"

#include "errors.h"
#include "util.h"
#include "res.h"
#include "log.h"

#include <sys/socket.h>
#include <sys/stat.h>

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <stdio.h>

#include <fcntl.h>
#include <time.h>

#define rdebug(f, ...)                                                                                                 \
  debug("(" FG_BOLD "socket " FG_CYAN "%d" FG_RESET FG_BOLD " Response " FG_CYAN "0x%p" FG_RESET ") " f,               \
      res->con->socket,                                                                                                \
      res,                                                                                                             \
      ##__VA_ARGS__)
#define rsend(b, s, f)  connection_send(res->con, b, s, f)

void __rprintf(ctorm_res_t *res, char *fmt, ...) {
  int size = 0;
  va_list args, args_cp;

  va_start(args, fmt);
  va_copy(args_cp, args);

  size = vsnprintf(NULL, 0, fmt, args) + 1;
  char buf[size];
  vsnprintf(buf, size, fmt, args_cp);

  rsend(buf, size-1, MSG_NOSIGNAL);
  va_end(args);
}

#define rprintf(f, ...) __rprintf(res, f, ##__VA_ARGS__)
//#define rprintf(f, ...) dprintf(res->con->socket, f, ##__VA_ARGS__)

void ctorm_res_init(ctorm_res_t *res, connection_t *con) {
  bzero(res, sizeof(*res));

  res->con       = con;
  res->version   = NULL;
  res->bodysize  = 0;
  res->body      = NULL;
  res->bodyfd    = -1;
  res->code      = 200;
  res->completed = false;

  ctorm_headers_init(&res->headers);
  ctorm_headers_set(res->headers, "server", "ctorm", false);
  ctorm_headers_set(res->headers, "connection", "close", false);

  struct tm *gmt;
  time_t     raw;

  time(&raw);
  gmt = gmtime(&raw);

  char date[50];
  // https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Date
  strftime(date, 50, "%a, %d %b %Y %H:%M:%S GMT", gmt);
  ctorm_res_set(res, "date", date);
}

void ctorm_res_free(ctorm_res_t *res) {
  ctorm_headers_free(res->headers);
  ctorm_res_clear(res);
}

void ctorm_res_set(ctorm_res_t *res, char *name, char *value) {
  if (NULL == name || NULL == value)
    errno = BadHeaderPointer;
  else
    ctorm_headers_set(res->headers, strdup(name), strdup(value), true);
}

void ctorm_res_del(ctorm_res_t *res, char *name) {
  if (NULL == name) {
    errno = BadHeaderPointer;
    return;
  }

  ctorm_headers_del(res->headers, name);
}

void ctorm_res_clear(ctorm_res_t *res) {
  free(res->body);

  if (res->bodyfd > 0)
    close(res->bodyfd);

  res->body     = NULL;
  res->bodyfd   = -1;
  res->bodysize = 0;
}

void ctorm_res_send(ctorm_res_t *res, char *data, uint64_t size) {
  if (NULL == data) {
    errno = BadDataPointer;
    return;
  }

  ctorm_res_clear(res);

  if (size <= 0)
    res->bodysize = cu_strlen(data);

  res->body = malloc(res->bodysize);
  memcpy(res->body, data, res->bodysize);
}

bool ctorm_res_sendfile(ctorm_res_t *res, char *path) {
  if (NULL == path) {
    errno = BadPathPointer;
    return false;
  }

  struct stat buf;
  ctorm_res_clear(res);

  if ((res->bodyfd = open(path, O_RDONLY)) < 0) {
    switch (errno) {
    case ENOENT:
      errno = FileNotExists;
      break;

    case EPERM:
      errno = BadReadPerm;
      break;
    }

    // otherwise the errno is set by open()
    return false;
  }

  if (fstat(res->bodyfd, &buf) != 0) {
    errno = SizeFail;
    return false;
  }

  res->bodysize = buf.st_size;

  // HACK: maybe a structure that stores extensions and types would be better
  if (cu_endswith(path, ".html"))
    ctorm_res_set(res, "content-type", "text/html; charset=utf-8");
  else if (cu_endswith(path, ".json"))
    ctorm_res_set(res, "content-type", "application/json; charset=utf-8");
  else if (cu_endswith(path, ".css"))
    ctorm_res_set(res, "content-type", "text/css; charset=utf-8");
  else if (cu_endswith(path, ".js"))
    ctorm_res_set(res, "content-type", "text/javascript; charset=utf-8");
  else
    ctorm_res_set(res, "content-type", "text/plain; charset=utf-8");

  return true;
}

bool ctorm_res_fmt(ctorm_res_t *res, const char *fmt, ...) {
  if (NULL == fmt) {
    errno = BadFmtPointer;
    return false;
  }

  va_list args, argscp;
  bool    ret = false;

  va_start(args, fmt);
  va_copy(argscp, args);

  ctorm_res_clear(res);

  res->bodysize = vsnprintf(NULL, 0, fmt, args);
  res->body     = malloc(res->bodysize + 1);

  ret = vsnprintf(res->body, res->bodysize + 1, fmt, argscp) > 0;

  ctorm_res_set(res, "content-type", "text/plain; charset=utf-8");

  va_end(args);
  va_end(argscp);

  return ret;
}

bool ctorm_res_add(ctorm_res_t *res, const char *fmt, ...) {
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
    ctorm_res_set(res, "content-type", "text/plain; charset=utf-8");
    vsize     = vsnprintf(NULL, 0, fmt, args);
    res->body = malloc(res->bodysize + vsize + 1);
  } else {
    vsize     = vsnprintf(NULL, 0, fmt, args);
    res->body = realloc(res->body, res->bodysize + vsize);
  }

  ret = vsnprintf(res->body + res->bodysize, (res->bodysize + 1) + vsize, fmt, argscp) > 0;
  res->bodysize += vsize;

  va_end(args);
  va_end(argscp);

  return ret;
}

bool ctorm_res_json(ctorm_res_t *res, cJSON *json) {
  if (NULL == json) {
    errno = BadJsonPointer;
    return false;
  }

  ctorm_res_clear(res);

  if ((res->body = ctorm_json_dump(json, &res->bodysize)) == NULL)
    return false;

  ctorm_res_set(res, "content-type", "application/json; charset=utf-8");
  return true;
}

void ctorm_res_redirect(ctorm_res_t *res, char *url) {
  if (NULL == url) {
    errno = BadUrlPointer;
    return;
  }

  res->code = 301;
  ctorm_res_set(res, "location", url);
}

bool ctorm_res_end(ctorm_res_t *res) {
  if (res->completed) {
    errno = ResponseAlreadySent;
    return false;
  }

  ctorm_header_pos_t pos;

  // fix the HTTP code if its invalid
  if (res->code > http_static.res_code_max || res->code < http_static.res_code_min) {
    errno = BadResponseCode;
    return false;
  }

  // send the HTTP response
  if (NULL == res->version)
    rprintf("HTTP/1.1 %u\r\n", res->code);
  else
    rprintf("%s %u\r\n", res->version, res->code);

  // send response headers
  ctorm_headers_start(&pos);

  while (ctorm_headers_next(res->headers, &pos))
    rprintf("%s: %s\r\n", pos.name, pos.value);
  rprintf("content-length: %lu\r\n", res->bodysize);
  rprintf("\r\n");

  // send the body
  if (res->bodyfd > 0) {
    size_t read_size = 0;
    char   read_buf[50];

    while ((read_size = read(res->bodyfd, read_buf, sizeof(read_buf))) > 0)
      rsend(read_buf, read_size, 0);
  }

  else if (res->bodysize > 0)
    rsend(res->body, res->bodysize, 0);

  res->completed = true;
  return true;
}
