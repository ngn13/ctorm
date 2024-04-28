#include "ctorm.h"
#include "req.h"
#define BUF_MAX 8192

typedef enum parse_res {
  RES_CONT = 0,
  RES_FAIL = 1,
  RES_OK   = 2,
} parse_res;

typedef enum parse_ret {
  RET_CONFAIL = 0,
  RET_BADREQ = 1,
  RET_OK = 2,
} parse_ret;

enum parse_state {
  METHOD = 0,
  SPACE = 1,
  PATH = 2,
  VERSION = 3,
  NEWLINE = 4,
  NAME = 5,
  VALUE = 6,
  BODY = 7,
};

parse_ret parse_request(req_t *, int);

parse_res parse_method(req_t *, int, char *);
parse_res parse_path(req_t *, int, char *);
parse_res parse_urldata(table_t *, char *, int);
parse_res parse_version(req_t *, int, char *);

parse_res parse_header_name(req_t *, int, char *);
parse_res parse_header_value(req_t *, int, char *);

parse_res parse_body(req_t *, int, char *);
