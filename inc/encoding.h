#pragma once
#include <stdint.h>
#include "pair.h"

// URL encoding (application/x-www-form-urlencoded)

/*!

 * @brief URL decoded data

 * This type is used for storing URL decoded data

*/
typedef ctorm_pair_t ctorm_url_t;

/*!

 * Parse URL encoded data from the data buffer

 * @param[in] data: Data buffer
 * @param[in] len: Data buffer length
 * @return    URL encoded data

*/
ctorm_url_t *ctorm_url_parse(char *data, uint64_t len);

/*!

 * Get a value from the URL encoded data

 * @param[in] data: URL decoded data
 * @param[in] name: Key name
 * @return    Value associated with the key

*/
char *ctorm_url_get(ctorm_url_t *data, char *name);

/*!

 * Free the URL decoded data

 * @param[in] data: URL encoded data

*/
void ctorm_url_free(ctorm_url_t *data);

// JSON encoding (application/json)

#if __has_include(<cjson/cJSON.h>)
#include <cjson/cJSON.h>
#else
typedef void cJSON;
#endif

/*!

 * Parse JSON encoded data from the data buffer

 * @param[in] data: Data buffer

*/
cJSON *ctorm_json_parse(char *data);

/*!

 * Dump JSON decoded data to a buffer

 * @param[in]  json: JSON decoded data
 * @param[out] size: JSON encoded data size
 * @return     JSON encoded data

*/
char *ctorm_json_dump(cJSON *json, uint64_t *size);

/*!

 * Free the JSON decoded data

 * @param[in] json: JSON decoded data

*/
void ctorm_json_free(cJSON *json);
