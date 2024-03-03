#include "../include/errors.h"
#include <stdlib.h>
#include <errno.h>

struct error_desc_t descs[] = {
  {.code=EventFailed,   .desc="failed to create event base"},
  {.code=ListenFailed,  .desc="failed to listen on the interface"},
  {.code=BadAddress,    .desc="bad address for the interface"},
  {.code=BadPort,       .desc="bad port number for the interface"},
  {.code=OptFailed,     .desc="failed to set socket options"},
  {.code=AllocFailed,   .desc="memory allocation failed"},
  {.code=UnknownErr,    .desc="unknown error"},
};

char* geterror_code(int code) {
  for(int i = 0; i < sizeof(descs)/sizeof(struct error_desc_t); i++){
    return descs[i].desc;
  }
  return NULL;
}

char* geterror() {
  return geterror_code(errno);
}
