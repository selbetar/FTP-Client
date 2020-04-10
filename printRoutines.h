
#ifndef __PRINTROUTINES_H__
#define __PRINTROUTINES_H__

#include <stdio.h>

int printHelp (FILE *file);

int printErrorInvalidCommand (FILE *file, const char *command);
int printErrorInvalidParameter (FILE *file, const char *parameter);
int printErrorCommmandTooLong (FILE *file);

int printResponse (FILE *file, const char *response);
int printSent (FILE *file, const char *msg);

void printDebug (char *msg);
#endif