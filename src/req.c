#include "encoding.h"
#include "headers.h"
#include "error.h"

#include "conn.h"
#include "http.h"
#include "pair.h"
#include "util.h"

#include "uri.h"
#include "req.h"
#include "log.h"

#include <sys/socket.h>
#include <arpa/inet.h>

#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#define req_debug(f, ...)                                                      \
  debug("(" FG_BOLD "request " FG_CYAN "0x%p %d" FG_RESET FG_BOLD FG_RESET     \
        ") " f,                                                                \
      req,                                                                     \
      req->conn->socket,                                                       \
      ##__VA_ARGS__)

#define req_recv(buf, len, flags)     _ctorm_req_recv(req, buf, len, flags)
#define req_recv_until(buf, max, del) _ctorm_req_recv_until(req, buf, max, del)
#define req_recv_alloc(buf, max, del) _ctorm_req_recv_alloc(req, buf, max, del)
#define req_recv_char(char)           _ctorm_req_recv_char(req, char)

int64_t _ctorm_req_recv(ctorm_req_t *req, char *buf, uint64_t len, int flags) {
  int64_t ret = ctorm_conn_recv(req->conn, buf, len, flags);

  if (ret >= 0)
    return ret;

  switch (errno) {
  case ETIMEDOUT:
  case EAGAIN:
    req->code = 408;
    break;
  }

  return ret;
}

int64_t _ctorm_req_recv_until(
    ctorm_req_t *req, char *buf, int64_t max, char del) {
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
  memset(buf, 0, max);
  return -1;
}

int64_t _ctorm_req_recv_alloc(
    ctorm_req_t *req, char **buf, int64_t max, char del) {
  int64_t indx = 0, size = 0;
  char    cur = 0;

  for (; max > indx && req_recv(&cur, 1, MSG_WAITALL) > 0; indx++) {
    // no need to append to the buffer if we reach the end
    if (cur == del)
      return indx;

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

int32_t _ctorm_req_recv_char(ctorm_req_t *req, char *c) {
  if (NULL != c)
    return req_recv(c, 1, MSG_WAITALL) == 1 ? 1 : -1;

  char tc = -1;
  req_recv(&tc, 1, MSG_WAITALL);
  return tc;
}

void ctorm_req_init(ctorm_req_t *req, ctorm_conn_t *conn) {
  if (NULL == req || NULL == conn)
    return;

  memset(req, 0, sizeof(*req));

  // request stuff (not HTTP related)
  req->conn   = conn;
  req->cancel = false;

  // default stuff
  req->version = CTORM_HTTP_1_1;
  req->code    = 400; // bad request

  // headers and body
  ctorm_headers_init(&req->headers);
  req->body_size = -1;
}

void ctorm_req_free(ctorm_req_t *req) {
  if (!ctorm_http_code_is_error(req->code)) {
    // receive rest of the body from the connection
    for (char c = 0; req->body_size > 0; req->body_size--)
      if (req_recv_char(&c) != 1)
        break;
  }

  if (NULL != req->body_form)
    ctorm_query_free(req->body_form);

  if (NULL != req->body_json)
    ctorm_json_free(req->body_json);

  ctorm_pair_free(req->locals);

  ctorm_query_free(req->queries);
  ctorm_pair_free(req->params);

  ctorm_headers_free(req->headers);
}

// 5.3.1. origin-form
bool _ctorm_req_parse_origin(ctorm_req_t *req) {
  ctorm_uri_t uri;
  ctorm_uri_init(&uri);

  if (NULL == ctorm_uri_parse_path(&uri, req->target)) {
    req_debug("failed to parse: %s", ctorm_error());
    return false;
  }

  // save all the URI fields
  req->path    = uri.path;
  req->queries = uri.query;

  // remove the used fields to prevent them from being freed
  uri.path  = NULL;
  uri.query = NULL;

  ctorm_uri_free(&uri);
  return true;
}

// 5.3.2. absolute-form
bool _ctorm_req_parse_absolute(ctorm_req_t *req) {
  ctorm_uri_t uri;
  ctorm_uri_init(&uri);

  // parse the absolute URI
  if (!ctorm_uri_parse(&uri, req->target)) {
    req_debug("failed to parse: %s", ctorm_error());
    return false;
  }

  req->host    = uri.host;
  req->path    = uri.path;
  req->queries = uri.query;

  uri.host  = NULL;
  uri.path  = NULL;
  uri.query = NULL;

  ctorm_uri_free(&uri);
  return true;
}

// 5.3.3. authority-form
bool _ctorm_req_parse_authority(ctorm_req_t *req) {
  if (CTORM_HTTP_CONNECT != req->method)
    return false;

  ctorm_uri_t uri;
  ctorm_uri_init(&uri);

  // only parse the authority component
  if (!ctorm_uri_parse_auth(&uri, req->target)) {
    req_debug("failed to parse: %s", ctorm_error());
    return false;
  }

  // this format should not include any userinfo
  if (NULL != uri.userinfo) {
    ctorm_uri_free(&uri);
    return false;
  }

  req->host = uri.host;
  uri.host  = NULL;

  ctorm_uri_free(&uri);
  return true;
}

// 5.3.4. asterisk-form
bool _ctorm_req_parse_asterisk(ctorm_req_t *req) {
  if (CTORM_HTTP_OPTIONS != req->method)
    return false;

  if (!cu_streq(req->target, "*"))
    return false;

  req->path = strdup("*");
  return true;
}

bool ctorm_req_recv(ctorm_req_t *req) {
  char    http_method[CTORM_HTTP_METHOD_MAX + 1];
  char    http_version[CTORM_HTTP_VERSION_LEN + 1];
  int64_t size = 0;

  // clear the method and the version buffer
  memset(http_method, 0, sizeof(http_method));
  memset(http_version, 0, sizeof(http_version));

  // receive the method from the request line
  if (req_recv_until(http_method, sizeof(http_method), ' ') < 0) {
    req_debug("failed to receive the HTTP method");
    return false;
  }

  if (!ctorm_http_method(http_method, &req->method)) {
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

  if (!ctorm_http_version(http_version, &req->version)) {
    req_debug("received an invalid HTTP version: %s", http_version);
    return false;
  }

  char   *name = NULL, *value = NULL;
  uint8_t count = 0;
  char    c     = 0;

  while (req_recv(&c, 1, MSG_PEEK) == 1 && c != '\r') {
    // check the counter
    if (count >= CTORM_HTTP_HEADER_MAX) {
      req_debug("too many headers");
      req->code = 413; // too large
      return false;
    }

    // receive the header name
    if ((size = req_recv_alloc(&name, ctorm_http_header_name_max, ':')) <= 0) {
      req_debug("failed to receive the HTTP header name");
      return false;
    }

    if (!ctorm_http_is_valid_header_name(name, size)) {
      req_debug("invalid HTTP header name");
      free(name);
      return false;
    }

    // receive the optional whitespace
    if (req_recv(&c, 1, MSG_PEEK) != 1) {
      req_debug("failed to receive the HTTP header OWS");
      free(name);
      return false;
    }

    // OWS = *( SP / HTAB )
    if ((' ' == c || '\t' == c) && req_recv(&c, 1, MSG_WAITALL) != 1) {
      req_debug("failed to skip the HTTP header OWS");
      free(name);
      return false;
    }

    // receive the header value
    if ((size = req_recv_alloc(&value, ctorm_http_header_value_max, '\r')) <=
        0) {
      req_debug("failed to receive the HTTP header value");
      free(name);
      return false;
    }

    if (req_recv_char(NULL) != '\n') {
      req_debug("failed to receive the CRLF for the HTTP header");
      free(name);
      free(value);
      return false;
    }

    // remove the trailing OWS
    if (' ' == value[size - 1] || '\t' == value[size - 1])
      value[size - 1] = 0;

    if (!ctorm_http_is_valid_header_value(value, size)) {
      req_debug("invalid HTTP header value");
      free(name);
      free(value);
      return false;
    }

    // add the new headers to the request
    ctorm_headers_set(req->headers, name, value, true);
    name = value = NULL;

    // increase the header counter
    count++;
  }

  if (req_recv_char(NULL) != '\r' || req_recv_char(NULL) != '\n') {
    req_debug("failed to receive the CRLF after the headers");
    req->code = 400;
    return false;
  }

  char *content_len  = ctorm_req_get(req, CTORM_HTTP_CONTENT_LENGTH);
  char *transfer_enc = ctorm_req_get(req, CTORM_HTTP_TRANSFER_ENCODING);
  char *host         = ctorm_req_get(req, CTORM_HTTP_HOST);

  // if no host is specified in the URI, read it from the host header
  if (NULL == req->host && NULL != host)
    req->host = strdup(host);

  if (NULL == req->host) {
    req_debug("missing request host");
    return false;
  }

  if ((NULL != content_len || NULL != transfer_enc) &&
      !ctorm_http_method_allows_req_body(req->method)) {
    req_debug("request contains a body but it's not allowed");
    return false;
  }

  if (NULL != transfer_enc) {
    req_debug("transfer encoding is not supported");
    req->code = 501;
    return false;
  }

  if (NULL == content_len || (req->body_size = atol(content_len)) < 0)
    req->body_size = 0;

  if (0 == req->body_size && ctorm_http_method_needs_req_body(req->method)) {
    req_debug("missing or empty request body");
    req->code = 400;
    return false;
  }

  req->code = 200;
  return true;
}

char *ctorm_req_query(ctorm_req_t *req, char *name) {
  if (NULL == name) {
    errno = CTORM_ERR_BAD_QUERY_PTR;
    return NULL;
  }

  return ctorm_query_get(req->queries, name);
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

ctorm_query_t *ctorm_req_form(ctorm_req_t *req) {
  if (NULL != req->body_form)
    return req->body_form;

  char   *type = ctorm_req_get(req, CTORM_HTTP_CONTENT_TYPE);
  int64_t size = 0;

  if (NULL == type ||
      !cu_startswith(type, "application/x-www-form-urlencoded")) {
    errno = CTORM_ERR_BAD_CONTENT_TYPE;
    return NULL;
  }

  if ((size = req->body_size) <= 0) {
    errno = CTORM_ERR_EMPTY_BODY;
    return NULL;
  }

  char data[size];
  memset(data, 0, size);

  if (ctorm_req_body(req, data, size) != size)
    return NULL; // errno set by ctorm_req_body()

  // errno is set by ctorm_query_parse() if the it fails
  return req->body_form = ctorm_query_parse(data, size);
}

cJSON *ctorm_req_json(ctorm_req_t *req) {
#if CTORM_JSON_SUPPORT
  if (NULL != req->body_json)
    return req->body_json;

  char   *type = ctorm_req_get(req, CTORM_HTTP_CONTENT_TYPE);
  int64_t size = 0;

  if (!cu_startswith(type, "application/json")) {
    errno = CTORM_ERR_BAD_CONTENT_TYPE;
    return NULL;
  }

  if ((size = req->body_size) <= 0) {
    errno = CTORM_ERR_EMPTY_BODY;
    return NULL;
  }

  // receive all the data to provide to ctorm_json_decode()
  char data[size + 1];
  memset(data, 0, size + 1);

  if (ctorm_req_body(req, data, size) != size)
    return NULL; // errno set by ctorm_req_body()

  // errno is set by ctorm_json_decode() if the it fails
  return req->body_json = ctorm_json_decode(data);
#else
  errno = CTORM_ERR_NO_JSON_SUPPORT;
  return NULL;
#endif
}

const char *ctorm_req_method(ctorm_req_t *req) {
  if (ctorm_http_code_is_error(req->code))
    return NULL;
  return ctorm_http_method_name(req->method);
}

char *ctorm_req_get(ctorm_req_t *req, char *name) {
  if (NULL == name) {
    errno = CTORM_ERR_BAD_HEADER_PTR;
    return NULL;
  }

  return ctorm_headers_get(req->headers, name);
}

int64_t ctorm_req_body(ctorm_req_t *req, char *buffer, int64_t size) {
  if (NULL == buffer || size <= 0) {
    errno = CTORM_ERR_BAD_BUFFER;
    return -1;
  }

  if (req->body_size <= 0) {
    errno = CTORM_ERR_EMPTY_BODY;
    return -1;
  }

  if (size > req->body_size)
    size = req->body_size;
  req->body_size -= size;

  if (size == 0)
    return 0;

  return req_recv(buffer, size, MSG_WAITALL);
}

bool ctorm_req_persist(ctorm_req_t *req) {
  if (ctorm_http_code_is_error(req->code))
    return false;

  char *con = ctorm_req_get(req, "connection");

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
