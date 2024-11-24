#include "../include/parse.h"
#include "../include/table.h"

#include "../include/util.h"

#include "../include/log.h"
#include "../include/req.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>

void req_init(req_t *req) {
  headers_init(&req->headers);
  table_init(&req->query, NULL, NULL);

  req->cancel  = false;
  req->version = NULL;
  req->encpath = NULL;
  req->path    = NULL;

  req->bodysize = 0;
  req->body     = NULL;
}

void req_free(req_t *req) {
  headers_free(&req->headers);
  table_free(&req->query);

  free(req->body);

  if (req->encpath == req->path) {
    free(req->encpath);
    return;
  }

  free(req->encpath);
  free(req->path);
}

char *req_query(req_t *req, char *name) {
  table_node_t *query = table_get(&req->query, name);

  if (NULL == query)
    return NULL;

  return query->value;
}

bool req_body(req_t *req, char *buffer) {
  if (NULL == req->body)
    return false;

  memcpy(buffer, req->body, req->bodysize);
  buffer[req->bodysize] = 0;
  return true;
}

size_t req_body_size(req_t *req) {
  if (req->bodysize <= 0)
    return 0;
  return req->bodysize + 1;
}

bool req_form_parse(req_t *req, form_t *form) {
  size_t size = req_body_size(req);
  if (size == 0)
    return false;

  char *contentt = req_get(req, "content-type");
  if (!startswith(contentt, "application/x-www-form-urlencoded"))
    return false;

  char data[size];
  if (!req_body(req, data))
    return false;

  form_init(form);

  if (!parse_form(form, data)) {
    form_free(form);
    return false;
  }

  return true;
}

void req_form_free(form_t *form) {
  form_free(form);
}

cJSON *req_json_parse(req_t *req) {
  size_t size = req_body_size(req);
  if (size == 0)
    return NULL;

  char *contentt = req_get(req, "content-type");
  if (!startswith(contentt, "application/json"))
    return NULL;

  char data[size];
  if (!req_body(req, data))
    return NULL;

  return cJSON_Parse(data);
}

void req_json_free(cJSON *json) {
  if (NULL != json)
    cJSON_Delete(json);
}

char *req_method(req_t *req) {
  return http_method_name(req->method);
}

void req_set(req_t *req, char *name, char *value, bool dup) {
  if (NULL == name || NULL == value)
    errno = BadHeaderPointer;
  else
    headers_set(&req->headers, name, value, dup);
}

size_t req_size(req_t *req) {
  char    *method = req_method(req);
  header_t cur;

  size_t size = strlen(method) + 1; // "GET "
  size += strlen(req->encpath) + 1; // "/ "
  size += strlen(req->version) + 1; // "HTTP/1.1\n"

  headers_start(&cur);

  while (headers_next(&req->headers, &cur)) {
    size += strlen(cur.key) + 2;   // "User-Agent: "
    size += strlen(cur.value) + 1; // "curl\n"
  }

  size += 1; // "\n"
  return size;
}

void req_tostr(req_t *req, char *str) {
  char    *method = req_method(req);
  size_t   index  = 0;
  header_t cur;

  index += sprintf(str + index, "%s %s %s\n", method, req->encpath, req->version);

  headers_start(&cur);

  while (headers_next(&req->headers, &cur))
    index += sprintf(str + index, "%s: %s\n", cur.key, cur.value);

  sprintf(str + index, "\n");
}

char *req_get(req_t *req, char *name) {
  return headers_get(&req->headers, name);
}
