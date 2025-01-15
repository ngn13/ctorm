#include "errors.h"
#include <string.h>

struct ctorm_error_desc descs[] = {
    {BadTcpTimeout,       "invalid TCP timeout"                         },
    {BadPoolSize,         "invalid pool size"                           },
    {PoolFailed,          "failed to create threadpool"                 },
    {ListenFailed,        "failed to listen on the interface"           },
    {BadHost,             "bad host address"                            },
    {BadPort,             "invalid port number"                         },
    {OptFailed,           "failed to set socket options"                },
    {AllocFailed,         "memory allocation failed"                    },
    {UnknownErr,          "unknown error"                               },
    {BadReadPerm,         "permissions do not allow reading"            },
    {SizeFail,            "failed to get the size of the file"          },
    {CantRead,            "failed to read the file"                     },
    {FileNotExists,       "file does not exist"                         },
    {BadPath,             "invalid HTTP path (should start with /)"     },
    {InvalidAppPointer,   "invalid app pointer"                         },
    {BadUrlPointer,       "invalid URL pointer"                         },
    {BadJsonPointer,      "invalid cJSON pointer"                       },
    {BadFmtPointer,       "invalid string format pointer"               },
    {BadPathPointer,      "invalid path pointer"                        },
    {BadDataPointer,      "invalid data pointer"                        },
    {BadLocalPointer,     "invalid local name pointer"                  },
    {BadParamPointer,     "invalid URL parameter name pointer"          },
    {BadQueryPointer,     "invalid URL query name pointer"              },
    {BadHeaderPointer,    "invalid header name/value pointer"           },
    {BadMaxConnCount,     "invalid max connection count"                },
    {NoJSONSupport,       "library not compiled with JSON support"      },
    {MutexFail,           "failed to initialize thread mutex"           },
    {BadResponseCode,     "specified response code is invalid"          },
    {ResponseAlreadySent, "response has already been sent"              },
    {InvalidContentType,  "body is not using the requested content type"},
    {EmptyBody,           "body is empty"                               },
    {BodyRecvFail,        "failed to receive the body"                  },
    {PortTooLarge,        "host port number is too large"               },
    {NameTooLarge,        "host name is too large"                      },
    {BadName,             "invalid hostname"                            },
};

const char *ctorm_geterror_from_code(ctorm_error_t code) {
  for (int i = 0; i < sizeof(descs) / sizeof(*descs); i++) {
    if (descs[i].code == code)
      return descs[i].desc;
  }
  return strerror(code);
}
