#include "../include/util.h"
#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>

char* readall(char* path, int* sz){
 if(!file_canread(path))
    return NULL;

  int fd = open(path, O_RDONLY);
  *sz = lseek(fd, 0, SEEK_END);
  lseek(fd, 0, SEEK_SET);

  char* data = malloc(*sz+1);
  read(fd, data, *sz);
  data[*sz] = '\0';
  close(fd);

  return data;
}

// https://gist.github.com/dhess/975639/bb91cd552c0a92306b8ef49b417c6796f67036ce
char* strrep(char *s1, char *s2, char *s3){
  if (!s1 || !s2 || !s3)
    return 0;

  size_t s1_len = strlen(s1);
  if (!s1_len)
    return (char *)s1;
  size_t s2_len = strlen(s2);
  if (!s2_len)
    return (char *)s1;

  size_t count = 0;
  const char *p = s1;
  assert(s2_len);
  do {
    p = strstr(p, s2);
    if (p) {
      p += s2_len;
      ++count;
    } 
  } while (p);

  if (!count)
    return (char *)s1;

  assert(s1_len >= count * s2_len);
  assert(count);
  size_t s1_without_s2_len = s1_len - count * s2_len;
  size_t s3_len = strlen(s3);
  size_t s1_with_s3_len = s1_without_s2_len + count * s3_len;
  if (s3_len &&
      ((s1_with_s3_len <= s1_without_s2_len) || (s1_with_s3_len + 1 == 0)))
    return 0;
    
  char *s1_with_s3 = (char *)malloc(s1_with_s3_len + 1); 
  if (!s1_with_s3)
    return 0;
    
  char *dst = s1_with_s3;
  const char *start_substr = s1;
  size_t i;
  for(i = 0; i != count; ++i) {
    const char *end_substr = strstr(start_substr, s2);
    assert(end_substr);
    size_t substr_len = end_substr - start_substr;
    memcpy(dst, start_substr, substr_len);
    dst += substr_len;
    memcpy(dst, s3, s3_len);
    dst += s3_len;
    start_substr = end_substr + s2_len;
  }

  size_t remains = s1_len - (start_substr - s1) + 1;
  assert(dst + remains == s1_with_s3 + s1_with_s3_len + 1);
  memcpy(dst, start_substr, remains);
  assert(strlen(s1_with_s3) == s1_with_s3_len);
  return s1_with_s3;
}
 
bool file_canread(char* path){
  if(access(path, O_RDONLY)<0)
    return false;
  return true;
}

int digits(int n){
  if (n<0) return digits ((n== INT_MIN) ? INT_MAX: -n);
  if (n<10) return 1;
  return digits(n/10)+1;
}

char* join(char* p1, char* p2){
  char* fp = malloc(strlen(p1)+strlen(p2)+1);
  sprintf(fp, "%s/%s", p1, p2);
  return fp;
}

int stolower(char* src, char* dst) {
  for(int i = 0; src[i]; i++)
    dst[i] = tolower(src[i]);
  return 0;
}

bool contains(char* str, char c){
  for(int i = 0; i<strlen(str); i++)
    if(str[i] == c)
      return true;
  return false;
}

bool validate(char* str, char* valids, char end) {
  size_t validl = strlen(valids);
  size_t strl = strlen(str);

  for(int c = 0; c < strl; c++){
    bool pass = false;
    for(int v = 0; v < validl; v++){
      if(str[c]==valids[v]){
        pass = true;
        break;
      }
    }

    if( end != 0 && c == strl-1 
        && str[c] == end)
      pass = true;

    if(!pass)
      return false;
  }
  return true;
}

void urldecode(char *str){
  char * curr_ptr = str;
  char * step_ptr = str;
  unsigned char value;

  while (*curr_ptr){
    if (*curr_ptr == '+')
      *step_ptr = ' ';
    else if (*curr_ptr == '%') {
      sscanf(curr_ptr + 1, "%02hhx", &value);
      *step_ptr = value;
            curr_ptr = curr_ptr + 2;
    } else
      *step_ptr = *curr_ptr;
    curr_ptr++;
    step_ptr++;
  }
  
  *(step_ptr) = '\0';
}


char* replace(char* s, char c, char n){
  char* copy = strdup(s);
  for(char i = 0; i < strlen(copy); i++){
    if(copy[i] == c)
      copy[i] = n;
  }
  return copy;
}
