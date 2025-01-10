#pragma once
#include <stdint.h>

#ifndef CTORM_EXPORT

struct ctorm_error_desc {
  uint16_t    code;
  const char *desc;
};

#endif

typedef enum app_error {
  BadTcpTimeout       = 9908,
  BadPoolSize         = 9909,
  PoolFailed          = 9910,
  ListenFailed        = 9911,
  BadAddress          = 9912,
  BadPort             = 9913,
  OptFailed           = 9914,
  AllocFailed         = 9915,
  UnknownErr          = 9916,
  CantRead            = 9917,
  SizeFail            = 9918,
  BadReadPerm         = 9919,
  FileNotExists       = 9920,
  BadPath             = 9921,
  InvalidAppPointer   = 9922,
  BadUrlPointer       = 9923,
  BadJsonPointer      = 9924,
  BadFmtPointer       = 9925,
  BadPathPointer      = 9926,
  BadDataPointer      = 9927,
  BadHeaderPointer    = 9928,
  BadMaxConnCount     = 9929,
  NoJSONSupport       = 9930,
  MutexFail           = 9931,
  BadResponseCode     = 9932,
  ResponseAlreadySent = 9933,
  InvalidContentType  = 9934,
  EmptyBody           = 9935,
  BodyRecvFail        = 9936,
} ctorm_error_t;

const char *ctorm_geterror_code(ctorm_error_t error);
const char *ctorm_geterror();
