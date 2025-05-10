#include "encoding.h"
#include "headers.h"
#include "error.h"

#include "http.h"
#include "pair.h"
#include "util.h"

#include "req.h"
#include "log.h"

#include <sys/socket.h>
#include <arpa/inet.h>

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#define req_debug(f, ...)                                                      \
  debug("(" FG_BOLD "socket " FG_CYAN "%d" FG_RESET FG_BOLD                    \
        " request " FG_CYAN "0x%p" FG_RESET ") " f,                            \
      req->con->socket,                                                        \
      req,                                                                     \
      ##__VA_ARGS__)

#define req_recv(b, s, f) ctorm_conn_recv(req->con, b, s, f)

int64_t _req_recv_until(ctorm_req_t *req, char *buf, int64_t max, char del) {
  int64_t indx = 0;
  char    cur  = 0;

  for (; max > indx && req_recv(&cur, 1, MSG_WAITALL) > 0; indx++) {
    // if no buffer is provided, just check the delimiter
    if (NULL == buf) {
      if (cur == del)
        return indx;
      continue;
    }

    // if a buffer is provided, check the delimiter and replace it with '\0'
    if (cur == del) {
      buf[indx] = 0;
      return indx;
    }

    // otherwise just add the current char to the buffer
    buf[indx] = cur;
  }

  // we reached the max length, reset the buffer and return -1 for failure
  bzero(buf, max);
  return -1;
}

int64_t _req_recv_alloc(ctorm_req_t *req, char **buf, int64_t max, char del) {
  int64_t indx = 0, size = 0;
  char    cur = 0;

  for (; max > indx && req_recv(&cur, 1, MSG_WAITALL) > 0; indx++) {
    // no need to append to the buffer if we reach the end
    if (cur == del)
      return ++indx;

    // allocate/reallocate the buffer
    if (NULL == *buf) // size == 0
      *buf = malloc(size += indx + 16);
    else if (indx + 1 >= size)
      *buf = realloc(*buf, size += indx + 16);

    // append the current char to the buffer and add a NULL terminator
    (*buf)[indx]     = cur;
    (*buf)[indx + 1] = 0;
  }

  // if we reach the max length, free the buffer and return -1 for failure
  free(*buf);
  *buf = NULL;
  return -1;
}

int32_t _req_recv_char(ctorm_req_t *req, char *c) {
  if (NULL != c)
    return req_recv(c, 1, MSG_WAITALL) == 1 ? 1 : -1;

  char tc = -1;
  req_recv(&tc, 1, MSG_WAITALL);
  return tc;
}

#define req_recv_until(buf, max, del) _req_recv_until(req, buf, max, del)
#define req_recv_alloc(buf, max, del) _req_recv_alloc(req, buf, max, del)
#define req_recv_char(char)           _req_recv_char(req, char)

void ctorm_req_init(ctorm_req_t *req, ctorm_conn_t *con) {
  bzero(req, sizeof(*req));

  // request stuff (not related to HTTP)
  req->con    = con;
  req->cancel = false;

  // HTTP method and version
  req->method  = -1;
  req->version = -1;

  // headers and body
  ctorm_headers_init(&req->headers);
  req->body_size = -1;
}

void ctorm_req_free(ctorm_req_t *req) {
  if (ctorm_req_is_valid(req)) {
    // get the request body size
    ctorm_req_body_size(req);

    // receive rest of the body from the connection
    for (char c = 0; req->body_size > 0; req->body_size--)
      if (req_recv_char(&c) != 1)
        break;
  }

  ctorm_pair_free(req->locals);

  ctorm_url_free(req->queries);
  ctorm_pair_free(req->params);

  ctorm_headers_free(req->headers);
}

// 5.3.1. origin-form
bool _ctorm_req_parse_origin(ctorm_req_t *req) {
  cu_unused(req);
  // TODO: implement
  return false;
}

// 5.3.2. absolute-form
bool _ctorm_req_parse_absolute(ctorm_req_t *req) {
  cu_unused(req);
  // TODO: implement
  return false;
}

// 5.3.3. authority-form
bool _ctorm_req_parse_authority(ctorm_req_t *req) {
  cu_unused(req);
  // TODO: implement
  return false;
}

// 5.3.4. asterisk-form
bool _ctorm_req_parse_asterisk(ctorm_req_t *req) {
  cu_unused(req);
  // TODO: implement
  return false;
}

bool ctorm_req_recv(ctorm_req_t *req) {
  char    http_method[CTORM_HTTP_METHOD_MAX + 1];
  char    http_version[CTORM_HTTP_VERSION_LEN + 1];
  int64_t size = 0;

  // clear the method and the version buffer
  bzero(http_method, sizeof(http_method));
  bzero(http_version, sizeof(http_version));

  // receive the method from the request line
  if (req_recv_until(http_method, sizeof(http_method), ' ') < 0) {
    req_debug("failed to receive the HTTP method");
    return false;
  }

  if ((req->method = ctorm_http_method(http_method)) < 0) {
    req_debug("received an invalid HTTP method: %s", http_method);
    return false;
  }

  // receive the HTTP request target
  if ((size = req_recv_alloc(&req->target, ctorm_http_target_max, ' ')) < 0) {
    req_debug("failed to receive the HTTP request target");
    return false;
  }

  if (size <= 0) {
    req_debug("received an invalid HTTP path");
    return false;
  }

  // parse the request target (see "5.3. Request Target")
  switch (*req->target) {
  // origin-form
  case '/':
    if (!_ctorm_req_parse_origin(req))
      return false;
    break;

  // asterisk-form
  case '*':
    if (!_ctorm_req_parse_asterisk(req))
      return false;
    break;

  // absolute-form or authority-form
  default:
    if (!_ctorm_req_parse_absolute(req) && !_ctorm_req_parse_authority(req))
      return false;
  }

  // receive the HTTP version
  if (req_recv_until(http_version, sizeof(http_version), '\r') < 0) {
    req_debug("failed to receive the HTTP version");
    return false;
  }

  if (req_recv_char(NULL) != '\n') {
    req_debug("failed to receive the CRLF of the request line");
    return false;
  }

  if ((req->version = ctorm_http_version(http_version)) < 0) {
    req_debug("received an invalid HTTP version: %s", http_version);
    return false;
  }

  // TODO: receive all the headers
  return false;
}

char *ctorm_req_query(ctorm_req_t *req, char *name) {
  if (NULL == name) {
    errno = CTORM_ERR_BAD_QUERY_PTR;
    return NULL;
  }

  return ctorm_url_get(req->queries, name);
}

char *ctorm_req_param(ctorm_req_t *req, char *name) {
  if (NULL == name) {
    errno = CTORM_ERR_BAD_PARAM_PTR;
    return NULL;
  }

  ctorm_pair_t *param = ctorm_pair_find(req->params, name);
  return NULL == param ? NULL : param->value;
}

void *ctorm_req_local(ctorm_req_t *req, char *name, char *value) {
  if (NULL == name) {
    errno = CTORM_ERR_BAD_LOCAL_PTR;
    return NULL;
  }

  ctorm_pair_t *local = NULL;

  if (NULL == value)
    local = ctorm_pair_find(req->locals, name);
  else
    local = ctorm_pair_add(&req->locals, name, value);

  return NULL == local ? NULL : local->value;
}

ctorm_url_t *ctorm_req_form(ctorm_req_t *req) {
  char        *type = ctorm_req_get(req, "content-type");
  ctorm_url_t *form = NULL;
  int64_t      size = 0;

  if (!cu_startswith(type, "application/x-www-form-urlencoded")) {
    errno = CTORM_ERR_BAD_LOCAL_PTR;
    return NULL;
  }

  if ((size = ctorm_req_body_size(req)) <= 0) {
    errno = CTORM_ERR_EMPTY_BODY;
    return NULL;
  }

  char data[size];
  bzero(data, size);

  if (ctorm_req_body(req, data, size) != size)
    return NULL; // errno set by ctorm_req_body()

  if ((form = ctorm_url_parse(data, size)) == NULL)
    return NULL;

  return form;
}

cJSON *ctorm_req_json(ctorm_req_t *req) {
#if CTORM_JSON_SUPPORT
  char   *type = ctorm_req_get(req, "content-type");
  int64_t size = 0;

  if (!cu_startswith(type, "application/json")) {
    errno = CTORM_ERR_BAD_CONTENT_TYPE;
    return NULL;
  }

  if ((size = ctorm_req_body_size(req)) <= 0) {
    errno = CTORM_ERR_EMPTY_BODY;
    return NULL;
  }

  char data[size + 1];
  bzero(data, size);

  if (ctorm_req_body(req, data, size) != size)
    return NULL; // errno set by ctorm_req_body()

  return ctorm_json_parse(data);
#else
  errno = CTORM_ERR_NO_JSON_SUPPORT;
  return NULL;
#endif
}

const char *ctorm_req_method(ctorm_req_t *req) {
  if (ctorm_req_is_valid(req))
    return ctorm_http_method_name(req->method);
  return NULL;
}

char *ctorm_req_get(ctorm_req_t *req, char *name) {
  if (NULL == name) {
    errno = CTORM_ERR_BAD_HEADER_PTR;
    return NULL;
  }

  return ctorm_headers_get(req->headers, name);
}

int64_t ctorm_req_body_size(ctorm_req_t *req) {
  if (req->body_size >= 0)
    return req->body_size;

  if (!ctorm_http_method_allows_req_body(req->method)) {
    req->body_size = 0;
    return 0;
  }

  char *len = ctorm_req_get(req, "content-length");

  if (NULL == len) {
    req->body_size = 0;
    return 0;
  }

  if ((req->body_size = atol(len)) < 0)
    req->body_size = 0;

  return req->body_size;
}

int64_t ctorm_req_body(ctorm_req_t *req, char *buffer, int64_t size) {
  if (NULL == buffer || size <= 0) {
    errno = CTORM_ERR_BAD_BUFFER;
    return -1;
  }

  // receive all the headers so we can receive the body next
  ctorm_req_get(req, NULL);

  if (size >= ctorm_req_body_size(req))
    size = req->body_size;
  req->body_size -= size;

  if (size == 0)
    return 0;

  return req_recv(buffer, size, MSG_WAITALL);
}

char *ctorm_req_ip(ctorm_req_t *req, char *_ipbuf) {
  char *ipbuf = _ipbuf;

  if (NULL == ipbuf)
    ipbuf = malloc(INET6_ADDRSTRLEN > INET_ADDRSTRLEN ? INET6_ADDRSTRLEN
                                                      : INET_ADDRSTRLEN);

  if (NULL == ipbuf) {
    errno = CTORM_ERR_ALLOC_FAIL;
    return NULL;
  }

  switch (req->con->addr.sa_family) {
  case AF_INET:
    inet_ntop(AF_INET,
        &((struct sockaddr_in *)&req->con->addr)->sin_addr,
        ipbuf,
        INET_ADDRSTRLEN);
    break;

  case AF_INET6:
    inet_ntop(AF_INET6,
        &((struct sockaddr_in *)&req->con->addr)->sin_addr,
        ipbuf,
        INET6_ADDRSTRLEN);
    break;
  }

  return ipbuf;
}

bool ctorm_req_persist(ctorm_req_t *req) {
  if (!ctorm_req_is_valid(req))
    return false;

  const char *con = ctorm_req_get(req, "connection");

  if (NULL != con && cu_streq(con, "close"))
    return false;

  switch (req->version) {
  case CTORM_HTTP_1_1:
    return true;

  case CTORM_HTTP_1_0:
    return NULL != con && cu_streq(con, "keep-alive");
  }

  return false;
}
