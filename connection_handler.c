#include "connection_handler.h"
#include <stdio.h>
#include <stddef.h>  /* ssize_t */
#include <string.h>  /* strerror() */
#include <stdlib.h>  /* abort() */
#include <sys/socket.h>
#include "errno.h"
#include <fcntl.h>
#include <unistd.h>
#include "http_errors.h"

struct RequestLine {
    char *request_method;
    char *request_path;
    char *http_version;
};

int strcmp(const char *s1, const char *s2) {
    while (*s1 == *s2 && *s1 != '\0' && *s2 != '\0') {
        s1++;
        s2++;
    }
    if (*s1 == *s2) return 1; // the strings are equal
    else return 0; // the strings are not equal
}

/**
 * * : (colon) - typically used to separate volume labels from paths (e.g., "C:\")
* * (asterisk) - can be confused with wildcard characters
* ? (question mark) - can be confused with the query string separator
* " (double quote) - can be confused with string literals
* / (forward slash) - typically used to separate directory levels
* < (less-than sign) - can be confused with HTML tags
* > (greater-than sign) - can be confused with HTML tags
* | (vertical bar) - can be confused with pipe characters
* \ (backslash) - can be confused with path separators on Unix-like systems
* */
int validateRequestPathName(char *val) {
    return 0;
}

int verifyRequestMethod(const char *method) {

    if (strcmp(method, "GET") + strcmp(method, "POST") + strcmp(method, "DELETE") + strcmp(method, "PUT") + 
            strcmp(method, "HEAD") + strcmp(method, "CONNECT") + strcmp(method, "OPTIONS") + strcmp(method, "TRACE") != 1) {
        return 0;
    }
    return 1;
}

int verifyRequestPath(char *path) {
    char *token;
    token = strtok(path, "/");
    while (token != NULL) {
        token = strtok(NULL, "/");
        printf("token -> %s\n", token);
    }

    return 1;
}



int getRequestLine(char *buf, struct RequestLine *requestLine) {
    int index = 0;
    // Parse the request method
    while (*(buf + index) != ' ') {
        requestLine->request_method[index] = *(buf + index);
        ++index;
    }

    if (verifyRequestMethod(requestLine->request_method) != 1) {
        return ERR_BAD_REQUEST;
    }

    ++index;

    int path_index = 0;
    // Parse the file path
    while (*(buf + index) != ' ') {
        // BUG: Handle spaces in file path
        requestLine->request_path[path_index++] = *(buf + index);
        ++index;
    }

    puts("Verifying request path");

    if (verifyRequestPath(requestLine->request_path) == 0) {
        return ERR_BAD_REQUEST;
    }


    ++index;
    int http_version_index = 0;
    // Parse the HTTP version
    while (*(buf + index) != ' ') {
        requestLine->http_version[http_version_index++] = *(buf + index);
        ++index;
    }


    if (strcmp(requestLine->http_version, "HTTP/1.1") == 1) {
        return ERR_HTTP_VERSION_NOT_SUPPORTED;
    }

    // FIX THIS. 0 should be success, anything less than 0 is a error
    return -1;
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

    if (res >= 0) {
        switch (res) {
            case ERR_HTTP_VERSION_NOT_SUPPORTED:
                fprintf(stderr, "HTTP Version not supported. Only HTTP/1.1");
                break;
            case ERR_BAD_REQUEST:
                fprintf(stderr, "BAD REQUEST");
                break;
        }
    }

    //printf("buffer contents: %s\n\n", buf);
    if (count < 0) {
        fprintf(stderr, "Receive error: %s\n", strerror(errno));
        abort();
    } else if (count == 0) {
        puts("Connection lost");
    }
}



