#include "../include/res.h"
#include "../include/util.h"
#include "../include/errors.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void res_init(res_t *res) {
  table_init(&res->headers);
  table_init(&res->render);

  res->version  = NULL;
  res->bodysize = 0;
  res->body     = NULL;
  res->code     = 200;

  res_set(res, "Server", "ctorm");
  res_set(res, "Connection", "close");
  res_set(res, "Content-Length", "0");

  struct tm *gmt;
  time_t     raw;

  time(&raw);
  gmt = gmtime(&raw);

  char date[50];
  // https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Date
  strftime(date, 50, "%a, %d %b %Y %H:%M:%S GMT", gmt);
  res_set(res, "Date", date);
}

void res_free(res_t *res) {
  table_free(&res->headers);
  table_free(&res->render);
  free(res->body);
}

void res_set(res_t *res, char *name, char *value) {
  if (table_update(&res->headers, name, strdup(value)))
    return;

  table_add(&res->headers, strdup(name), true);
  table_set(&res->headers, strdup(value));
}

void res_del(res_t *res, char *name){
  table_del(&res->headers, name);
}

void res_send(res_t *res, char *data, size_t size) {
  free(res->body);

  if(NULL == data){
    res->body = NULL;
    res->bodysize = 0;
    return;
  }

  if(size <= 0)
    res->bodysize = strlen(data);

  res->body = malloc(res->bodysize);
  memcpy(res->body, data, res->bodysize);

  int  len = digits(res->bodysize) + 1;
  char buf[len];

  snprintf(buf, len, "%lu", res->bodysize);
  res_set(res, "Content-Length", buf);
}

bool res_sendfile(res_t *res, char *path) {
  if(!file_canread(path)){
    if(errno == ENOENT)
      errno = FileNotExists;
    else
      errno = BadReadPerm;
    return false;
  }

  free(res->body);
  res->body = NULL;

  res->bodysize = file_size(path);
  if(res->bodysize < 0){
    errno = SizeFail; 
    res->bodysize = 0;
    return false;
  }

  res->body = malloc(res->bodysize);

  if(!file_read(path, res->body, res->bodysize)){
    errno = CantRead;
    return false;
  }

  if (NULL == res->body)
    return false;

  int  len = digits(res->bodysize) + 1;
  char size[len];

  snprintf(size, len, "%lu", res->bodysize);
  res_set(res, "Content-Length", size);

  if (endswith(path, ".html"))
    res_set(res, "Content-Type", "text/html; charset=utf-8");
  else if (endswith(path, ".json"))
    res_set(res, "Content-Type", "application/json; charset=utf-8");
  else if (endswith(path, ".css"))
    res_set(res, "Content-Type", "text/css; charset=utf-8");
  else if (endswith(path, ".js"))
    res_set(res, "Content-Type", "text/javascript; charset=utf-8");
  else
    res_set(res, "Content-Type", "text/plain; charset=utf-8");

  return true;
}

size_t res_size(res_t *res) {
  size_t size = 0;
  size += http_static.version_len + 1; // "HTTP/1.1 "
  size += 5; // "200\r\n"
  
  char **cur = table_next(&res->headers, NULL);
  while (cur) {
    size += strlen(cur[0]) + 2; // "User-Agent: "
    size += strlen(cur[1]) + 2; // "curl\r\n"
    cur = table_next(&res->headers, cur);
  }

  size += 2; // "\r\n"
  
  // body
  size += res->bodysize;
  return size;
}

void res_tostr(res_t *res, char *str) {
  char **cur = table_next(&res->headers, NULL);
  size_t index = 0;

  // fix the HTTP code if its invalid
  if(res->code > 999 || res->code < 100)
    res->code = 200;

  if (NULL == res->version)
    index += sprintf(str, "HTTP/1.1 %d\r\n", res->code);
  else
    index += sprintf(str, "%s %d\r\n", res->version, res->code);

  while (cur) {
    index += sprintf(str + index, "%s: %s\r\n", cur[0], cur[1]);
    cur = table_next(&res->headers, cur);
  }

  index += sprintf(str + index, "\r\n");
  if (res->bodysize > 0)
    memcpy(str + index, res->body, res->bodysize);
}
