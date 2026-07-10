#include "connection_handler.h"
#include <asm-generic/errno-base.h>
#include <stdio.h>
#include <stddef.h>  /* ssize_t */
#include <string.h>  /* strerror() */
#include <stdlib.h>  /* abort() */
#include <sys/socket.h>
#include "errno.h"
#include <fcntl.h>
#include <unistd.h>
#include "http_errors.h"
#include <fnmatch.h>
#include <sys/stat.h>

#define CHUNK_SIZE 4096

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

int verifyRequestMethod(const char *method) {

    if (strcmp(method, "GET") + strcmp(method, "POST") + strcmp(method, "DELETE") + strcmp(method, "PUT") + 
            strcmp(method, "HEAD") + strcmp(method, "CONNECT") + strcmp(method, "OPTIONS") + strcmp(method, "TRACE") != 1) {
        return 0;
    }
    return 1;
}

/**
 * 0 for fail, 1 for success
 */
int verifyRequestPath(char *path) {
    const char *pattern = "*./*:*?|\"";
    return fnmatch(pattern, path, 0);
}

/**
 * 1 = Success, path is valid
 * 0 = Fail, File/Directory doesn't exist
 */
int isPathValid(char *path) {
    int res = access(path, F_OK);
    if (res == 0) {
        puts("File found successfully");
        return 1;
    } else {
        printf("ERRNO -> %d\n", errno);
        switch(errno) {
            case ENOENT:
                fprintf(stderr, "No such file of directory for request path");
        }
        return 0;
    }
}

/**
 * Opens file and handle errors when opening
 */
int openFile(const char *path) {
    // Open the file in read only mode
    int fd = open(path, O_RDONLY | O_NONBLOCK, 0);
    if (fd < 0) {
        switch (errno) {
            case ENOENT:
                printf("No such file or directory for path: %s\n", path);
                break;
            case EACCES:
                printf("Permission Denied for path: %s\n", path);
                break;
            case EIO:
                printf("Input/output error(your disk is full)");
        }
        return -1;
    }
    return fd;
}

int getRequestLine(char *buf, struct RequestLine *request_line) {
    int index = 0;
    // Parse the request method
    while (*(buf + index) != ' ') {
        request_line->request_method[index] = *(buf + index);
        ++index;
    }

    if (verifyRequestMethod(request_line->request_method) != 1) {
        return ERR_BAD_REQUEST;
    }

    ++index;

    int path_index = 0;
    // Parse the file path
    while (*(buf + index) != ' ') {
        request_line->request_path[path_index++] = *(buf + index);
        ++index;
    }

    // Verify path doesn't have any incorrect characters
    int validRequestPath = verifyRequestPath(request_line->request_path);

    if (validRequestPath == 0) {
        return ERR_BAD_REQUEST;
    }

    // Verify path is valid
    int validPath = isPathValid(request_line->request_path);
    if (validPath == 0) {
        return ERR_NOT_FOUND;
    }

    ++index;
    int http_version_index = 0;
    // Parse the HTTP version
    while (*(buf + index) != ' ') {
        request_line->http_version[http_version_index++] = *(buf + index);
        ++index;
    }


    if (strcmp(request_line->http_version, "HTTP/1.1") == 1) {
        return ERR_HTTP_VERSION_NOT_SUPPORTED;
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


    char buf[100];
    ssize_t count = recv(fd, &buf, sizeof(buf), 0);

    if (count < 0) {
        fprintf(stderr, "Receive error: %s\n", strerror(errno));
        abort();
    } else if (count == 0) {
        puts("Connection lost");
    }

    struct RequestLine request_line;
    request_line.request_method = malloc(10);
    memset(request_line.request_method, '\0', 10);
    request_line.request_path = malloc(100);
    memset(request_line.request_path, '\0', 100);
    request_line.http_version = malloc(5);
    memset(request_line.http_version, '\0', 5);

    int res = getRequestLine(buf, &request_line);

    if (res < 0) {
        switch (res) {
            case ERR_HTTP_VERSION_NOT_SUPPORTED:
                fprintf(stderr, "HTTP Version not supported. Only HTTP/1.1");
                break;
            case ERR_BAD_REQUEST:
                fprintf(stderr, "BAD REQUEST");
                break;
        }
    }

    if (strcmp(request_line.request_method, "GET") == 1) {

        // Now lets open the file, serialize it, and return back to the user
        int file = openFile(request_line.request_path);
        if (file < 0) {
            perror("Error finding file");
            return;
        }

        // Get file information
        struct stat sb;
        fstat(file, &sb);

        // Send headers first
        char headers[200];
        // TODO: determine if connection should be keep-alive or close
        sprintf(headers, "HTTP/1.1 200 OK\n"
                    "Content-Length: %ld\r\n"
                    "Connection: close\r\n"
                    "\r\n", sb.st_size);

        send(fd, headers, strlen(headers), 0);

        char *buffer = malloc(CHUNK_SIZE);

        while (1) {
            off_t bytes_read = read(file, buffer, CHUNK_SIZE);

            if (bytes_read == 0) { // End of file reached
                puts("End of file reached");
                return;
            } else if (bytes_read == -1) { // Error reading chunk
                perror("Error reading bytes");
                break;
            }

            send(fd, buffer, CHUNK_SIZE, 0);
        }

        free(buffer);
        close(fd);
    }

}



