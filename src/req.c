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

  req->version = NULL;
  req->encpath = NULL;
  req->path    = NULL;
  req->addr    = NULL;

  req->bodysize = 0;
  req->body     = NULL;
}

void req_free(req_t *req) {
  table_free(&req->headers);
  table_free(&req->query);
  
  free(req->body);
  free(req->addr);

  if(req->encpath == req->path){
    free(req->encpath);
    return;
  }

  free(req->encpath);
  free(req->path);
}

char *req_query(req_t *req, char *name) {
  return table_get(&req->query, name);
}

bool req_body(req_t *req, char *buffer) {
  if (NULL == req->body)
    return false;

  memcpy(buffer, req->body, req->bodysize);
  buffer[req->bodysize] = 0;
  return true;
}

size_t req_body_size(req_t *req){
  if(req->bodysize <= 0)
    return 0;
  return req->bodysize+1;
}

table_t *req_body_parse(req_t *req) {
  size_t size = req_body_size(req);
  if(size == 0)
    return NULL;

  char *contentt = req_header(req, "content-type");
  if (!startswith(contentt, "application/x-www-form-urlencoded"))
    return NULL;
  
  char data[size];
  req_body(req, data);

  table_t *table = malloc(sizeof(table_t));
  table_init(table);
  
  if(!parse_form(table, data)){
    table_free(table);
    free(table);
    return NULL;
  }

  return table;
}

char *req_body_get(table_t *body, char *key) {
  return table_get(body, key);
}

void req_body_free(table_t *body) {
  table_free(body);
}

char *req_method(req_t *req) {
  return http_method_name(req->method);
}

void req_add_header(req_t *req, char *name) {
  table_add(&req->headers, strdup(name), true);
}

void req_add_header_value(req_t *req, char *value) {
  table_set(&req->headers, strdup(value));
}

size_t req_size(req_t *req) {
  char *method = req_method(req);

  size_t size = strlen(method) + 1; // "GET "
  size += strlen(req->encpath) + 1; // "/ "
  size += strlen(req->version) + 1; // "HTTP/1.1\n"

  char **cur = table_next(&req->headers, NULL);
  while (cur) {
    size += strlen(cur[0]) + 2; // "User-Agent: "
    size += strlen(cur[1]) + 1; // "curl\n"
    cur = table_next(&req->headers, cur);
  }

  size += 1; // "\n"
  return size;
}

void req_tostr(req_t *req, char *str) {
  char *method = req_method(req);
  char **cur = table_next(&req->headers, NULL);
  size_t index = 0;
  
  index += sprintf(str+index, "%s %s %s\n", method, req->encpath, req->version);

  while (cur) {
    index += sprintf(str + index, "%s: %s\n", cur[0], cur[1]);
    cur = table_next(&req->headers, cur);
  }

  sprintf(str+index, "\n");
}

char *req_header(req_t *req, char *name) {
  size_t len = strlen(name);
  char low[len+1];
  stolower(name, low);

  char **cur = table_next(&req->headers, NULL);
  while (cur) {
    stolower(cur[0], cur[0]);

    if (eq(low, cur[0])) {
      char *ret = cur[1];
      free(cur);
      return ret;
    }

    cur = table_next(&req->headers, cur);
  }

  return NULL;
}
