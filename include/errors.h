#pragma once

typedef enum app_error_t {
  BadTcpTimeout     = 9908,
  BadPoolSize       = 9909,
  EventFailed       = 9910,
  PoolFailed        = 9911,
  ListenFailed      = 9912,
  BadAddress        = 9913,
  BadPort           = 9914,
  OptFailed         = 9915,
  AllocFailed       = 9917,
  UnknownErr        = 9918,
  CantRead          = 9919,
  SizeFail          = 9920,
  BadReadPerm       = 9921,
  FileNotExists     = 9922,
  BadPath           = 9923,
  InvalidAppPointer = 9924,
  BadUrlPointer     = 9925,
  BadJsonPointer    = 9926,
  BadFmtPointer     = 9927,
  BadPathPointer    = 9928,
  BadDataPointer    = 9929,
  BadHeaderPointer  = 9930,
  MutexFail         = 9931,
} app_error_t;

struct app_error_desc_t {
  int   code;
  char *desc;
};

char *app_geterror_code(app_error_t);
char *app_geterror();
