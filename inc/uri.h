#pragma once
#include "enc/query.h"

typedef struct {
  char          *scheme, *userinfo, *host, *path, *fragment;
  ctorm_query_t *query;
  uint16_t       port;
} ctorm_uri_t;

#ifndef CTORM_EXPORT
#include <stdbool.h>

#define ctorm_uri_init(uri) bzero((uri), sizeof(ctorm_uri_t))
void ctorm_uri_free(ctorm_uri_t *uri);

bool  ctorm_uri_parse(ctorm_uri_t *uri, char *str);
char *ctorm_uri_parse_auth(ctorm_uri_t *uri, char *auth);
char *ctorm_uri_parse_host(ctorm_uri_t *uri, char *addr);
char *ctorm_uri_parse_path(ctorm_uri_t *uri, char *path);

#endif
