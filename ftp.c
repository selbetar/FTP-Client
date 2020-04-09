#include "ftp.h"
#include "network.h"
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

    printf ("--> %s", buffer);

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

        int scode = executeCommand (command, parameter);
    }
    return 0;
}

int executeCommand (const char *command, const char *parameter)
{
    if (strcasecmp (command, "quit") == 0) {
        // todo
    }
    else if (strcasecmp (command, "user") == 0) {
        // todo
    }
    else if (strcasecmp (command, "pass") == 0) {
        // todo
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
        // todo
    }
}

int send_msg (const char *msg)
{
}

int read_response (char *buf)
{
    ssize_t length = read (sfd, buf, BUFSIZ);
    return length;
}

// pasvr_t getPasvData()
// {
// }

// void flush()
// {
// }
