#include "ftp.h"
#include "common.h"
#include "network.h"
#include "printRoutines.h"

#include <strings.h>
unsigned int sfd; // Control Connection fd

int main (int argc, char *argv[])
{
    char line[MAX_LINE]; // stores terminal commands
    char *command, *parameter;
    int c;
    // 1 program
    // 2 server
    // 3 port
    if (argc < 2 || argc > 3) {
        fprintf (stderr, "Usage: %s <host> <port>\n", argv[0]);
        fprintf (stderr, "     <host>  The address of the ftp server.\n");
        fprintf (stderr, "     <port>  Optional:  Specifies the port the client should initiate connection on.\n");
        return -1;
    }

    const char *port = argc == 3 ? argv[2] : DEFAULT_PORT;

    sfd = create_connection (argv[1], port);

    if (sfd < 0)
        exit (EXIT_FAILURE);

    char buffer[BUF_SIZE];
    memset (buffer, 0, BUF_SIZE);

    if (read_response (buffer) <= 0)
        exit (EXIT_FAILURE);

    while (1) {
        // show prompt, but only if input comes from a terminal
        if (isatty (STDIN_FILENO))
            printf ("> ");

        if (!fgets (line, sizeof (line), stdin))
            break;

        if (!strchr (line, '\n')) {
            while ((c = fgetc (stdin)) != EOF && c != '\n')
                ;
            if (c == '\n') {
                //printErrorCommmandTooLong(stdout);
                fprintf (stdout, "%s", "ok");
                continue;
            }
        }

        command = strtok (line, " \t\n\f\r\v");
        if (!command)
            continue; // ignore blank lines;

        // get parameters if provided
        parameter = strtok (NULL, "\n\r");

        int scode = execute_command (command, parameter);

        if (scode < 0) {
            cleanup();
            break;
        }
    }
    return 0;
}

// returns -1 on error [connection need to be closed and client]
// returns -2 on conncetion closure [client exit with no error]
// returns 1 otherwise
int execute_command (const char *command, const char *parameter)
{
    if (strcasecmp (command, "quit") == 0) {
        if (check_argc (parameter, 0))
            return quit_cmd();
        return printErrorInvalidParameter (stdout, parameter);
    }
    else if (strcasecmp (command, "user") == 0) {
        if (check_argc (parameter, 1))
            return two_cmd ("user", parameter);
        return printErrorInvalidParameter (stdout, parameter);
    }
    else if (strcasecmp (command, "pass") == 0) {
        if (check_argc (parameter, 1))
            return two_cmd ("pass", parameter);
        return printErrorInvalidParameter (stdout, parameter);
    }
    else if (strcasecmp (command, "get") == 0) {
        if (check_argc (parameter, 1))
            return get_cmd (parameter);
        return printErrorInvalidParameter (stdout, parameter);
    }
    else if (strcasecmp (command, "features") == 0) {
        if (check_argc (parameter, 0))
            return one_cmd ("feat");
        return printErrorInvalidParameter (stdout, parameter);
    }
    else if (strcasecmp (command, "cd") == 0) {
        if (check_argc (parameter, 1))
            return two_cmd ("cwd", parameter);
        return printErrorInvalidParameter (stdout, parameter);
    }
    else if (strcasecmp (command, "nlist") == 0) {
        if (check_argc (parameter, 0) || check_argc (parameter, 1))
            return nlst_cmd (parameter);
        return printErrorInvalidParameter (stdout, parameter);
    }
    else if (strcasecmp (command, "pwd") == 0) {
        if (check_argc (parameter, 0))
            return one_cmd ("pwd");
        return printErrorInvalidParameter (stdout, parameter);
    }
    else if (strcasecmp (command, "put") == 0) {
        // todo
    }
    else if (strcasecmp (command, "delete") == 0) {
        // todo
    }
    else {
        return printErrorInvalidCommand (stdout, command);
    }
    return 0;
}

/**
 * RETURN:
 * length of server response to the msg
 * -1 on error
 * */
