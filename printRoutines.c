
#include "printRoutines.h"

int printErrorInvalidCommand (FILE *file, const char *command)
{
    // print HELP
    return fprintf (file, "Invalid command: %s\n",
                    command);
}

int printErrorInvalidParameter (FILE *file, const char *parameter)
{
    return fprintf (file, "Invalid parameter or number of parameters: %s\n",
                    parameter);
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
