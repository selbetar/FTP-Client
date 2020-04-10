#ifndef __FTCLIENT_H__
#define __FTCLIENT_H__

#include <fcntl.h>
#include <sys/stat.h>

#define DEFAULT_PORT "21"
#define MAX_LINE     256
#define CRLF         "\r\n"

int execute_command (const char *command, const char *parameter);
int one_cmd (const char *parameter);
int two_cmd (const char *cmd, const char *parameter);
int quit_cmd();
int feat_cmd();
int pasv_cmd();
int nlst_cmd (const char *path);

int get_cmd (const char *file);

int read_response (char *buf);
int send_msg (const char *msg);

int check_argc (const char *args, unsigned int exepcted);
void cleanup();
#endif