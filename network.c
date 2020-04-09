 #include "network.h"
 #include "common.h"
#include <stdio.h>

int create_connection (const char *host, const char *port)
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd, s, j;
    size_t len;
    ssize_t nread;
    char buf[BUF_SIZE];

    /* Obtain address(es) matching host/port */
    memset (&hints, 0, sizeof (struct addrinfo));
    hints.ai_family   = PF_INET;   /* Allow IPv4 */
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = 0;
    hints.ai_protocol = 0; /* Any protocol */

    s = getaddrinfo (host, port, &hints, &result);
    if (s != 0) {
        fprintf (stderr, "getaddrinfo: %s\n", gai_strerror (s));
        return -1;
    }

    /* getaddrinfo() returns a list of address structures.
              Try each address until we successfully connect(2).
              If socket(2) (or connect(2)) fails, we (close the socket
              and) try the next address. */

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket (rp->ai_family, rp->ai_socktype,
                      rp->ai_protocol);
        if (sfd == -1)
            continue;

        if (connect (sfd, rp->ai_addr, rp->ai_addrlen) != -1)
            break; /* Success */

        close (sfd);
    }

    if (rp == NULL) { /* No address succeeded */
        fprintf (stderr, "Could not connect\n");
        return -1;
    }

    freeaddrinfo (result); /* No longer needed */

    return sfd;
}