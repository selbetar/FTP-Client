#ifndef __FTCLIENT_H__
#define __FTCLIENT_H__

#define DEFAULT_PORT "21"
#define MAX_LINE 256

typedef struct pasv_response {
    
} pasvr_t;

int read_response (char *buf);
int send_msg(const char *msg);
int executeCommand(const char *command, const char *parameter);

#endif