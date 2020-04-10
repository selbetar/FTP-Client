#ifndef __FTCLIENT_H__
#define __FTCLIENT_H__

#include <fcntl.h>
#include <unistd.h>

#define DEFAULT_PORT "21"
#define MAX_LINE     256
#define CRLF         "\r\n"

int execute_command (const char *command, const char *parameter);
int login_cmd (const char *cmd, const char *param, char *response);
int quit_cmd (char *response);
int pasv_cmd (char *response);
int nlst_cmd (const char *path, char *response);
int get_cmd (const char *file, char *response);
int put_cmd (const char *file, char *response);

int rand_cmd (const char *cmd, const char *param, char *response);
int bin_type (const char *response);

ssize_t read_response (char *response);
ssize_t send_msg (const char *msg);

int check_argc (const char *args, unsigned int exepcted);
void cleanup();
#endif