#include "../include/errors.h"

#include <stdlib.h>
#include <string.h>

#include <errno.h>

struct app_error_desc_t descs[] = {
    {.code = BadTcpTimeout,     .desc = "invalid TCP timeout"                    },
    {.code = BadPoolSize,       .desc = "invalid pool size"                      },
    {.code = PoolFailed,        .desc = "failed to create threadpool"            },
    {.code = ListenFailed,      .desc = "failed to listen on the interface"      },
    {.code = BadAddress,        .desc = "bad address for the interface"          },
    {.code = BadPort,           .desc = "bad port number for the interface"      },
    {.code = OptFailed,         .desc = "failed to set socket options"           },
    {.code = AllocFailed,       .desc = "memory allocation failed"               },
    {.code = UnknownErr,        .desc = "unknown error"                          },
    {.code = BadReadPerm,       .desc = "permissions do not allow reading"       },
    {.code = SizeFail,          .desc = "failed to get the size of the file"     },
    {.code = CantRead,          .desc = "failed to read the file"                },
    {.code = FileNotExists,     .desc = "file does not exist"                    },
    {.code = BadPath,           .desc = "invalid HTTP path (should start with /)"},
    {.code = InvalidAppPointer, .desc = "invalid app pointer"                    },
    {.code = BadUrlPointer,     .desc = "invalid URL pointer"                    },
    {.code = BadJsonPointer,    .desc = "invalid cJSON pointer"                  },
    {.code = BadFmtPointer,     .desc = "invalid string format pointer"          },
    {.code = BadPathPointer,    .desc = "invalid path pointer"                   },
    {.code = BadDataPointer,    .desc = "invalid data pointer"                   },
    {.code = BadHeaderPointer,  .desc = "invalid header name/value pointer"      },
    {.code = BadMaxConnCount,   .desc = "invalid max connection count"           },
};

char *app_geterror_code(app_error_t code) {
  for (int i = 0; i < sizeof(descs) / sizeof(struct app_error_desc_t); i++) {
    if (descs[i].code == code)
      return descs[i].desc;
  }
  return strerror(code);
}

char *app_geterror() {
  return app_geterror_code(errno);
}
