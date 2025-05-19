#include "error.h"
#include "app.h"

#include <pthread.h>
#include <string.h>

struct ctorm_error_desc {
  uint16_t    code;
  const char *str;
};

struct ctorm_error_desc _ctorm_err_descs[] = {
    {CTORM_ERR_BAD_TCP_TIMEOUT,       "invalid TCP timeout"                   },
    {CTORM_ERR_BAD_POOL_SIZE,         "invalid pool size"                     },
    {CTORM_ERR_BAD_MAX_CONN_COUNT,    "invalid max connection count"          },
    {CTORM_ERR_BAD_RESPONSE_CODE,     "specified response code is invalid"    },
    {CTORM_ERR_BAD_CONTENT_TYPE,
     "body is not using the requested content type"                           },
    {CTORM_ERR_BAD_BUFFER,            "invalid buffer pointer or size"        },
    {CTORM_ERR_BAD_URI_FORMAT,        "invalid URI format"                    },
    {CTORM_ERR_BAD_SCHEME,            "invalid URI scheme"                    },
    {CTORM_ERR_BAD_AUTHORITY,         "invalid URI authority"                 },
    {CTORM_ERR_BAD_USERINFO,          "invalid URI userinfo"                  },
    {CTORM_ERR_BAD_HOST,              "bad host"                              },
    {CTORM_ERR_BAD_PORT,              "bad port number"                       },
    {CTORM_ERR_BAD_PATH,              "invalid path"                          },
    {CTORM_ERR_BAD_QUERY,             "invalid query"                         },

    {CTORM_ERR_BAD_APP_PTR,           "invalid app pointer"                   },
    {CTORM_ERR_BAD_ADDR_PTR,          "invalid address pointer"               },
    {CTORM_ERR_BAD_JSON_PTR,          "invalid cJSON pointer"                 },
    {CTORM_ERR_BAD_FMT_PTR,           "invalid string format pointer"         },
    {CTORM_ERR_BAD_DATA_PTR,          "invalid data pointer"                  },
    {CTORM_ERR_BAD_LOCAL_PTR,         "invalid local name pointer"            },
    {CTORM_ERR_BAD_PARAM_PTR,         "invalid route parameter name pointer"  },
    {CTORM_ERR_BAD_HEADER_PTR,        "invalid header name/value pointer"     },
    {CTORM_ERR_BAD_QUERY_PTR,         "invalid URI query name pointer"        },
    {CTORM_ERR_BAD_URI_PTR,           "invalid URI pointer"                   },
    {CTORM_ERR_BAD_AUTHORITY_PTR,     "invalid URI authority pointer"         },
    {CTORM_ERR_BAD_HOST_PTR,          "invalid host pointer"                  },
    {CTORM_ERR_BAD_PATH_PTR,          "invalid path pointer"                  },

    {CTORM_ERR_SCHEME_TOO_LARGE,      "URI scheme is too large"               },
    {CTORM_ERR_USERINFO_TOO_LARGE,    "URI userinfo is too large"             },
    {CTORM_ERR_PORT_TOO_LARGE,        "port number is too large"              },
    {CTORM_ERR_HOST_TOO_LARGE,        "host is too large"                     },
    {CTORM_ERR_PATH_TOO_LARGE,        "path is too large"                     },
    {CTORM_ERR_QUERY_TOO_LARGE,       "URI query is too large"                },
    {CTORM_ERR_QUERY_KEY_TOO_LARGE,   "URI query key is too large"            },
    {CTORM_ERR_QUERY_VALUE_TOO_LARGE, "URI query value is too large"          },

    {CTORM_ERR_POOL_FAIL,             "failed to create thread pool"          },
    {CTORM_ERR_LISTEN_FAIL,           "failed to listen on the interface"     },
    {CTORM_ERR_SOCKET_OPT_FAIL,       "failed to set socket options"          },
    {CTORM_ERR_FCNTL_FAIL,            "failed to modify socket with fcntl"    },
    {CTORM_ERR_ALLOC_FAIL,            "memory allocation failed"              },
    {CTORM_ERR_SEEK_FAIL,             "file seek failed"                      },
    {CTORM_ERR_READ_FAIL,             "failed to read the file"               },
    {CTORM_ERR_MUTEX_FAIL,            "failed to initialize thread mutex"     },
    {CTORM_ERR_JSON_FAIL,             "cJSON failed, use cJSON_GetErrorPtr()" },
    {CTORM_ERR_RESOLVE_FAIL,          "failed to resolve the address"         },
    {CTORM_ERR_SOCKET_FAIL,           "failed to create socket"               },
    {CTORM_ERR_BIND_FAIL,             "failed to bind the socket"             },

    {CTORM_ERR_NOT_EXISTS,            "file does not exist"                   },
    {CTORM_ERR_NO_READ_PERM,          "missing read permission"               },
    {CTORM_ERR_NO_JSON_SUPPORT,       "library not compiled with JSON support"},
    {CTORM_ERR_EMPTY_BODY,            "body is empty"                         },
    {CTORM_ERR_EMPTY_QUERY,           "query does not contain any values"     },

    {CTORM_ERR_UNKNOWN,               "unknown error"                         },
    {0,                               NULL                                    }
};

const char *ctorm_error_str(int code) {
  struct ctorm_error_desc *desc = &_ctorm_err_descs[0];

  if (code == 0)
    return "no error";

  for (; desc->str != NULL; desc++)
    if (desc->code == code)
      return desc->str;

  return strerror(code);
}

const char *ctorm_error_details(ctorm_app_t *app) {
  return app->error == 0 ? "no details" : ctorm_error_str(app->error);
}

void ctorm_error_set(ctorm_app_t *app, int error) {
  // save the current errno
  pthread_mutex_lock(&app->mod_mutex);
  app->error = errno;
  pthread_mutex_unlock(&app->mod_mutex);

  // set the new errno
  errno = error;
}

void ctorm_error_clear(ctorm_app_t *app) {
  // clear the saved errno
  pthread_mutex_lock(&app->mod_mutex);
  app->error = 0;
  pthread_mutex_unlock(&app->mod_mutex);

  // clear the current errno
  errno = 0;
}
