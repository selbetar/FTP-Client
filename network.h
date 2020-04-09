#ifndef __NETWORK_H__
#define __NETWORK_H__

#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

int create_connection (const char *host, const char *port);

#endif