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

        // break loop if EOF
        if (!fgets (line, sizeof (line), stdin))
            break;

        // line could not be read entirely
        if (!strchr (line, '\n')) {
            // read to the end of the lline
            while ((c = fgetc (stdin)) != EOF && c != '\n')
                ;
            if (c == '\n') {
                printErrorCommmandTooLong (stdout);
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

int execute_command (const char *command, const char *parameter)
{
    if (strcasecmp (command, "quit") == 0) {
        if (!check_argc (parameter, 0))
            return printErrorInvalidParameter (stdout, parameter);
        return quit_cmd (NULL);
    }
    else if (strcasecmp (command, "user") == 0) {
        if (!check_argc (parameter, 1))
            return printErrorInvalidParameter (stdout, parameter);
        return login_cmd ("user", parameter, NULL);
    }
    else if (strcasecmp (command, "pass") == 0) {
        if (!check_argc (parameter, 1))
            return printErrorInvalidParameter (stdout, parameter);
        return login_cmd ("pass", parameter, NULL);
    }
    else if (strcasecmp (command, "get") == 0) {
        if (!check_argc (parameter, 1))
            return printErrorInvalidParameter (stdout, parameter);
        return get_cmd (parameter, NULL);
    }
    else if (strcasecmp (command, "features") == 0) {
        if (!check_argc (parameter, 0))
            return printErrorInvalidParameter (stdout, parameter);
        return rand_cmd ("feat", NULL, NULL);
    }
    else if (strcasecmp (command, "cd") == 0) {
        if (!check_argc (parameter, 1))
            return printErrorInvalidParameter (stdout, parameter);
        return rand_cmd ("cwd", parameter, NULL);
    }
    else if (strcasecmp (command, "nlist") == 0) {
        if (!check_argc (parameter, 0) && !check_argc (parameter, 1))
            return printErrorInvalidParameter (stdout, parameter);
        return nlst_cmd (parameter, NULL);
    }
    else if (strcasecmp (command, "pwd") == 0) {
        if (!check_argc (parameter, 0))
            return printErrorInvalidParameter (stdout, parameter);
        return rand_cmd ("pwd", NULL, NULL);
    }
    else if (strcasecmp (command, "put") == 0) {
        if (!check_argc (parameter, 1))
            return printErrorInvalidParameter (stdout, parameter);
        return put_cmd (parameter, NULL);
    }
    else if (strcasecmp (command, "delete") == 0) {
        if (!check_argc (parameter, 1))
            return printErrorInvalidParameter (stdout, parameter);
        return rand_cmd ("dele", parameter, NULL);
    }
    else {
        return printErrorInvalidCommand (stdout, command);
    }
    return 0;
}

int login_cmd (const char *cmd, const char *param, char *response)
{
    int length;
    char buffer[BUF_SIZE];
    char *res_buf = response == NULL ? buffer : response;
    length        = rand_cmd (cmd, param, res_buf);

    return length < 0 ? -1 : bin_type (res_buf);
}

int rand_cmd (const char *cmd, const char *param, char *response)
{
    char buffer[BUF_SIZE];

    char *res_buff = response == NULL ? buffer : response;
    memset (res_buff, 0, BUF_SIZE);

    if (param != NULL) {
        snprintf (res_buff, BUF_SIZE, "%s %s", cmd, param);
    }
    else {
        snprintf (res_buff, BUF_SIZE, "%s", cmd);
    }

    ssize_t length = send_msg (res_buff);

    return length < 0 ? -1 : read_response (res_buff);
}

int pasv_cmd (char *response)
{
    char buffer[BUF_SIZE];
    u_int32_t a, h1, h2, h3, h4, p1, p2;
    char t[256];

    int data_fd   = -1;
    char *res_buf = response == NULL ? buffer : response;
    memset (res_buf, 0, BUF_SIZE);
    snprintf (res_buf, BUF_SIZE, "%s", "pasv");

    ssize_t length = send_msg (res_buf);

    if (length < 0)
        return -1;

    if (read_response (res_buf) < 0)
        return -1;

    if (strstr (res_buf, "227") != res_buf)
        return 0;

    length = sscanf (res_buf, "%d %s %s %s (%u,%u,%u,%u,%u,%u)",
                     &a, &t[1], &t[2], &t[3], &h1, &h2, &h3, &h4, &p1, &p2);
    if (length != 10) {
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

int nlst_cmd (const char *path, char *response)
{
    char buffer[BUF_SIZE];
    int data_fd;
    ssize_t length;

    char *res_buf = response == NULL ? buffer : response;

    data_fd = pasv_cmd (NULL);
    if (data_fd <= 0)
        return data_fd;

    length = rand_cmd ("nlst", path, res_buf);
    if (length < 0)
        return -1;

    if (strstr (res_buf, "150 ") == res_buf || strstr (res_buf, "125 ") == res_buf) {
        memset (res_buf, 0, BUF_SIZE);
        while (read (data_fd, res_buf, BUF_SIZE) > 0) {
            fprintf (stdout, "%s", res_buf);
            memset (res_buf, 0, BUF_SIZE);
        }
        length = read_response (res_buf);
        if (length < 0)
            return -1;
    }

    close (data_fd);
    return 1;
}

int get_cmd (const char *file, char *response)
{
    char buffer[BUF_SIZE];
    int data_fd, file_fd;
    ssize_t length, w_len;
    char *res_buf = response == NULL ? buffer : response;

    // check if file exists so we do not overwrite it
    file_fd = open (file, O_WRONLY | O_CREAT | O_EXCL, 0777);
    if (file_fd == -1) {
        perror ("Failed to create the file");
        fprintf (stderr, "Rename the file on disk first then call get again.\n");
        return 1;
    }

    data_fd = pasv_cmd (NULL);
    if (data_fd <= 0)
        return data_fd;

    length = rand_cmd ("retr", file, res_buf);
    if (length < 0)
        return -1;

    if (strstr (res_buf, "150 ") == res_buf || strstr (res_buf, "125 ") == res_buf) {
        memset (res_buf, 0, BUF_SIZE);
        while ((length = read (data_fd, res_buf, BUF_SIZE)) > 0) {
            w_len = write (file_fd, res_buf, length);
            if (w_len < 0 || length < 0) {
                perror ("An Error Occuered during file transfer. Closing Client");
                close (data_fd);
                close (file_fd);
                return -1;
            }
            memset (res_buf, 0, BUF_SIZE);
        }
        length = read_response (res_buf);
        if (length < 0)
            return -1;
    }

    close (data_fd);
    close (file_fd);

    return 1;
}

int put_cmd (const char *file, char *response)
{
    char buffer[BUF_SIZE];
    int data_fd, file_fd;
    ssize_t length, w_len;
    char *res_buf = response == NULL ? buffer : response;

    file_fd = open (file, O_RDONLY);
    if (file_fd == -1) {
        perror ("failed to open the file");
        return 1;
    }

    data_fd = pasv_cmd (NULL);
    if (data_fd <= 0)
        return data_fd;

    length = rand_cmd ("stor", file, res_buf);
    if (length < 0)
        return -1;

    if (strstr (res_buf, "150 ") == res_buf || strstr (res_buf, "125 ") == res_buf) {
        memset (res_buf, 0, BUF_SIZE);
        while ((length = read (file_fd, res_buf, BUF_SIZE)) > 0) {
            w_len = send (data_fd, res_buf, length, 0);
            if (w_len != length) {
                perror ("An Error Occuered during file transfer. Closing Client");
                close (data_fd);
                close (file_fd);
                return -1;
            }
            memset (res_buf, 0, BUF_SIZE);
        }

        close (data_fd);
        close (file_fd);

        length = read_response (res_buf);
        if (length < 0)
            return -1;
    }

    close (data_fd);
    close (file_fd);
    return 1;
}

int quit_cmd (char *response)
{
    char buffer[BUF_SIZE];
    memset (buffer, 0, BUF_SIZE);
    snprintf (buffer, BUF_SIZE, "%s", "quit");

    ssize_t length = send_msg (buffer);

    if (length < 0)
        return -1;

    char *res_buf = response == NULL ? buffer : response;
    read_response (res_buf);
    cleanup();
    return -2;
}

int bin_type (const char *response)
{
    if (strstr (response, "230 ")) {
        ssize_t length = rand_cmd ("type", "I", NULL);
        if (length < 0)
            return -1;
    }
    return 1;
}

ssize_t send_msg (const char *msg)
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

ssize_t read_response (char *response)
{

    memset (response, 0, BUF_SIZE);
    ssize_t length = read (sfd, response, BUF_SIZE);
    if (length <= 0) {
        perror ("Error while reading from server. Terminiating Control Connection");
        return -1;
    }

    printResponse (stdout, response);

    int i_rcode;
    char buf[24];
    memset (buf, 0, 24);
    // check for multiline response: d{3}-
    if ((sscanf (response, "%u%s", &i_rcode, buf) == 2) && (strstr (buf, "-") == buf)) {
        memset (buf, 0, 24);
        snprintf (buf, 24, "%d%s", i_rcode, " End");

        // check if the entire response hasn't been sent
        if (strstr (response, buf) == NULL) {
            while (1) {
                memset (response, 0, BUF_SIZE);
                length = read (sfd, response, BUF_SIZE);
                if (strstr (response, buf) != NULL)
                    break;
            }
        }
    }

    // copy the response to manipulate it
    // without modifing the original buffer
    char rcopy[length];
    memset (rcopy, 0, length);
    snprintf (rcopy, BUF_SIZE, "%s", response);

    char *rcode = strtok (rcopy, " ");
    if (strcmp (rcode, "421") == 0) {
        printf ("%s\n", "Server Closing Control Connection. Shutting Down.");
        return -1;
    }
    return length;
}

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
