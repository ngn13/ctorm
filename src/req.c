#include "headers.h"
#include "errors.h"

#include "http.h"
#include "util.h"

#include "req.h"
#include "log.h"

#include <arpa/inet.h>

#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <stdio.h>

#define rdebug(f, ...)                                                                                                 \
  debug("(" FG_BOLD "socket " FG_CYAN "%d" FG_RESET FG_BOLD " Request " FG_CYAN "0x%p" FG_RESET ") " f,                \
      req->con->socket,                                                                                                \
      req,                                                                                                             \
      ##__VA_ARGS__)
#define rrecv(b, s, f) connection_recv(req->con, b, s, f)
#define RECV_BUF_SIZE  20

bool __rrecv_until(ctorm_req_t *req, char **buf, uint64_t *size, char del, bool (*is_valid)(char)) {
  if (NULL == req)
    return NULL;

  uint64_t buf_size = 0, buf_indx = 0;
  char     cur = 0;
  bool     ret = false;

  if (NULL != buf && NULL == *buf) {
    buf_size = RECV_BUF_SIZE;
    *buf     = malloc(buf_size);
  }

  while (rrecv(&cur, sizeof(cur), MSG_WAITALL) > 0) {
    if (NULL != size && *size != 0 && buf_indx >= *size)
      break;

    if (buf_size > 0 && buf_indx >= buf_size)
      *buf = realloc(*buf, buf_size += RECV_BUF_SIZE);

    if (NULL != buf)
      *(*buf + buf_indx) = cur;

    if (del == cur) {
      if (NULL != buf)
        *(*buf + buf_indx) = 0; // replace the delemiter with a NULL terminator

      if (NULL != size)
        *size = buf_indx + 1;

      ret = true;
      break;
    }

    if (NULL != is_valid && !is_valid(cur))
      break;

    buf_indx++;
  }

  if (ret)
    return true;

  if (buf_size > 0) {
    free(*buf);
    *buf = NULL;
  }

  if (NULL != size)
    *size = 0;

  return false;
}

#define rrecv_until(b, s, d, v) __rrecv_until(req, b, s, d, v)

bool rrecv_is_valid_header(char c) {
  return c == '\r' || http_is_valid_header_char(c);
}

bool rrecv_is_valid_path(char c) {
  return c == '\r' || http_is_valid_path_char(c);
}

void ctorm_req_init(ctorm_req_t *req, connection_t *con) {
  bzero(req, sizeof(*req));

  ctorm_headers_init(&req->headers);
  req->received_headers = false;

  req->queries  = NULL;
  req->params   = NULL;
  req->bodysize = -1;

  req->con     = con;
  req->cancel  = false;
  req->version = NULL;
  req->encpath = NULL;
  req->path    = NULL;
}

void ctorm_req_free(ctorm_req_t *req) {
  ctorm_headers_free(req->headers);
  ctorm_url_free(req->queries);
  ctorm_pair_free(req->params);

  free(req->encpath);
  free(req->path);

  // req->version is a static pointer
}

bool ctorm_req_start(ctorm_req_t *req) {
  char     _http_method[http_static.method_max + 1], *http_method    = _http_method;
  char     _http_version[http_static.version_len + 2], *http_version = _http_version;
  uint64_t buf_size = 0;

  // get the HTTP method
  buf_size = sizeof(_http_method);

  if (!rrecv_until(&http_method, &buf_size, ' ', NULL)) {
    rdebug("failed to get the HTTP method");
    return false;
  }

  if ((req->method = http_method_id(http_method)) == -1) {
    rdebug("invalid HTTP method: %s", http_method);
    return false;
  }

  // get the HTTP path
  buf_size = http_static.path_max + 1;

  if (!rrecv_until(&req->encpath, &buf_size, ' ', rrecv_is_valid_path)) {
    rdebug("failed to get the request path");
    return false;
  }

  // get the HTTP version
  buf_size = sizeof(_http_version);

  if (!rrecv_until(&http_version, &buf_size, '\n', NULL)) {
    rdebug("failed to get the HTTP version");
    return false;
  }

  cu_truncate(http_version, buf_size, 2, '\r');

  if (NULL == (req->version = http_version_get(http_version))) {
    rdebug("received an invalid HTTP version: %s", http_version);
    return false;
  }

  // decode the path (queries and shit)
  char *save = NULL, *rest = NULL, *dup = NULL;

  if ((dup = strdup(req->encpath)) == NULL) {
    rdebug("failed to duplicate encpath: %s", strerror(errno));
    return false;
  }

  if (NULL == (req->path = strtok_r(dup, "?", &save)) || NULL == (rest = strtok_r(NULL, "?", &save)))
    req->path = dup;
  else
    req->queries = ctorm_url_parse(rest, 0);

  cu_url_decode(req->path);
  return true;
}

void ctorm_req_end(ctorm_req_t *req) {
  // get the request body size
  ctorm_req_body_size(req);

  if (!req->received_headers) {
    // skip all the headers (we aint gonna need them after this point)
    uint64_t size = 0;

    while (rrecv_until(NULL, &size, '\n', NULL) && (size != 1 && size != 2))
      continue;

    req->received_headers = true;
  }

  // receive all the body from the connection
  char c = 0;
  while (ctorm_req_body(req, &c, sizeof(c)) > 0)
    ;
}

char *ctorm_req_query(ctorm_req_t *req, char *name) {
  if (NULL == name)
    return NULL;
  return ctorm_url_get(req->queries, name);
}

char *ctorm_req_param(ctorm_req_t *req, char *name) {
  if (NULL == name)
    return NULL;

  ctorm_pair_t *pair = ctorm_pair_find(req->params, name);

  if (NULL == pair)
    return NULL;

  return pair->value;
}

ctorm_url_t *ctorm_req_form(ctorm_req_t *req) {
  char        *type = ctorm_req_get(req, "content-type");
  ctorm_url_t *form = NULL;
  uint64_t     size = 0;

  if (!cu_startswith(type, "application/x-www-form-urlencoded")) {
    errno = InvalidContentType;
    return NULL;
  }

  if ((size = ctorm_req_body_size(req)) == 0) {
    errno = EmptyBody;
    return NULL;
  }

  char data[size];
  bzero(data, size);

  if (ctorm_req_body(req, data, size) != size) {
    errno = BodyRecvFail;
    return NULL;
  }

  if ((form = ctorm_url_parse(data, size)) == NULL)
    return NULL;

  return form;
}

cJSON *ctorm_req_json(ctorm_req_t *req) {
#if CTORM_JSON_SUPPORT
  char    *type = ctorm_req_get(req, "content-type");
  uint64_t size = 0;

  if (!cu_startswith(type, "application/json")) {
    errno = InvalidContentType;
    return NULL;
  }

  if ((size = ctorm_req_body_size(req)) == 0) {
    errno = BodyRecvFail;
    return NULL;
  }

  char data[size + 1];
  bzero(data, size);

  if (ctorm_req_body(req, data, size) != size) {
    errno = BodyRecvFail;
    return NULL;
  }

  return ctorm_json_parse(data);
#else
  errno = NoJSONSupport;
  return NULL;
#endif
}

const char *ctorm_req_method(ctorm_req_t *req) {
  return http_method_name(req->method);
}

char *ctorm_req_get(ctorm_req_t *req, char *name) {
  char *header_val = NULL;

  // if the name equals NULL, we want to receive all the headers
  if (NULL != name && (header_val = ctorm_headers_get(req->headers, name)) != NULL)
    return header_val;

  if (req->received_headers)
    return NULL;

  char    *header_name = NULL, sep = 0;
  uint64_t buf_size = 0;

next_header:
  // check if we reached the body
  if (rrecv(&sep, sizeof(sep), MSG_PEEK) == sizeof(sep)) {
    if (sep == '\r') {
      rrecv(&sep, sizeof(sep), MSG_WAITALL);
      rrecv(&sep, sizeof(sep), MSG_PEEK);
    }

    // we reached the body
    if (sep == '\n') {
      rrecv(&sep, sizeof(sep), MSG_WAITALL);
      rdebug("received all the headers");
      req->received_headers = true;
      return NULL;
    }
  }

  // receive the header name
  buf_size    = http_static.header_max;
  header_name = NULL;

  if (!rrecv_until(&header_name, &buf_size, ':', rrecv_is_valid_header)) {
    rdebug("failed to get the header name");
    return NULL;
  }

  // receive the header value
  if (rrecv(&sep, sizeof(sep), MSG_WAITALL) != sizeof(sep) || sep != ' ') {
    rdebug("failed to receive the header value (invalid separator)");
    return NULL;
  }

  buf_size   = http_static.header_max;
  header_val = NULL;

  if (!rrecv_until(&header_val, &buf_size, '\n', rrecv_is_valid_header)) {
    rdebug("failed to get the header value");
    return NULL;
  }

  cu_truncate(header_val, buf_size, 2, '\r');
  rdebug("received a new header: %s (%.5s...)", header_name, header_val);
  ctorm_headers_set(req->headers, header_name, header_val, true);

  if (NULL != name && ctorm_headers_cmp(header_name, name))
    return header_val;

  goto next_header;
}

uint64_t ctorm_req_body_size(ctorm_req_t *req) {
  if (req->bodysize >= 0)
    return req->bodysize;

  if (!http_method_has_body(req->method)) {
    req->bodysize = 0;
    return 0;
  }

  char *len = ctorm_req_get(req, "content-length");

  if (NULL == len) {
    req->bodysize = 0;
    return 0;
  }

  if ((req->bodysize = atol(len)) < 0)
    req->bodysize = 0;

  return req->bodysize;
}

uint64_t ctorm_req_body(ctorm_req_t *req, char *buffer, uint64_t size) {
  // receive all the headers so we can receive the body next
  ctorm_req_get(req, NULL);

  if (size >= ctorm_req_body_size(req))
    size = req->bodysize;
  req->bodysize -= size;

  if (size == 0)
    return 0;

  return rrecv(buffer, size, MSG_WAITALL);
}

char *ctorm_req_ip(ctorm_req_t *req, char *_ipbuf) {
  char *ipbuf = _ipbuf;

  if (NULL == ipbuf)
    ipbuf = malloc(INET6_ADDRSTRLEN > INET_ADDRSTRLEN ? INET6_ADDRSTRLEN : INET_ADDRSTRLEN);

  if (NULL == ipbuf) {
    errno = AllocFailed;
    return NULL;
  }

  switch (req->con->addr.sa_family) {
  case AF_INET:
    inet_ntop(AF_INET, &((struct sockaddr_in *)&req->con->addr)->sin_addr, ipbuf, INET_ADDRSTRLEN);
    break;

  case AF_INET6:
    inet_ntop(AF_INET6, &((struct sockaddr_in *)&req->con->addr)->sin_addr, ipbuf, INET6_ADDRSTRLEN);
    break;
  }

  return ipbuf;
}
