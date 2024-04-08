#include "ctorm.h"
#include "req.h"
#define BUF_MAX 8192

typedef enum parse_res {
  P_CONT = 0,
  P_FAIL = 1,
  P_OK = 2,
} parse_res;

typedef enum handle_st {
  H_CONFAIL = 0,
  H_BADREQ = 1,
  H_OK = 2,
} handle_st;

enum parse_st {
  METHOD = 0,
  SPACE = 1,
  PATH = 2,
  VERSION = 3,
  NEWLINE = 4,
  NAME = 5,
  VALUE = 6,
  BODY = 7,
};

handle_st handle_test(req_t *, int);
handle_st handle_request(req_t *, int);

parse_res parse_method(req_t *, int, char *);
parse_res parse_path(req_t *, int, char *);
parse_res parse_urldata(table_t *, char *, int);
parse_res parse_version(req_t *, int, char *);

parse_res parse_header_name(req_t *, int, char *);
parse_res parse_header_value(req_t *, int, char *);

parse_res parse_body(req_t *, int, char *);
