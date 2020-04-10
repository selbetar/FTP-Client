
#include "printRoutines.h"

int printErrorInvalidCommand (FILE *file, const char *command)
{
    // print HELP
    fprintf (file, "Invalid command: %s\n",
             command);
    printHelp (stdout);
    return 0;
}

int printErrorInvalidParameter (FILE *file, const char *parameter)
{
    fprintf (file, "Invalid parameter or number of parameters: %s\n",
             parameter);
    printHelp (stdout);
    return 0;
}

int printResponse (FILE *file, const char *response)
{
    return fprintf (file, "--> %s",
                    response);
}

int printSent (FILE *file, const char *msg)
{
    return fprintf (file, "<-- %s",
                    msg);
}

int printErrorCommmandTooLong (FILE *file)
{
    return fprintf (file, "Command is too long, ignored.\n");
}

int printHelp (FILE *file)
{
    return fprintf (file, "Commands: \n%s",
                    "              user   <user>\n"
                    "              pass   <pass>\n"
                    "              get    <file>    - to retrieve a file\n"
                    "              put    <file>    - to store a file on the ftp server\n"
                    "              delete <file>    - to delete a file on the ftp server\n"
                    "              cd     <path>    - to naviagte to a different folder\n"
                    "              nlist  <path>?   - lists files in <path> or current directory if no argument is provided\n"
                    "              pwd              - print current directory\n"
                    "              features         - list ftp server features\n"
                    "              quit             - ends session and closes the client\n");
}

void printDebug (char *msg)
{
    printf ("DEBUG: %s%s\n", "reached ", msg);
}