#pragma once
#include <stdbool.h>
#include <string.h>

bool eq(char *, char*);
bool startswith(char *, char*);
bool endswith(char *, char*);

bool file_read(char *, char *, size_t);
bool file_canread(char *);
size_t file_size(char *);

void  urldecode(char *);
void stolower(char *, char *);
char *replace(char *, char, char);
char *join(char *, char *);
bool  contains(char *, char);
bool  validate(char *, char *, char);
int   digits(int);
bool  is_digit(char);
bool  is_letter(char);
