#pragma once
#include <stdbool.h>

#define eq(s1, s2) strcmp(s1, s2)==0
#define neq(s1, s2) strcmp(s1, s2)!=0
#define startswith(str, pre) (strlen(pre)<=strlen(str) && strncmp(pre, str, strlen(pre))==0)
#define endswith(str, suf) (strlen(suf)<=strlen(str) && strncmp(str+strlen(str)-strlen(suf), suf, strlen(suf))==0)

char* readall(char*, int*);
char* strrep(char*, char*, char*);
void urldecode(char*);
int stolower(char*, char*);
bool file_canread(char*);
char* replace(char*, char, char);
char* join(char*, char*);
bool contains(char*, char);
bool validate(char*, char*, char);
int digits(int);
