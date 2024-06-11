#include "../include/parse.h"
#include "../include/ctorm.h"
#include "../include/log.h"
#include "../include/req.h"
#include "../include/socket.h"
#include "../include/util.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

char valid_header[] = "_ :;.,\\/\"'?!(){}[]@<>=-+*#$&`|~^%";
char valid_path[]   = "-._~:/?#[]@!$&'()*+,;%=";

bool parse_form(table_t *table, char *data) {
  if(NULL == data || NULL == table)
    return false;

  size_t size = strlen(data), index = 0;
  char key[size+1], value[size+1];
  bool iskey = true;

  if(size <= 0)
    return false;

  bzero(key, size+1);
  bzero(value, size+1);

  for (char *c = data; *c != 0; c++) {
    if (*c == '=' && iskey) {
      iskey = false;
      index = 0;
      continue;
    }

    if (!iskey && *c == '&') {
      iskey = true;
      index = 0;

      urldecode(key);
      urldecode(value);
      
      table_add(table, strdup(key), true);
      table_set(table, strdup(value));

      continue;
    }

    if(iskey){
      key[index] = *c;
      key[index+1] = 0;
    }else {
      value[index] = *c;
      value[index+1] = 0;
    }

    index++;
  }

  if(iskey)
    return true;
  
  urldecode(key);
  urldecode(value);
      
  table_add(table, strdup(key), true);
  table_set(table, strdup(value));

  return true;
}

