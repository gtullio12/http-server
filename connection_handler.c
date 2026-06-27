#include "connection_handler.h"
#include <stdio.h>
#include <stddef.h>  /* ssize_t */
#include <string.h>  /* strerror() */
#include <stdlib.h>  /* abort() */
#include <sys/socket.h>
#include "errno.h"
#include <fcntl.h>
#include <unistd.h>

struct RequestLine {
    char *request_method;
    char *request_path;
    char *http_version;
};


int getRequestLine(char *buf, struct RequestLine *requestLine) {
    int index;
    for (index=0;*(buf + index) != '/'; index++) {
        requestLine->request_method[index] = *(buf + index);
    }
    return 0;
}

void handle_connection(int fd) {
    printf("Established connection to socket: %d\n", fd);

    // Read bytes from network. Typical CURL would be: 
    /*
       GET <url> HTTP/1.1
       Host: <hostname>
       Accept: 
       */


    printf("Hello world\n");
    char buf[100];
    ssize_t count = recv(fd, &buf, sizeof(buf), 0);

    int save = open("save.txt", O_RDWR | O_CREAT, 0644);
    write(save, buf, strlen(buf));

    struct RequestLine requestLine;
    requestLine.request_method = malloc(10);
    memset(requestLine.request_method, '\0', 10);
    requestLine.request_path = malloc(100);
    memset(requestLine.request_path, '\0', 100);
    requestLine.http_version = malloc(5);
    memset(requestLine.http_version, '\0', 5);

    int res = getRequestLine(buf, &requestLine);

    //printf("buffer contents: %s\n\n", buf);
    if (count < 0) {
        fprintf(stderr, "Receive error: %s\n", strerror(errno));
        abort();
    } else if (count == 0) {
        puts("Connection lost");
    }
}



