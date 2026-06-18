#include "connection_handler.h"
#include <stdio.h>
#include <stddef.h>  /* ssize_t */
#include <string.h>  /* strerror() */
#include <stdlib.h>  /* abort() */
#include <sys/socket.h>
#include "errno.h"

void handle_connection(int fd) {
    printf("Established connection to socket: %d\n", fd);

    // Read bytes from network. Typical CURL would be: 
    /*
       GET <url> HTTP/1.1
       Host: <hostname>
       Accept: 
       */

    char buf[5];
    for(;;) {
        const ssize_t received = recv(fd, &buf, sizeof(buf), 0);

        if (received < 0) {
            fprintf(stderr, "Receive error: %s\n", strerror(errno));
            abort();
        }

        printf("%s\n", buf);

    }
}


