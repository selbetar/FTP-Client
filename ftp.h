#ifndef __FTCLIENT_H__
#define __FTCLIENT_H__

#include <fcntl.h>
#include <unistd.h>

#define DEFAULT_PORT "21"
#define CRLF         "\r\n"
#define MAX_LINE     256
#define BUF_SIZE     1024

/**
 * Executes an ftp command
 * 
 * @param command  the command to be executed
 * @param paramter optional command parameter
 * @return  1 if execution was successful
 *          0 if command wasn't understood
 *         -1 on error, which causes the control connection
 *          and client to close
 *         -2 if quit was issued. Client exits normally
 **/
int execute_command (const char *command, const char *parameter);

/**
 * Login cmd which handles user and pass commands
 * It sets the ftp server to use binary mode
 * on successful login
 * @param cmd  the command to be executed
 * @param param command parameter
 * @return  1 if execution was successful
 *         -1 on error, which causes the control connection
 *          and client to close
 **/
int login_cmd (const char *cmd, const char *param, char *response);

/**
 * This command causes the client to exit.
 * @param response OPTIONAL server response will be stored here
 *                 if it is not set to NULL
 * @return  -2 if execution was successful
 *          -1 on error
 **/
int quit_cmd (char *response);

/**
 * The pasv commands which establishes a data connection
 * with the server
 * @param response OPTIONAL server response will be stored here
 *                 if it is not set to NULL
 * @return  the socket fd
 *          0 if the server did not response with 227
 *         -1 on error
 **/
int pasv_cmd (char *response);

/**
 * @param path     OPTIONAL the path
 * @param response OPTIONAL server response will be stored here
 *                 if it is not set to NULL
 * @return  1 if execution was successful
 *          0 if establishing pasv connection failed
 *         -1 on error
 **/
int nlst_cmd (const char *path, char *response);

/**
 *  RETR command
 * @param file     the file to be retrieved, shouldn't not contain a path [not supported]
 * @param response OPTIONAL server response will be stored here
 *                 if it is not set to NULL
 * @return  1 if execution was successful
 *          0 if establishing pasv connection failed
 *         -1 on error
 **/
int get_cmd (const char *file, char *response);

/**
 *  STOR command
 * @param file     the file to be sent to the server
 * @param response OPTIONAL server response will be stored here
 *                 if it is not set to NULL
 * @return  1 if execution was successful
 *          0 if establishing pasv connection failed
 *         -1 on error
 **/
int put_cmd (const char *file, char *response);

/**
 * Sends an arbitrary cmd to the ftp server
 * @param cmd  the command to be executed
 * @param param OPTIONAL the command parameter
 * @param response OPTIONAL server response will be stored here
 *                 if it is not set to NULL
 * @return  1 if execution was successful
 *         -1 on error, which causes the control connection
 *          and client to close
 **/
int rand_cmd (const char *cmd, const char *param, char *response);
/**
 *  Depends on response received by login_cmd()
 *  Sets the transfer type to binary on successful login
 * @param response OPTIONAL server response will be stored here
 *                 if it is not set to NULL
 * @return  1 if execution was successful
 *          -1 on error
 **/
int bin_type (const char *response);

/**
 *  Reads a response from the server and stores the read result in
 * the parameter response.
 * @param response server response will be stored here, this arg must be provided
 * @return  number of bytes read
 *          -1 on error
 **/
ssize_t read_response (char *response);

/**
 *  Sends a msg to the server
 * @param msg the msg to be sent
 * @return  number of bytes sent
 *          -1 on error
 **/
ssize_t send_msg (const char *msg);

/**
 * Checks if the number of string seperated by spaces
 *  in args matches the expected number
 * 
 * @return 1 if argc == expected
 *         0 otherwise
 **/
int check_argc (const char *args, unsigned int exepcted);
void cleanup();
#endif