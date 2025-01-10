#pragma once
#include <stdint.h>
#include "util.h"

// URL encoding (application/x-www-form-urlencoded)
typedef pair_t enc_url_t;

enc_url_t *enc_url_parse(char *, uint64_t);      // parse URL encoded data from the byte array
char      *enc_url_get(enc_url_t *, char *name); // get a value by it's name from the URL encoded data
void       enc_url_free(enc_url_t *);            // free the URL encoded data

// JSON encoding (application/json)
#if __has_include(<cjson/cJSON.h>)
#include <cjson/cJSON.h>
#else
typedef void cJSON;
#endif

cJSON *enc_json_parse(char *);             // parse JSON encoded data from the byte array
char  *enc_json_dump(cJSON *, uint64_t *); // dump JSON encoded data to a byte array
void   enc_json_free(cJSON *);             // free the JSON encoded data
