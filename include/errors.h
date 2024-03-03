#pragma once

enum error_t {
  EventFailed  = 9910,
  ListenFailed = 9912,
  BadAddress   = 9913,
  BadPort      = 9914,
  OptFailed    = 9915, 
  AllocFailed  = 9917,
  UnknownErr   = 9918,
};

struct error_desc_t {
  int code;
  char* desc;
};

char* geterror_code(int);
char* geterror();
