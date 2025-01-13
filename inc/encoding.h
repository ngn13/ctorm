#pragma once
#include <stdint.h>
#include "pair.h"

// URL encoding (application/x-www-form-urlencoded)
typedef ctorm_pair_t ctorm_url_t;

ctorm_url_t *ctorm_url_parse(char *data, uint64_t);        // parse URL encoded data from the byte array
char        *ctorm_url_get(ctorm_url_t *data, char *name); // get a value by it's name from the URL encoded data
void         ctorm_url_free(ctorm_url_t *data);            // free the URL encoded data

// JSON encoding (application/json)
#if __has_include(<cjson/cJSON.h>)
#include <cjson/cJSON.h>
#else
typedef void cJSON;
#endif

cJSON *ctorm_json_parse(char *data);                 // parse JSON encoded data from the byte array
char  *ctorm_json_dump(cJSON *json, uint64_t *size); // dump JSON encoded data to a byte array
void   ctorm_json_free(cJSON *json);                 // free the JSON encoded data
