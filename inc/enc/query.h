#pragma once
#include <stdint.h>
#include "pair.h"

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
