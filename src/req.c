#include "../include/req.h"
#include "../include/log.h"
#include "../include/parse.h"
#include "../include/table.h"
#include "../include/util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void req_init(req_t *req) {
  table_init(&req->headers);
  table_init(&req->query);
  req->fullpath = NULL;
  req->encpath = NULL;
  req->version = NULL;
  req->path = NULL;
  req->body = NULL;
  req->bodysz = 0;
}

void req_free(req_t *req) {
  table_free(&req->headers);
  table_free(&req->query);
  free(req->fullpath);
  free(req->encpath);
  free(req->path);
  free(req->body);
}

char *req_query(req_t *req, char *name) { return table_get(&req->query, name); }

char *req_body(req_t *req) {
  if (NULL == req->body)
    return NULL;

  char *printable = malloc(req->bodysz + 1);
  memcpy(printable, req->body, req->bodysz);
  printable[req->bodysz] = '\0';
  return printable;
}

table_t *req_body_parse(req_t *req) {
  if (!req_has_body(req))
    return NULL;

  char *contentt = req_header(req, "Content-Type");
  if (!startswith(contentt, "application/x-www-form-urlencoded"))
    return NULL;

  char *print = req_body(req);
  table_t *data = malloc(sizeof(table_t));
  table_init(data);
  if (parse_urldata(data, print, strlen(print)) != P_OK) {
    free(print);
    table_free(data);
    free(data);
    return NULL;
  }

  free(print);
  return data;
}

char *req_body_get(table_t *body, char *key) { return table_get(body, key); }

void req_body_free(table_t *body) { table_free(body); }

char *req_method(req_t *req) {
  for (int i = 0; i < http_method_sz; i++)
    if (http_method_map[i].code == req->method)
      return http_method_map[i].name;
  return NULL;
}

bool req_has_body(req_t *req) {
  return req_can_have_body(req) && req_body_size(req) > 0;
}

bool req_can_have_body(req_t *req) {
  for (int i = 0; i < http_method_sz; i++)
    if (http_method_map[i].code == req->method)
      return http_method_map[i].body;
  return false;
}

int req_body_size(req_t *req) {
  char *val = req_header(req, "content-length");
  if (NULL == val)
    return 0;

  int size = atoi(val);
  if (size <= 0)
    return 0;

  return size;
}

void req_add_header(req_t *req, char *name) {
  table_add(&req->headers, strdup(name), true);
}

void req_add_header_value(req_t *req, char *value) {
  table_set(&req->headers, strdup(value));
}

int req_size(req_t *req) {
  char *method = req_method(req);

  // GET / HTTP/1.1
  int size = strlen(method) + 1;
  size += strlen(req->encpath) + 1;
  size += strlen(req->version) + 1;

  // Name: value
  char **cur = table_next(&req->headers, NULL);
  while (cur) {
    size += strlen(cur[0]) + 2;
    size += strlen(cur[1]) + 1;
    cur = table_next(&req->headers, cur);
  }

  // Empty newline
  size += 1;
  return size;
}

void req_tostr(req_t *req, char *str) {
  char *method = req_method(req);
  sprintf(str, "%s %s %s\n", method, req->encpath, req->version);

  char **cur = table_next(&req->headers, NULL);
  while (cur) {
    sprintf(str + strlen(str), "%s: %s\n", cur[0], cur[1]);
    cur = table_next(&req->headers, cur);
  }
}

char *req_header(req_t *req, char *name) {
  char *low = strdup(name);
  stolower(name, low);

  char **cur = table_next(&req->headers, NULL);
  while (cur) {
    char *copy = strdup(cur[0]);
    stolower(cur[0], copy);

    if (eq(low, copy)) {
      free(low);
      free(copy);
      char *ret = cur[1];
      free(cur);
      return ret;
    }

    free(copy);
    cur = table_next(&req->headers, cur);
  }

  free(low);
  return NULL;
}
