#include "encoding.h"
#include "error.h"

#include "http.h"
#include "util.h"

#include "res.h"
#include "log.h"

#include <sys/socket.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>

#define res_debug(f, ...)                                                      \
  debug("(" FG_BOLD "socket " FG_CYAN "%d" FG_RESET FG_BOLD                    \
        " response " FG_CYAN "0x%p" FG_RESET ") " f,                           \
      res->socket,                                                             \
      res,                                                                     \
      ##__VA_ARGS__)

#define res_send(b, s, f) send(res->socket, b, s, f)

bool _ctorm_res_send_str(ctorm_res_t *res, char *str) {
  if (NULL == str)
    return true;

  for (; *str != 0; str++)
    res_send(str, 1, MSG_NOSIGNAL);

  return false;
}

void _ctorm_res_send_fmt(ctorm_res_t *res, char *fmt, ...) {
  int     size = 0;
  va_list args, args_cp;

  va_start(args, fmt);
  va_copy(args_cp, args);

  size = vsnprintf(NULL, 0, fmt, args) + 1;
  char buf[size];
  vsnprintf(buf, size, fmt, args_cp);

  res_send(buf, size - 1, MSG_NOSIGNAL);

  va_end(args);
  va_end(args_cp);
}

#define res_send_str(str)      _ctorm_res_send_str(res, str)
#define res_send_fmt(fmt, ...) _ctorm_res_send_fmt(res, fmt, ##__VA_ARGS__)

void ctorm_res_init(ctorm_res_t *res, int socket, struct sockaddr *addr) {
  if (NULL == res || NULL == addr)
    return;

  memset(res, 0, sizeof(*res));

  res->socket = socket;
  memcpy(&res->addr, addr, sizeof(res->addr));
  res->body_size = 0;
  res->body      = NULL;
  res->body_fd   = -1;
  res->code      = 200;

  ctorm_headers_init(&res->headers);
  ctorm_headers_set(res->headers, CTORM_HTTP_SERVER, "ctorm", false);

  struct tm *gmt;
  time_t     raw;

  time(&raw);
  gmt = gmtime(&raw);

  char date[50];
  // https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Date
  strftime(date, 50, "%a, %d %b %Y %H:%M:%S GMT", gmt);
  ctorm_res_set(res, CTORM_HTTP_DATE, date);
}

void ctorm_res_free(ctorm_res_t *res) {
  ctorm_headers_free(res->headers);
  ctorm_res_clear(res);
}

bool ctorm_res_code(ctorm_res_t *res, uint16_t code) {
  if (!ctorm_http_code_is_valid(code)) {
    errno = CTORM_ERR_BAD_RESPONSE_CODE;
    return false;
  }

  res->code = code;
  return true;
}

void ctorm_res_set(ctorm_res_t *res, char *name, char *value) {
  if (NULL == name || NULL == value)
    errno = CTORM_ERR_BAD_HEADER_PTR;
  else
    ctorm_headers_set(res->headers, strdup(name), strdup(value), true);
}

void ctorm_res_del(ctorm_res_t *res, char *name) {
  if (NULL == name) {
    errno = CTORM_ERR_BAD_HEADER_PTR;
    return;
  }

  ctorm_headers_del(res->headers, name);
}

void ctorm_res_clear(ctorm_res_t *res) {
  free(res->body);

  if (res->body_fd > 0)
    close(res->body_fd);

  res->body      = NULL;
  res->body_fd   = -1;
  res->body_size = 0;
}

uint32_t ctorm_res_body(ctorm_res_t *res, char *data, uint32_t size) {
  if (NULL == data) {
    errno = CTORM_ERR_BAD_DATA_PTR;
    return 0;
  }

  ctorm_res_clear(res);

  if (size <= 0)
    res->body_size = cu_strlen(data);

  if (res->body_size <= 0)
    return 0;

  if (NULL == (res->body = malloc(res->body_size))) {
    errno          = CTORM_ERR_ALLOC_FAIL;
    res->body_size = 0;
    return 0;
  }

  memcpy(res->body, data, res->body_size);
  return res->body_size;
}

bool ctorm_res_file(ctorm_res_t *res, char *path) {
  if (NULL == path) {
    errno = CTORM_ERR_BAD_PATH_PTR;
    return false;
  }

  ctorm_res_clear(res);

  if ((res->body_fd = open(path, O_RDONLY)) < 0) {
    switch (errno) {
    case ENOENT:
      errno = CTORM_ERR_NOT_EXISTS;
      break;

    case EPERM:
      errno = CTORM_ERR_NO_READ_PERM;
      break;
    }

    // otherwise the errno is set by open()
    return false;
  }

  // get the current and the end offset
  off_t cur = lseek(res->body_fd, 0, SEEK_CUR);
  off_t end = lseek(res->body_fd, 0, SEEK_END);

  if (cur < 0 || end < 0 || lseek(res->body_fd, cur, SEEK_SET) < 0) {
    errno = CTORM_ERR_SEEK_FAIL;

    close(res->body_fd);
    res->body_fd = -1;

    return false;
  }

  res->body_size = end - cur;

  // HACK: maybe a structure that stores extensions and types would be better
  if (cu_endswith(path, ".html"))
    ctorm_res_set(res, CTORM_HTTP_CONTENT_TYPE, "text/html; charset=utf-8");
  else if (cu_endswith(path, ".json"))
    ctorm_res_set(
        res, CTORM_HTTP_CONTENT_TYPE, "application/json; charset=utf-8");
  else if (cu_endswith(path, ".css"))
    ctorm_res_set(res, CTORM_HTTP_CONTENT_TYPE, "text/css; charset=utf-8");
  else if (cu_endswith(path, ".js"))
    ctorm_res_set(
        res, CTORM_HTTP_CONTENT_TYPE, "text/javascript; charset=utf-8");
  else
    ctorm_res_set(res, CTORM_HTTP_CONTENT_TYPE, "text/plain; charset=utf-8");

  return true;
}

int ctorm_res_fmt(ctorm_res_t *res, const char *fmt, ...) {
  if (NULL == fmt) {
    errno = CTORM_ERR_BAD_FMT_PTR;
    return -1;
  }

  va_list args, argscp;
  int     ret = -1;

  va_start(args, fmt);
  va_copy(argscp, args);

  ctorm_res_clear(res);

  res->body_size = vsnprintf(NULL, 0, fmt, args);
  res->body      = malloc(res->body_size + 1);

  ret = vsnprintf(res->body, res->body_size + 1, fmt, argscp);

  ctorm_res_set(res, CTORM_HTTP_CONTENT_TYPE, "text/plain; charset=utf-8");

  va_end(args);
  va_end(argscp);

  return ret;
}

int ctorm_res_add(ctorm_res_t *res, const char *fmt, ...) {
  if (NULL == fmt) {
    errno = CTORM_ERR_BAD_FMT_PTR;
    return -1;
  }

  va_list args, argscp;
  int     ret = -1, vsize = 0;

  va_start(args, fmt);
  va_copy(argscp, args);

  if (NULL == res->body || res->body_size <= 0) {
    ctorm_res_set(res, CTORM_HTTP_CONTENT_TYPE, "text/plain; charset=utf-8");
    vsize     = vsnprintf(NULL, 0, fmt, args);
    res->body = malloc(res->body_size + vsize + 1);
  } else {
    vsize     = vsnprintf(NULL, 0, fmt, args);
    res->body = realloc(res->body, res->body_size + vsize);
  }

  ret = vsnprintf(
      res->body + res->body_size, res->body_size + 1 + vsize, fmt, argscp);
  res->body_size += vsize;

  va_end(args);
  va_end(argscp);

  return ret;
}

bool ctorm_res_json(ctorm_res_t *res, cJSON *json) {
  if (NULL == json) {
    errno = CTORM_ERR_BAD_JSON_PTR;
    return false;
  }

  ctorm_res_clear(res);

  if ((res->body = ctorm_json_encode(json)) == NULL)
    return false;

  res->body_size = cu_strlen(res->body);

  ctorm_res_set(
      res, CTORM_HTTP_CONTENT_TYPE, "application/json; charset=utf-8");
  return true;
}

void ctorm_res_redirect(ctorm_res_t *res, char *uri) {
  if (NULL == uri) {
    errno = CTORM_ERR_BAD_URI_PTR;
    return;
  }

  res->code = 301;
  ctorm_res_set(res, "location", uri);
}

bool ctorm_res_send(ctorm_res_t *res) {
  ctorm_header_pos_t pos;

  switch (res->version) {
  case CTORM_HTTP_1_0:
    res_send_fmt("HTTP/1.0 %hu", res->code);
    break;

  case CTORM_HTTP_1_1:
    res_send_fmt("HTTP/1.1 %hu", res->code);
    break;
  }

  // send the newline
  res_send_str("\r\n");

  // send response headers
  ctorm_headers_start(&pos);

  while (ctorm_headers_next(res->headers, &pos))
    res_send_fmt("%s: %s\r\n", pos.name, pos.value);

  res_send_fmt("content-length: %lu\r\n", res->body_size);
  res_send_str("\r\n");

  // if a file is specified read the body from file and send it
  if (res->body_fd > 0) {
    size_t read_size = 0;
    char   read_buff[50];

    while ((read_size = read(res->body_fd, read_buff, sizeof(read_buff))) > 0)
      res_send(read_buff, read_size, MSG_NOSIGNAL);
  }

  // if a body is specified, send it
  else if (res->body_size > 0)
    res_send(res->body, res->body_size, MSG_NOSIGNAL);

  return true;
}