int one_cmd (const char *cmd)
{
    char buffer[BUF_SIZE];
    memset (buffer, 0, BUF_SIZE);
    snprintf (buffer, BUF_SIZE, "%s", cmd);

    ssize_t length = send_msg (buffer);

    if (length < 0)
        return -1;

    return read_response (buffer);
}

int two_cmd (const char *cmd, const char *param)
{
    char buffer[BUF_SIZE];
    memset (buffer, 0, BUF_SIZE);
    snprintf (buffer, BUF_SIZE, "%s%s%s", cmd, " ", param);

    ssize_t length = send_msg (buffer);

    if (length < 0)
        return -1;

    return read_response (buffer);
}

/**
 * RETURNS:
 * the socket_fd on which data connection
 * wil occur
 * -1 on error
 * 0 if 227 wasn't received
 * */
int pasv_cmd()
{
    int data_fd = -1;
    char buffer[BUF_SIZE];
    u_int32_t h1, h2, h3, h4, p1, p2;

    memset (buffer, 0, BUF_SIZE);
    snprintf (buffer, BUF_SIZE, "%s", "pasv");

    ssize_t length = send_msg (buffer);

    if (length < 0)
        return -1;

    if (read_response (buffer) < 0)
        return -1;

    if (strstr (buffer, "227") != buffer)
        return 0;

    length = sscanf (buffer, "227 Entering Passive Mode (%u,%u,%u,%u,%u,%u)",
                     &h1, &h2, &h3, &h4, &p1, &p2);
    if (length != 6) {
        fprintf (stderr, "PASV scan failure!\n");
        return -1;
    }

    struct sockaddr_in data_addr;
    memset (&data_addr, 0, sizeof (struct sockaddr_in));
    data_addr.sin_addr.s_addr = htonl ((h1 << 24) | ((h2 & 0xff) << 16) | ((h3 & 0xff) << 8) | (h4 & 0xff));
    data_addr.sin_family      = AF_INET;
    data_addr.sin_port        = htons (((p1 & 0xff) << 8) | (p2 & 0xff));

    data_fd = socket (PF_INET, SOCK_STREAM, 0);

    if (data_fd < 0) {
        perror ("Data Socket Creation Failed");
        return -1;
    }

    int scode = connect (data_fd, (const struct sockaddr *)&data_addr, sizeof (data_addr));
    if (scode < 0)
        return -1;
    return data_fd;
}

int nlst_cmd (const char *path)
{
    char buffer[BUF_SIZE];
    int rcode, data_fd;
    ssize_t length;

    data_fd = pasv_cmd();

    if (data_fd <= 0)
        return data_fd;

    memset (buffer, 0, BUF_SIZE);
    if (path == NULL) {
        length = snprintf (buffer, BUF_SIZE, "%s", "nlst");
    }
    else {
        length = snprintf (buffer, BUF_SIZE, "%s%s", "nlst ", path);
    }

    rcode = send_msg (buffer);
    if (rcode < 0)
        return -1;

    rcode = read_response (buffer);
    if (rcode < 0)
        return -1;

    if (strstr (buffer, "150 ") == buffer || strstr (buffer, "125 ") == buffer) {
        memset (buffer, 0, BUF_SIZE);
        while (read (data_fd, buffer, BUF_SIZE) > 0) {
            // todo change print format
            printResponse (stdout, buffer);
            memset (buffer, 0, BUF_SIZE);
        }
        rcode = read_response (buffer);
        if (rcode < 0)
            return -1;
    }

    close (data_fd);
    return 1;
}

