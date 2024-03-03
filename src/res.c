#include "../include/util.h"
#include "../include/res.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

void res_init(res_t* res){
  pair_init(&res->headers);
  pair_init(&res->render);
  res->version = NULL;
  res->body = NULL;
  res->code = 200;

  res_set(res, "Server", "ctorm");
  res_set(res, "Connection", "close");
  res_set(res, "Content-Length", "0");

  struct tm* gmt;
  time_t raw; 

  time(&raw);
  gmt = gmtime(&raw);

  char date[50];
  // https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Date
  strftime(date, 50, "%a, %d %b %Y %H:%M:%S GMT", gmt); 
  res_set(res, "Date", date);
}

void res_free(res_t* res){
  pair_free(&res->headers);
  pair_free(&res->render);
  free(res->version);
  free(res->body);
}

void res_set(res_t* res, char* name, char* value){
  for(int i = 0; i < res->headers.size; i++){
    if(eq(res->headers.list[i].name, name)){
      if(res->headers.list[i].alloced)
        free(res->headers.list[i].value);
      res->headers.list[i].value = strdup(value);
      return;
    }
  }

  pair_add(&res->headers, strdup(name), true);
  pair_set(&res->headers, strdup(value));
}

void res_send(res_t* res, char* data){
  free(res->body);
  res->body = strdup(data);

  int len = digits(strlen(res->body))+1;
  char size[len];

  snprintf(size, len, "%lu", strlen(res->body));
  res_set(res, "Content-Length", size);
}

void res_render_add(res_t* res, char* key, char* value){
  pair_add(&res->render, strdup(key), true);
  pair_set(&res->render, strdup(value));
}

bool res_render(res_t* res, char* path){
  int sz;
  res->body = readall(path, &sz);
  if(NULL == res->body)
    return false;

  for(int i = 0; i < res->render.size; i++){
    int keysize = strlen(res->render.list[i].name)+6;
    char pattern[keysize];

    snprintf(pattern, keysize, "{{%s}}", res->render.list[i].name);
    res->body = strrep(res->body, pattern, res->render.list[i].value);
  }

  sz = strlen(res->body);
  int len = digits(sz)+1;
  char size[len];
  snprintf(size, len, "%d", sz);

  res_set(res, "Content-Length", size);
  return true;
}

bool res_sendfile(res_t* res, char* path){
  if(endswith(path, ".html"))
    res_set(res, "Content-Type", "text/html; charset=utf-8");
  else if(endswith(path, ".json"))
    res_set(res, "Content-Type", "application/json; charset=utf-8");
  else if(endswith(path, ".css"))
    res_set(res, "Content-Type", "text/css; charset=utf-8");
  else if(endswith(path, ".js"))
    res_set(res, "Content-Type", "text/javascript; charset=utf-8");
  else
    res_set(res, "Content-Type", "text/plain; charset=utf-8");

  int sz;
  free(res->body);
  res->body = readall(path, &sz);

  if(NULL == res->body)
    return false;

  int len = digits(sz)+1;
  char size[len];
  snprintf(size, len, "%d", sz);

  res_set(res, "Content-Length", size);
  return true;
}

int res_size(res_t* res){
  // HTTP/1.1 301 
  int size = 1;
  if(NULL == res->version)
    size += strlen("HTTP/1.1");
  else 
    size += strlen(res->version);
  size += 3;
  
  // Name: value
  for(int i = 0; i < res->headers.size; i++){
    size += strlen(res->headers.list[i].name)+2;
    size += strlen(res->headers.list[i].value)+1;
  }

  // Newline + Body
  if(NULL == res->body)
    size += 2;
  else 
    size += strlen(res->body)+2;
  return size;
}

void res_tostr(res_t* res, char* str){
  if(NULL == res->version)
    sprintf(str, "HTTP/1.1 %d\n", res->code);
  else
    sprintf(str, "%s %d\n", 
        res->version, res->code);

  for(int i = 0; i < res->headers.size; i++){
    sprintf(str, "%s%s: %s\n", str, 
        res->headers.list[i].name, 
        res->headers.list[i].value);
  }

  if(NULL == res->body)
    sprintf(str, "%s\n", str);
  else
    sprintf(str, "%s\n%s", str, res->body);
}

void res_set_version(res_t* res, char* version){
  res->version = strdup(version);
}
