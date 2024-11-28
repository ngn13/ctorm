#include "../include/options.h"
#include "../include/headers.h"

#include "../include/errors.h"
#include "../include/util.h"
#include "../include/req.h"
#include "../include/log.h"

#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <stdio.h>

#define req_debug(f, ...)                                                                                              \
  debug("(" FG_BOLD "Socket " FG_CYAN "%d" FG_RESET FG_BOLD " Request " FG_CYAN "0x%p" FG_RESET ") " f,                \
      req->con->socket,                                                                                                \
      req,                                                                                                             \
      ##__VA_ARGS__)
#define rrecv(b, s, f) connection_recv(req->con, b, s, f)
#define RECV_BUF_SIZE  20

bool __rrecv_until(req_t *req, char **buf, uint64_t *size, char del, bool (*is_valid)(char)) {
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

  if (buf_size > 0)
    free(*buf);

  if (NULL != size)
    *size = 0;

  return false;
}

#define rrecv_until(b, s, d, v) __rrecv_until(req, b, s, d, v)

bool __rrecv_is_valid_header(char c) {
  return c == '\r' || http_is_valid_header_char(c);
}

bool __rrecv_is_valid_path(char c) {
  return c == '\r' || http_is_valid_path_char(c);
}

void req_init(req_t *req, connection_t *con) {
  headers_init(&req->headers);
  req->received_headers = false;

  req->queries  = NULL;
  req->bodysize = -1;

  req->con     = con;
  req->cancel  = false;
  req->version = NULL;
  req->encpath = NULL;
  req->path    = NULL;
}

void req_free(req_t *req) {
  headers_free(req->headers);
  enc_url_free(req->queries);

  if (req->encpath == req->path) {
    free(req->encpath);
    return;
  }

  free(req->encpath);
  free(req->path);

  // req->version is a static pointer
}

bool req_start(req_t *req) {
  char     _http_method[http_static.method_max + 1], *http_method    = _http_method;
  char     _http_version[http_static.version_len + 2], *http_version = _http_version;
  uint64_t buf_size = 0;

  // get the HTTP method
  buf_size = sizeof(_http_method);

  if (!rrecv_until(&http_method, &buf_size, ' ', NULL)) {
    req_debug("Failed to get the HTTP method");
    return false;
  }

  if ((req->method = http_method_id(http_method)) == -1) {
    req_debug("Invalid HTTP method: %s", http_method);
    return false;
  }

  // get the HTTP path
  buf_size = http_static.path_max + 1;

  if (!rrecv_until(&req->encpath, &buf_size, ' ', __rrecv_is_valid_path)) {
    req_debug("Failed to get the request path");
    return false;
  }

  // get the HTTP version
  buf_size = sizeof(_http_version);

  if (!rrecv_until(&http_version, &buf_size, '\n', NULL)) {
    req_debug("Failed to get the HTTP version");
    return false;
  }

  truncate_buf(http_version, buf_size, 2, '\r');

  if (NULL == (req->version = http_version_get(http_version))) {
    req_debug("Received an invalid HTTP version: %s", http_version);
    return false;
  }

  // decode the path (queries and shit)
  char *save = NULL, *rest = NULL, *dup = NULL;

  if (!contains(req->encpath, '?')) {
    req->path = req->encpath;
    return true;
  }

  dup = strdup(req->encpath);

  if (NULL == (req->path = strtok_r(dup, "?", &save))) {
    req->path = req->encpath;
    goto dup_free_ret;
  }

  if (NULL == (rest = strtok_r(NULL, "?", &save))) {
    req->path = strdup(req->path);
    goto dup_free_ret;
  }

  req->queries = enc_url_parse(rest, 0);
  req->path    = strdup(req->path);

dup_free_ret:
  free(dup);
  return true;
}

void req_end(req_t *req) {
  // get the request body size
  req_body_size(req);

  if (!req->received_headers) {
    // skip all the headers (we aint gonna need them after this point)
    uint64_t size = 0;

    while (rrecv_until(NULL, &size, '\n', NULL) && (size != 1 && size != 2))
      continue;

    req->received_headers = true;
  }

  // receive all the body from the connection
  char c = 0;
  while (req_body(req, &c, sizeof(c)) > 0)
    ;
}

char *req_query(req_t *req, char *name) {
  if (NULL == name)
    return NULL;
  return enc_url_get(req->queries, name);
}

urlenc_t *req_form(req_t *req) {
  char     *contentt = req_get(req, "content-type");
  urlenc_t *form     = NULL;
  uint64_t  size     = 0;

  if (!startswith(contentt, "application/x-www-form-urlencoded")) {
    errno = InvalidContentType;
    return NULL;
  }

  if ((size = req_body_size(req)) == 0) {
    errno = EmptyBody;
    return NULL;
  }

  char data[size];
  bzero(data, size);

  if (req_body(req, data, size) != size) {
    errno = BodyRecvFail;
    return NULL;
  }

  if ((form = enc_url_parse(data, size)) == NULL)
    return NULL;

  return form;
}

cJSON *req_json(req_t *req) {
#if CTORM_JSON_SUPPORT
  char    *contentt = req_get(req, "content-type");
  uint64_t size     = 0;

  if (!startswith(contentt, "application/json")) {
    errno = InvalidContentType;
    return NULL;
  }

  if ((size = req_body_size(req)) == 0) {
    errno = BodyRecvFail;
    return NULL;
  }

  char data[size + 1];
  bzero(data, size);

  if (req_body(req, data, size) != size) {
    errno = BodyRecvFail;
    return NULL;
  }

  return enc_json_parse(data);
#else
  errno = NoJSONSupport;
  return NULL;
#endif
}

char *req_method(req_t *req) {
  return http_method_name(req->method);
}

char *req_get(req_t *req, char *name) {
  char *header_val = NULL;

  // if the name equals NULL, we want to receive all the headers
  if (NULL != name && (header_val = headers_get(req->headers, name)) != NULL)
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
      req_debug("Received all the headers");
      req->received_headers = true;
      return NULL;
    }
  }

  // receive the header name
  buf_size    = http_static.header_max;
  header_name = NULL;

  if (!rrecv_until(&header_name, &buf_size, ':', __rrecv_is_valid_header)) {
    req_debug("Failed to get the header name");
    return NULL;
  }

  // receive the header value
  if (rrecv(&sep, sizeof(sep), MSG_WAITALL) != sizeof(sep) || sep != ' ') {
    req_debug("Failed to receive the header value (invalid separator)");
    return NULL;
  }

  buf_size   = http_static.header_max;
  header_val = NULL;

  if (!rrecv_until(&header_val, &buf_size, '\n', __rrecv_is_valid_header)) {
    req_debug("Failed to get the header value");
    return NULL;
  }

  truncate_buf(header_val, buf_size, 2, '\r');
  req_debug("Received a new header: %s (%.5s...)", header_name, header_val);
  headers_set(req->headers, header_name, header_val, true);

  if (NULL != name && headers_cmp(header_name, name))
    return header_val;

  goto next_header;
}

uint64_t req_body_size(req_t *req) {
  if (req->bodysize >= 0)
    return req->bodysize;

  if (!http_method_has_body(req->method)) {
    req->bodysize = 0;
    return 0;
  }

  char *content_len = req_get(req, "content-length");

  if (NULL == content_len) {
    req->bodysize = 0;
    return 0;
  }

  if ((req->bodysize = atol(content_len)) < 0)
    req->bodysize = 0;

  return req->bodysize;
}

uint64_t req_body(req_t *req, char *buffer, uint64_t size) {
  // receive all the headers so we can receive the body next
  req_get(req, NULL);

  if (size >= req_body_size(req))
    size = req->bodysize;
  req->bodysize -= size;

  if (size == 0)
    return 0;

  return rrecv(buffer, size, MSG_WAITALL);
}

char *req_ip(req_t *req, char *_ipbuf) {
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
