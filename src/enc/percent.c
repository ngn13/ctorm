#include "enc/percent.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

uint64_t ctorm_percent_decode(char *data, uint64_t size) {
  if (NULL == data)
    return 0;

  if (size == 0)
    size = cu_strlen(data);

  char    *cur = NULL, *pos = NULL;
  uint64_t len = 0;
  uint8_t  val = 0;

  for (cur = pos = data; *cur != 0; cur++, pos++, size--, len++) {
    if (*cur == '+') {
      *pos = ' ';
      continue;
    }

    if (*cur == '%' && size > 2) {
      sscanf(cur + 1, "%02hhx", &val);
      *pos = val;
      cur += 2;
      size -= 2;
      continue;
    }

    *pos = *cur;
  }

  *pos = 0;
  return len;
}
