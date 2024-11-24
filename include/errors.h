#pragma once

typedef enum app_error_t {
  BadTcpTimeout     = 9908,
  BadPoolSize       = 9909,
  PoolFailed        = 9910,
  ListenFailed      = 9911,
  BadAddress        = 9912,
  BadPort           = 9913,
  OptFailed         = 9914,
  AllocFailed       = 9915,
  UnknownErr        = 9916,
  CantRead          = 9917,
  SizeFail          = 9918,
  BadReadPerm       = 9919,
  FileNotExists     = 9920,
  BadPath           = 9921,
  InvalidAppPointer = 9922,
  BadUrlPointer     = 9923,
  BadJsonPointer    = 9924,
  BadFmtPointer     = 9925,
  BadPathPointer    = 9926,
  BadDataPointer    = 9927,
  BadHeaderPointer  = 9928,
  BadMaxConnCount   = 9929,
  NoJSONSupport     = 9930,
} app_error_t;

struct app_error_desc_t {
  int   code;
  char *desc;
};

char *app_geterror_code(app_error_t);
char *app_geterror();
