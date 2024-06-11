#include "../include/errors.h"
#include <errno.h>
#include <stdlib.h>

struct app_error_desc_t descs[] = {
    {.code = BadTcpTimeout, .desc = "invalid TCP timeout"               },
    {.code = BadPoolSize,   .desc = "invalid pool size"                 },
    {.code = EventFailed,   .desc = "failed to create event base"       },
    {.code = PoolFailed,    .desc = "failed to create threadpool"       },
    {.code = ListenFailed,  .desc = "failed to listen on the interface" },
    {.code = BadAddress,    .desc = "bad address for the interface"     },
    {.code = BadPort,       .desc = "bad port number for the interface" },
    {.code = OptFailed,     .desc = "failed to set socket options"      },
    {.code = AllocFailed,   .desc = "memory allocation failed"          },
    {.code = UnknownErr,    .desc = "unknown error"                     },
    {.code = BadReadPerm,   .desc = "permissions do not allow reading"  },
    {.code = SizeFail,      .desc = "failed to get the size of the file"},
    {.code = CantRead,      .desc = "failed to read the file"           },
    {.code = FileNotExists, .desc = "file does not exist"               },
};

char *app_geterror_code(app_error_t code) {
  for (int i = 0; i < sizeof(descs) / sizeof(struct app_error_desc_t); i++){
    if(descs[i].code == code)
      return descs[i].desc;
  }
  return NULL;
}

char *app_geterror() {
  return app_geterror_code(errno);
}