int get_cmd (const char *file)
{
    char buffer[BUF_SIZE];
    int rcode, data_fd, file_fd;
    ssize_t length;

    // check if file exists to not overwrite it
    file_fd = open (file, O_WRONLY | O_CREAT | O_EXCL, 0777);
    if (file_fd == -1) {
        perror ("failed to create the file");
        fprintf (stderr, "Rename the file on disk first then call get again.\n");
        return 1;
    }

    // transfer in binary mode only
    rcode = one_cmd ("type I");
    if (rcode < 0)
        return -1;

    data_fd = pasv_cmd();

    if (data_fd <= 0)
        return data_fd;

    memset (buffer, 0, BUF_SIZE);
    snprintf (buffer, BUF_SIZE, "%s%s", "retr ", file);

    rcode = send_msg (buffer);
    if (rcode < 0)
        return -1;

    rcode = read_response (buffer);
    if (rcode < 0)
        return -1;

    if (strstr (buffer, "150 ") == buffer || strstr (buffer, "125 ") == buffer) {
        while ((length = read (data_fd, buffer, BUF_SIZE)) > 0) {
            rcode = write (file_fd, buffer, length);
            if (rcode < 0 || length < 0) {
                perror ("An Error Occuered during file transfer. Closing Client");
                close (data_fd);
                close (file_fd);
                return -1;
            }
            memset (buffer, 0, BUF_SIZE);
        }
        rcode = read_response (buffer);
        if (rcode < 0)
            return -1;
    }

    close (data_fd);
    close (file_fd);
    return 1;
}

int quit_cmd()
{
    char buffer[BUF_SIZE];
    memset (buffer, 0, BUF_SIZE);
    snprintf (buffer, BUF_SIZE, "%s", "quit");

    ssize_t length = send_msg (buffer);

    if (length < 0)
        return -1;

    read_response (buffer);
    cleanup();
    return -2;
}

/*
* returns -1 on error, number of bytes sent otherwise
*/
int send_msg (const char *msg)
{
    char buffer[BUF_SIZE];
    ssize_t length;

    memset (buffer, 0, 1024);
    length = snprintf (buffer, 1024, "%s%s", msg, CRLF);
    if (length != send (sfd, buffer, length, 0)) {
        perror ("Couldn't sent the complete msg. Exiting..");
        return -1;
    }

    return length;
}

int read_response (char *buffer)
{

    memset (buffer, 0, BUF_SIZE);
    ssize_t length = read (sfd, buffer, BUF_SIZE);
    if (length <= 0) {
        perror ("Error while reading from server. Terminiating Control Connection");
        return -1;
    }

    printResponse (stdout, buffer);

    uint32_t i_rcode;
    char buf[24];
    memset (buf, 0, 24);
    // check for multiline response: d{3}-
    if (sscanf (buffer, "%u%s", &i_rcode, buf) == 2 && strcmp (buf, "-") == 0) {
        memset (buf, 0, 24);
        snprintf (buf, 24, "%d", i_rcode);
        while (1) {
            memset (buffer, 0, BUF_SIZE);
            length = read (sfd, buffer, BUF_SIZE);
            printResponse (stdout, buffer);
            if (strstr (buffer, buf) == buffer)
                break;
        }
    }
    // copy the response to manipulate it
    // without modifing the original buffer
    char rcopy[length];
    memset (rcopy, 0, length);
    snprintf (rcopy, BUF_SIZE, "%s", buffer);

    char *rcode = strtok (rcopy, " ");
    if (strcmp (rcode, "421") == 0) {
        printf ("%s\n", "Server Closing Control Connection. Shutting Down.");
        return -1;
    }
    return length;
}

/**
 * Checks if the number of string seperated by spaces
 *  in args matches the expected number
 * 
 * RETURN:
 * returns 1 if argc == expected
 * returns 0 otherwise
 * */
int check_argc (const char *args, unsigned int exepcted)
{
    int count = 1;
    char argcpy[BUF_SIZE];

    if (args == NULL) {
        return exepcted == 0;
    }

    // make a copy of parameter for count checking
    memset (argcpy, 0, BUF_SIZE);
    snprintf (argcpy, BUF_SIZE, "%s", args);

    char *param = strtok (argcpy, " \t\n\f\r\v");
    while (param != NULL) {
        param = strtok (NULL, " \t\n\f\r\v");
        if (param != NULL)
            count++;
    }
    return count == exepcted;
}

void cleanup()
{
    if (sfd != -1) {
        close (sfd);
        sfd = -1;
    }
}
