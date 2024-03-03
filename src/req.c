#include "../include/parse.h"
#include "../include/util.h"
#include "../include/log.h"
#include "../include/req.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void req_init(req_t* req){
  pair_init(&req->headers);
  pair_init(&req->query);
  req->fullpath = NULL;
  req->encpath = NULL;
  req->version = NULL;
  req->path = NULL;
  req->body = NULL;
  req->bodysz = 0;
}

void req_free(req_t* req){
  pair_free(&req->headers);
  pair_free(&req->query);
  free(req->fullpath);
  free(req->encpath);
  free(req->path);
  free(req->body);
}

char* req_query(req_t* req, char* name){
  for(int i = 0; i < req->query.size; i++)
    if(eq(req->query.list[i].name, name))
      return req->query.list[i].value;
  return NULL;
}

char* req_body(req_t* req){
  if(NULL == req->body)
    return NULL;

  char* printable = malloc(req->bodysz+1);
  memcpy(printable, req->body, req->bodysz);
  printable[req->bodysz] = '\0';
  return printable;
}

pairs_t* req_body_parse(req_t* req){
  if(!req_has_body(req))
    return NULL;
 
  char* contentt = req_header(req, "Content-Type");
  if(!startswith(contentt, "application/x-www-form-urlencoded"))
    return NULL;

  char* print = req_body(req);
  pairs_t* pairs = malloc(sizeof(pairs_t));
  pair_init(pairs);
  if(parse_urldata(pairs, print, strlen(print))!=P_OK){
    free(print);
    pair_free(pairs);
    free(pairs);
    return NULL;
  }
    
  free(print);
  return pairs;
}

char* req_body_get(pairs_t* body, char* key){
  for(int i = 0; i < body->size; i++)
    if(eq(body->list[i].name, key))
      return body->list[i].value;
  return NULL;
}

void req_body_free(pairs_t* body){
  pair_free(body);
}

char* req_method(req_t* req) {
  for(int i = 0; i < http_method_sz; i++)
    if(http_method_map[i].code == req->method)
      return http_method_map[i].name;
  return NULL;
}

bool req_has_body(req_t* req){
  return req_can_have_body(req) && req_body_size(req)>0;
}

bool req_can_have_body(req_t* req){
  for(int i = 0; i < http_method_sz; i++)
    if(http_method_map[i].code == req->method)
      return http_method_map[i].body; 
  return false;
}

int req_body_size(req_t* req){
  char* val = req_header(req, "content-length");
  if(NULL == val)
    return 0;

  int size = atoi(val);
  if(size <= 0)
    return 0;

  return size;
}

void req_add_header(req_t* req, char* name){
  pair_add(&req->headers, strdup(name), true);
}

void req_add_header_value(req_t* req, char* value){
  pair_set(&req->headers, strdup(value));
}

int req_size(req_t* req) {
  char* method = req_method(req);
  
  // GET / HTTP/1.1
  int size = strlen(method)+1;
  size += strlen(req->encpath)+1;
  size += strlen(req->version)+1;

  // Name: value
  for(int i = 0; i < req->headers.size; i++){
    size += strlen(req->headers.list[i].name)+2;
    size += strlen(req->headers.list[i].value)+1;
  }

  // Empty newline
  size += 1;
  return size;
}

void req_tostr(req_t* req, char* str){
  char* method = req_method(req);
  sprintf(str, "%s %s %s\n",
      method, req->encpath, req->version);

  for(int i = 0; i < req->headers.size; i++){
    sprintf(str, "%s%s: %s\n", str, 
        req->headers.list[i].name, 
        req->headers.list[i].value);
  }
}

char* req_header(req_t* req, char* name) {
  char* low = strdup(name);
  stolower(name, low);

  for(int i = 0; i < req->headers.size; i++){
    char* copy = strdup(req->headers.list[i].name);
    stolower(req->headers.list[i].name, copy);
    if(eq(low, copy)){
      free(low);
      free(copy);
      return req->headers.list[i].value;
    }
    free(copy);
  }

  free(low);
  return NULL;
}
