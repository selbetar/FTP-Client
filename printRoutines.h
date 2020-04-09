
#ifndef __PRINTROUTINES_H__
#define __PRINTROUTINES_H__

#include <stdio.h>

int printErrorInvalidCommand (FILE *file, const char *command);
int printErrorInvalidParameter (FILE *file, const char *parameter);

int printResponse (FILE *file, const char *response);

int printSent (FILE *file, const char *msg);

#endif