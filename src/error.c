#include "error.h"

#include <pthread.h>
#include <string.h>

struct ctorm_error_desc {
  uint16_t    code;
  const char *str;
};

struct ctorm_error_desc _ctorm_err_descs[] = {
    {CTORM_ERR_BAD_TCP_TIMEOUT,    "invalid TCP timeout"                    },
    {CTORM_ERR_BAD_POOL_SIZE,      "invalid pool size"                      },
    {CTORM_ERR_BAD_PORT,           "bad port number"                        },
    {CTORM_ERR_BAD_HOSTNAME,       "bad hostname"                           },
    {CTORM_ERR_BAD_PATH,           "invalid HTTP path (should start with /)"},
    {CTORM_ERR_BAD_MAX_CONN_COUNT, "invalid max connection count"           },
    {CTORM_ERR_BAD_RESPONSE_CODE,  "specified response code is invalid"     },
    {CTORM_ERR_BAD_CONTENT_TYPE,
     "body is not using the requested content type"                         },
    {CTORM_ERR_BAD_BUFFER,         "invalid buffer pointer or size"         },

    {CTORM_ERR_BAD_APP_PTR,        "invalid app pointer"                    },
    {CTORM_ERR_BAD_URI_PTR,        "invalid URI pointer"                    },
    {CTORM_ERR_BAD_JSON_PTR,       "invalid cJSON pointer"                  },
    {CTORM_ERR_BAD_FMT_PTR,        "invalid string format pointer"          },
    {CTORM_ERR_BAD_PATH_PTR,       "invalid path pointer"                   },
    {CTORM_ERR_BAD_DATA_PTR,       "invalid data pointer"                   },
    {CTORM_ERR_BAD_LOCAL_PTR,      "invalid local name pointer"             },
    {CTORM_ERR_BAD_PARAM_PTR,      "invalid URL parameter name pointer"     },
    {CTORM_ERR_BAD_QUERY_PTR,      "invalid URL query name pointer"         },
    {CTORM_ERR_BAD_HEADER_PTR,     "invalid header name/value pointer"      },
    {CTORM_ERR_BAD_HOST_PTR,       "invalid host address pointer"           },

    {CTORM_ERR_PORT_TOO_LARGE,     "host port number is too large"          },
    {CTORM_ERR_HOSTNAME_TOO_LARGE, "hostname is too large"                  },

    {CTORM_ERR_POOL_FAIL,          "failed to create thread pool"           },
    {CTORM_ERR_LISTEN_FAIL,        "failed to listen on the interface"      },
    {CTORM_ERR_SOCKET_OPT_FAIL,    "failed to set socket options"           },
    {CTORM_ERR_FCNTL_FAIL,         "failed to modify socket with fcntl"     },
    {CTORM_ERR_ALLOC_FAIL,         "memory allocation failed"               },
    {CTORM_ERR_SEEK_FAIL,          "file seek failed"                       },
    {CTORM_ERR_READ_FAIL,          "failed to read the file"                },
    {CTORM_ERR_MUTEX_FAIL,         "failed to initialize thread mutex"      },

    {CTORM_ERR_NOT_EXISTS,         "file does not exist"                    },
    {CTORM_ERR_NO_READ_PERM,       "missing read permission"                },
    {CTORM_ERR_NO_JSON_SUPPORT,    "library not compiled with JSON support" },
    {CTORM_ERR_EMPTY_BODY,         "body is empty"                          },

    {CTORM_ERR_UNKNOWN,            "unknown error"                          },
    {0,                            NULL                                     }
};

const char *ctorm_error_from(int code) {
  struct ctorm_error_desc *desc = &_ctorm_err_descs[0];

  for (; desc->str != NULL; desc++)
    if (desc->code == code)
      return desc->str;

  return strerror(code);
}
