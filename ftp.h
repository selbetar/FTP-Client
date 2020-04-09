#ifndef __FTCLIENT_H__
#define __FTCLIENT_H__

#define DEFAULT_PORT "21"
#define MAX_LINE 256
#define CRLF "\r\n"

typedef struct pasv_response {

} pasvr_t;
int execute_command (const char *command, const char *parameter);
int user_cmd(const char* parameter);
int pass_cmd(const char* parameter);
int get_cmd(const char* file);
int feat_cmd();
int cd_cmd(const char* path);
int pwd_cmd();
int pasv_cmd();


int read_response (char *buf);
int send_msg (const char *msg);

int check_argc (const char *args, unsigned int exepcted);
#endif