#include "ctorm.h"
#include "req.h"
#define BUF_MAX 8192

typedef enum parse_ret {
  RET_CONFAIL  = 0,
  RET_TOOLARGE = 1,
  RET_BADREQ   = 2,
  RET_OK       = 3,
} parse_ret_t;

typedef enum parse_state {
  STATE_METHOD_0  = 0, // "GET"
  STATE_PATH_1    = 1, // "/example"
  STATE_VERSION_2 = 2, // "HTTP/1.1"
  STATE_NEWLINE_3 = 3, // "\r\n"
  STATE_NAME_4    = 4, // "User-Agent"
  STATE_VALUE_5   = 5, // "curl/8.8.0"
  STATE_NEWLINE_6 = 6, // "\r\n"
  STATE_BODY_7    = 7,
} parse_state_t;

bool parse_form(table_t *, char *);
parse_ret_t parse_request(req_t *, int);
