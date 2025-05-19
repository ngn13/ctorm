#include "enc/percent.h"
#include "util.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

uint32_t ctorm_percent_decode(char *data, uint32_t size) {
  if (NULL == data)
    return 0;

  if (size == 0)
    size = cu_strlen(data);

  char    *cur = data, *pos = data;
  uint32_t len = 0;
  uint8_t  val = 0;

  for (; *cur != 0 && size > 0; cur++, pos++, size--, len++) {
    switch (*cur) {
    // '+' in percent encoding means space
    case '+':
      *pos = ' ';
      continue;

    // '%' marks the start of a percent encoded char
    case '%':
      // we need at least 3 chars for a proper encoding
      if (size < 3)
        break;

      // scan the next 2 chars as hex
      sscanf(cur + 1, "%02hhx", &val);

      // ignore encoded NULL bytes for safety
      if (val == 0)
        break;

      *pos = val;

      // skip the encoding chars (last one is skipped in the for loop)
      cur += 2;
      size -= 2;
      continue;
    }

    *pos = *cur;
  }

  *pos = 0;
  return len;
}
