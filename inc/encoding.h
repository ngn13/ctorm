#pragma once

#include <stdint.h>
#include "pair.h"

#if __has_include(<cjson/cJSON.h>)
#include <cjson/cJSON.h>
#else
typedef void cJSON;
#endif

// JSON encoding

/*!

 * Parse JSON encoded data from the data buffer. Please note that this function
 * will return NULL and fail if you did not compile ctorm with cJSON support.

 * @param[in] data: Data buffer
 * @return    Returns JSON decoded data stored in cJSON structure

*/
cJSON *ctorm_json_decode(char *data);

/*!

 * Encode JSON decoded data. Please note that this function will return NULL and
 * fail if you did not compile ctorm with cJSON support.

 * @param[in]  json: JSON decoded data
 * @return     JSON encoded data as a string, you should free() this pointer
 *             when you are done with it

*/
char *ctorm_json_encode(cJSON *json);

/*!

 * Free the JSON decoded data structure. Please note that this function will not
 * do any operation if you did not compile ctorm with cJSON support.

 * @param[in] json: JSON decoded data structure

*/
void ctorm_json_free(cJSON *json);

// URI query encoding

typedef ctorm_pair_t ctorm_query_t; /// Stores URL query data

/*!

 * Parse the URL query data from the provided data buffer. Parsed data will be
 * stored in a @ref ctorm_query_t structure, which will be returned as the
 * result. When you are done with the query data, you need to free this
 * structure with @ref ctorm_query_free

 * @param[in] data: Data buffer
 * @param[in] size: Size of the data buffer. If no size is specified, NULL
 *                  terminated string size of the data buffer will be used
 * @return    Query data

*/
ctorm_query_t *ctorm_query_parse(char *data, uint32_t size);

/*!

 * Get a value from the provided query data using the key name

 * @param[in] data: Query data
 * @param[in] name: Key name
 * @return    Value associated with the key

*/
char *ctorm_query_get(ctorm_query_t *data, char *name);

/*!

 * Free the query data structure

 * @param[in] data: Query data

*/
void ctorm_query_free(ctorm_query_t *data);

// URI percent encoding

/*!

 * Decode the provided percent encoded data buffer. Result will be overwritten
 * to the buffer.

 * @param[in] data: Data buffer
 * @param[in] size: Size of the data buffer. If no size is specified, NULL
 *                  terminated string size of the data buffer will be used
 * @return    Decoded size of the data buffer

*/
uint32_t ctorm_percent_decode(char *data, uint32_t size);
