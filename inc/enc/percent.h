#pragma once
#include <stdint.h>
#include "pair.h"

/*!

 * Decode the provided percent encoded data buffer. Result will be overwritten
 * to the buffer.

 * @param[in] data: Data buffer
 * @param[in] size: Size of the data buffer. If no size is specified, NULL
 *                  terminated string size of the data buffer will be used
 * @return    Pointer to the decoded data buffer

*/
uint64_t ctorm_percent_decode(char *data, uint64_t size);
