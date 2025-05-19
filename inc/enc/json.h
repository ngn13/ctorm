#pragma once
#include <stdint.h>

#if __has_include(<cjson/cJSON.h>)
#include <cjson/cJSON.h>
#else
typedef void cJSON;
#endif

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
