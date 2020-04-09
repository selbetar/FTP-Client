#include "ftp.h"
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

    char buffer[BUFSIZ];
    memset (buffer, 0, BUFSIZ);

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

        if (scode == -1) {
            // flush();
            break;
        }
    }
    return 0;
}

int execute_command (const char *command, const char *parameter)
{
    if (strcasecmp (command, "quit") == 0) {
        // todo
    }
    else if (strcasecmp (command, "user") == 0) {
        if (check_argc (parameter, 1))
            return user_cmd (parameter);
        return printErrorInvalidParameter (stdout, parameter);
    }
    else if (strcasecmp (command, "pass") == 0) {
        if (check_argc (parameter, 1))
            return pass_cmd (parameter);
        return printErrorInvalidParameter (stdout, parameter);
    }
    else if (strcasecmp (command, "get") == 0) {
        // todo
    }
    else if (strcasecmp (command, "features") == 0) {
        // todo
    }
    else if (strcasecmp (command, "cd") == 0) {
        // todo
    }
    else if (strcasecmp (command, "nlist") == 0) {
        // todo
    }
    else if (strcasecmp (command, "pwd") == 0) {
        // todo
    }
    else if (strcasecmp (command, "put") == 0) {
        // todo
    }
    else if (strcasecmp (command, "delete") == 0) {
        // todo
    }
    else {
        printErrorInvalidCommand (stdout, command);
    }
    return 0;
}

/**
 * RETURN:
 * length of server response to the msg
 * -1 on error
 * */
int user_cmd (const char *parameter)
{
    char buffer[BUFSIZ];
    memset (buffer, 0, BUFSIZ);
    snprintf (buffer, BUFSIZ, "%s%s", "user ", parameter);

    ssize_t length = send_msg (buffer);

    if (length < 0)
        return -1;

    return read_response (buffer);
}

int pass_cmd (const char *parameter)
{
    char buffer[BUFSIZ];
    memset (buffer, 0, BUFSIZ);
    snprintf (buffer, BUFSIZ, "%s%s", "pass ", parameter);

    ssize_t length = send_msg (buffer);

    if (length < 0)
        return -1;

    return read_response (buffer);
}
/*
* returns -1 on error, number of bytes sent otherwise
*/
int send_msg (const char *msg)
{
    char buffer[BUFSIZ];
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
    memset (buffer, 0, BUFSIZ);
    ssize_t length = read (sfd, buffer, BUFSIZ);
    if (length <= 0) {
        perror ("Error while reading from server. Terminiating Control Connection");
        return -1;
    }
    printResponse (stdout, buffer);
    char *rcode = strtok (buffer, " ");
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
    int count = 0;
    char argcpy[BUFSIZ];

    if (exepcted == 0) {
        return args == NULL;
    }

    // make a copy of parameter for count checking
    memset (argcpy, 0, BUFSIZ);
    snprintf (argcpy, BUFSIZ, "%s", args);

    char *param = strtok (argcpy, " \t\n\f\r\v");
    while (param != NULL) {
        param = strtok (NULL, " \t\n\f\r\v");
        count++;
    }

    return count == exepcted;
}

// pasvr_t getPasvData()
// {
// }

// void flush()
// {
// }
