#include "../include/parse.h"
#include "../include/ctorm.h"
#include "../include/log.h"
#include "../include/req.h"
#include "../include/socket.h"
#include "../include/util.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

char value_valid[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234"
                     "56789_ :;.,\\/\"'?!(){}[]@<>=-+*#$&`|~^%";
char url_valid[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456"
                   "789-._~:/?#[]@!$&'()*+,;%=";
char name_valid[] =
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_";
char *versions[] = {"HTTP/1.1", "HTTP/1.0"};

enum parse_state parse_order[] = {
    METHOD, SPACE, PATH, VERSION, NEWLINE, NAME, VALUE, NEWLINE, BODY,
};

parse_ret parse_request(req_t *req, int s) {
  size_t i = 0, indx = 0,
         parse_max = sizeof(parse_order) / sizeof(enum parse_state) - 1;
  enum parse_state *order = parse_order;
  parse_res r = RES_FAIL;
  char cur[1] = "0";
  bool done = false;
  char buf[BUF_MAX + 1];

  while (recv(s, cur, 1, MSG_WAITALL)) {
    if (BODY != order[i] && cur[0] == '\r')
      continue;

    if (BODY != order[i] && cur[0] == '\0')
      return RET_BADREQ;

    if (indx >= BUF_MAX)
      return RET_BADREQ;

    buf[indx] = cur[0];
    buf[indx + 1] = '\0';

    switch (order[i]) {
    case METHOD:
      r = parse_method(req, indx, buf);
      break;
    case SPACE:
      r = RES_FAIL;
      if (eq(buf, " "))
        r = RES_OK;
      break;
    case PATH:
      r = parse_path(req, indx, buf);
      break;
    case VERSION:
      r = parse_version(req, indx, buf);
      break;
    case NEWLINE:
      r = RES_FAIL;
      if (eq(buf, "\n")) {
        r = RES_OK;
        break;
      }

      if (i - 1 == VALUE) {
        r = RES_CONT;
        i -= 2;
      }
      break;
    case NAME:
      if (eq(buf, "\n")) {
        i = parse_max + 1;
        break;
      }
      r = parse_header_name(req, indx, buf);
      break;
    case VALUE:
      r = parse_header_value(req, indx, buf);
      break;
    case BODY:
      r = parse_body(req, indx, buf);
      break;
    default:
      break;
    }

    if (RES_FAIL == r)
      return RET_BADREQ;

    if (RES_OK == r) {
      debug("(Socket %d) Parsing the next section (%d/%d)", s, i, parse_max);
      r = RES_FAIL;
      indx = 0;
      i++;

      if (order[i] == BODY) {
        int size = req_body_size(req);
        if (size == 0 || !req_can_have_body(req)) {
          done = true;
          break;
        }
      }

      if (i > parse_max) {
        done = true;
        break;
      }

      continue;
    }

    indx++;
  }

  if (!done) {
    debug("(Socket %d) %s, most likely the connection died", s,
          strerror(errno));
    return RET_CONFAIL;
  }

  debug("Decoding URL");
  char *save, *pathdup = strdup(req->fullpath);
  req->encpath = strtok_r(pathdup, "?", &save);
  if (NULL == req->encpath) {
    req->encpath = strdup(req->fullpath);
  } else {
    req->encpath = strdup(req->encpath);
  }

  char *rest = strtok_r(NULL, "?", &save);
  if (NULL != rest) {
    parse_urldata(&req->query, rest, strlen(rest));
  }

  free(pathdup);
  req->path = strdup(req->encpath);
  urldecode(req->path);
  return RET_OK;
}

parse_res parse_urldata(table_t *data, char *rest, int size) {
  char key[size], value[size];
  int indx = 0, iskey = 1;
  int restsize = strlen(rest);

  for (int c = 0; c < restsize; c++) {
    if (rest[c] == '=' && iskey) {
      iskey = 0;
      indx = 0;
      continue;
    }

    if (!iskey && rest[c] == '&') {
      iskey = 1;
      indx = 0;
      urldecode(key);
      urldecode(value);

      table_add(data, strdup(key), true);
      table_set(data, strdup(value));
      continue;
    }

    if (iskey) {
      key[indx] = rest[c];
      key[indx + 1] = '\0';
    } else {
      value[indx] = rest[c];
      value[indx + 1] = '\0';
    }
    indx++;

    if (c == restsize - 1 && !iskey) {
      urldecode(key);
      urldecode(value);

      table_add(data, strdup(key), true);
      table_set(data, strdup(value));
    }
  }

  return RES_OK;
}

parse_res parse_body(req_t *req, int i, char *buf) {
  int size = req_body_size(req);
  if (i + 1 != size)
    return RES_CONT;

  req->body = malloc(i + 1);
  memcpy(req->body, buf, i + 1);
  req->bodysz = i + 1;
  return RES_OK;
}

parse_res parse_header_value(req_t *req, int i, char *buf) {
  if (buf[i] != '\n')
    return RES_CONT;

  if (!validate(buf, value_valid, '\n'))
    return RES_FAIL;

  char val[i + 1];
  for (int c = 0; c < i + 1; c++) {
    val[c] = buf[c];
  }
  val[i] = '\0';
  req_add_header_value(req, val);
  return RES_OK;
}

parse_res parse_header_name(req_t *req, int i, char *buf) {
  size_t namel = strlen(url_valid);

  if (i + 1 < 3)
    return RES_CONT;

  for (int c = 0; c < i + 1; c++) {
    int valid = 0;
    for (int v = 0; v < namel; v++) {
      if (buf[c] == name_valid[v]) {
        valid = 1;
        break;
      }
    }

    if (buf[c] == ':' && i == c)
      valid = 1;

    if (buf[c] == ':' && (i - 1) == c && buf[c + 1] == ' ')
      valid = 1;

    if (c >= 3 && buf[c] == ' ' && buf[c - 1] == ':' && i == c)
      valid = 1;

    if (!valid)
      return RES_FAIL;
  }

  if (buf[i] != ' ' || buf[i - 1] != ':')
    return RES_CONT;

  char name[i];
  for (int c = 0; c < i; c++)
    name[c] = buf[c];
  name[i - 1] = '\0';

  req_add_header(req, name);
  return RES_OK;
}

parse_res parse_version(req_t *req, int i, char *buf) {
  for (int v = 0; v < sizeof(versions) / sizeof(char *); v++) {
    if (eq(versions[v], buf)) {
      req->version = versions[v];
      return RES_OK;
    }

    if (startswith(versions[v], buf))
      return RES_CONT;
  }
  return RES_FAIL;
}

parse_res parse_path(req_t *req, int i, char *buf) {
  if (i + 1 == 1)
    return RES_CONT;

  if (!validate(buf, url_valid, ' '))
    return RES_FAIL;

  if (buf[i] != ' ')
    return RES_CONT;

  req->fullpath = malloc(i + 1);
  for (int c = 0; c < i + 1; c++) {
    req->fullpath[c] = buf[c];
  }

  req->fullpath[i] = '\0';
  return RES_OK;
}

parse_res parse_method(req_t *req, int i, char *buf) {
  bool prefix = false;
  for (int s = 0; s < http_method_sz; s++) {
    if (eq(http_method_map[s].name, buf)) {
      req->method = http_method_map[s].code;
      return RES_OK;
    }

    if (startswith(http_method_map[s].name, buf)) {
      prefix = true;
      break;
    }
  }

  if (!prefix)
    return RES_FAIL;
  return RES_CONT;
}