parse_ret_t parse_request(req_t *req, int socket) {
  // we extend the buffer 200 bytes everytime
  // this is to prevent using realloc as much as possible
  char        state = STATE_METHOD_0, temp = 0;
  size_t      size = 200, index = 0;
  ssize_t     read   = -1;
  char       *buffer = malloc(size);
  parse_ret_t ret    = RET_BADREQ;

  // one byte at a time...
  while ((read = recv(socket, buffer + index, 1, MSG_WAITALL)) > 0) {

    switch (state) {
    case STATE_METHOD_0:
      // if method is larger than the max method length
      // the its an invalid request
      if (index > http_static.method_max)
        goto end;

      // check if char is a valid method char
      if (!is_letter(buffer[index]) && !is_digit(buffer[index]) && buffer[index] != ' ')
        goto end;

      // if the current char is ' '
      // we just finished reading the HTTP method
      if (buffer[index] != ' ')
        break;

      // if the first char is ' ' then its a bad
      // invalid request
      if (0 == index)
        goto end;

      // we need the ID, so change the space with
      // null terminator
      buffer[index] = 0;
      req->method   = http_method_id(buffer);

      // if the method is invalid then its a bad
      // request, we should return
      if (-1 == req->method)
        goto end;

      debug("Method: %s", buffer);

      // move on to next state
      goto next;
      break;

    case STATE_PATH_1:
      // if buffer is larger than the maximum path
      // limit, then its a bad request
      if (index > http_static.path_max)
        goto end;

      // check if char is a valid path char
      if (!contains(valid_path, buffer[index]) && !is_letter(buffer[index]) && !is_digit(buffer[index]) &&
          buffer[index] != ' ')
        goto end;

      // check if we are end of the path
      if (buffer[index] != ' ')
        break;

      // if the first char is ' ' then its a bad
      // invalid request
      if (0 == index)
        goto end;

      // if so, replace the ' ' with null
      // terminator for string operation
      buffer[index] = 0;

      // allocate enough space for the path, +1 for null terminator
      req->encpath = malloc(index);
      bzero(req->encpath, index);

      // copy the path to allocated memory
      memcpy(req->encpath, buffer, index);
      debug("Path: %s", req->encpath);

      // move on to next state
      goto next;
      break;

    case STATE_VERSION_2:
      // if buffer is larger than the HTTP version
      // length, then its a bad request
      if (index > http_static.version_len)
        goto end;

      // check if the char is a valid
      if (!is_letter(buffer[index]) && !is_digit(buffer[index]) && buffer[index] != '/' && buffer[index] != '.' &&
          buffer[index] != '\r' && buffer[index] != '\n')
        goto end;

      // check if we are at the end of the version
      if (buffer[index] != '\r' && buffer[index] != '\n')
        break;

      // if its the first char, then the length is 0
      // and its a bad request
      if (0 == index)
        goto end;

      // replace the '\r' or '\n' with null terminator
      // we will restore it later
      temp = buffer[index];
      buffer[index] = 0;

      // get the static pointer for the HTTP version
      req->version = http_version_get(buffer);
      if (NULL == req->version)
        goto end;

      debug("Version: %s", req->version);

      // move on to next state, without resetting
      // the buffer, see the next section
      buffer[index] = temp;
      state++;
      break;

    case STATE_NEWLINE_3:
      /*
       * we have few cases to handle here
       *
       * first option:
       * =============================
       * GET / HTTP/1.1\r\n
       *                  ^
       * =============================
       *
       * second option:
       * =============================
       * GET / HTTP/1.1\n
       * \n
       *  ^
       * =============================
       *
       * third option:
       * =============================
       * GET / HTTP/1.1\n
       * User-Agent: curl
       * ^
       * =============================
       *
       */

      // if its the first option, just move on

      // no out-of-bounds here, we checked the index
      // in the previous section
      if ('\n' == buffer[index] && '\r' == buffer[index - 1])
        goto next;

      // if its the second option, we just read the request
      // and zero headers, so we dont have a body, we are done
      if ('\n' == buffer[index] && '\n' == buffer[index - 1]) {
        ret = RET_OK;
        goto end;
      }

      // if its the third option, clear the buffer and
      // restore the first char of the header
      temp = buffer[index];

      bzero(buffer, size);
      index = 0;
      state++;

      buffer[index] = temp;
      break;

    case STATE_NAME_4:
      // if instead of the header we got a newline, skip
      // the value state and move on to the body
      if (index == 0 && ('\r' == buffer[index] || '\n' == buffer[index])) {
        buffer[index] = 0;
        state         = STATE_BODY_7;
        index--;

        break;
      }

      // if the buffer is larger then the maximum header
      // name length, then the request is too large
      if (index > http_static.header_max) {
        ret = RET_TOOLARGE;
        goto end;
      }

      // if the header name contains an invalid char, then
      // return bad request
      if (!contains(valid_header, buffer[index]) && !is_digit(buffer[index]) && !is_letter(buffer[index]))
        goto end;

      // if the the char is ' ', then we probably just read
      // the header name
      if (buffer[index] != ' ')
        break;

      // if we are at the start or the previous char is not ':'
      // then its a bad header name, so return bad request
      if (0 == index || buffer[index - 1] != ':')
        goto end;

      // otherwise add the header to the request and move on
      buffer[index - 1] = 0;
      req_add_header(req, buffer);

      debug("Header: %s", buffer);

      goto next;
      break;

    case STATE_VALUE_5:
      // if the buffer is larger then the maximum header
      // value length, then the request is too large
      if (index > http_static.header_max) {
        ret = RET_TOOLARGE;
        goto end;
      }

      // if the header value contains an invalid char, then
      // return bad request
      if (!contains(valid_header, buffer[index]) && !is_digit(buffer[index]) && !is_letter(buffer[index]) &&
          buffer[index] != '\r' && buffer[index] != '\n')
        goto end;

      // if the the char is '\r' or '\n' we are done reading
      // the header value
      if (buffer[index] != '\r' && buffer[index] != '\n')
        break;

      // if we are at the start, then its a bad request
      // yes, empty header value is not allowed
      if (0 == index)
        goto end;

      // otherwise set the header value and move on
      // this time we will need to restore the char
      char prev     = buffer[index];
      buffer[index] = 0;

      req_add_header_value(req, buffer);
      debug("Value: %s", buffer);

      buffer[index] = prev;

      // why not goto next? well see the next sectipn
      state++;
      break;

    case STATE_NEWLINE_6:
      /*
       * kinda complicated, there are two options:
       * =====================================
       * User-Agent: curl\r\n
       *                    ^
       * =====================================
       *
       * second option is:
       * =====================================
       * User-Agent: curl\n
       * \n
       *  ^
       * =====================================
       *
       * third option is:
       * =====================================
       * User-Agent: curl\n
       * Host: whateverig
       * ^
       * =====================================
       *
       * this is why we didn't clear the buffer
       * after reading the value
       *
       */

      // if its the first option, go back to reading the
      // header name
      if (buffer[index] == '\n' && buffer[index - 1] == '\r') {
        state = STATE_NAME_4;
        goto reset;
        break;
      }

      // if its the second option move on to the body

      // we can't go out of bounds, index was checked
      // before this section
      if (buffer[index] == '\n' && buffer[index - 1] == '\n')
        goto next;

      // if its the third option, go back to reading the
      // header, but first clear the body and restore the char
      temp = buffer[index];

      bzero(buffer, size);
      index = 0;
      state = STATE_NAME_4;

      buffer[index] = temp;
      break;

    case STATE_BODY_7:
      ret = RET_OK;

      // can the request method have a body?
      if(!http_method_has_body(req->method))
        goto end;

      // do we have the content length header?
      char *contentlen = req_header(req, "content-length");
      if(NULL == contentlen)
        goto end;

      // if so, then parse the header value 
      req->bodysize = atol(contentlen);
      if(size <= 0)
        goto end;

      // make sure the body is not too large 
      if(req->bodysize > http_static.body_max){
        ret = RET_TOOLARGE;
        goto end;
      }

      // allocate and receive body
      req->body   = malloc(req->bodysize);
      if (recv(socket, req->body, req->bodysize, MSG_WAITALL) > 0)
        goto end;
     
      // connection failed? cleanup and return
      free(req->body);
      req->bodysize = 0;
      req->body = NULL;
      ret = RET_CONFAIL;
    
      goto end;
      break;

    default:
      debug("Unknown section");
      ret = RET_BADREQ;
      goto end;
    }

    // move to the next char
    index++;

    // realloc if buffer is full
    if (index >= size) {
      size += size;
      buffer = realloc(buffer, size);
    }

    continue;

  next:
    // clear buffer, reset index, and move
    // on to the next state
    state++;
  reset:
    bzero(buffer, size);
    index = 0;
  }

  if (read <= 0)
    ret = RET_CONFAIL;

end:
  if (RET_OK != ret)
    goto ret;

  char *save = NULL, *rest = NULL, *dup = NULL; 

  if(!contains(req->encpath, '?')){
    req->path = req->encpath;
    goto ret;
  }

  // read the first part of the path
  dup = strdup(req->encpath);
  req->path = strtok_r(dup, "?", &save);

  if(NULL == req->path){
    req->path = req->encpath;
    free(dup);
    goto ret;
  }

  rest = strtok_r(NULL, "?", &save);
  if(NULL == rest){
    req->path = strdup(req->path);
    free(dup);
    goto ret;
  }

  parse_form(&req->query, rest);
  req->path = strdup(req->path);
  free(dup);

ret:
  // free the buffer and return the result
  free(buffer);
  return ret;
}
