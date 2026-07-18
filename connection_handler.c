#include "connection_handler.h"
#include "parse_files.h"
#include "parse_header.h"
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

struct PostRequest {
    int content_length;
    char *file_name;
    char *boundary;
};

/**
 * 1 = Valid method
 * 0 = Invalid method
 * */
int verifyRequestMethod(const char *method) {
    if (strcmp(method, "GET") == 0 || strcmp(method, "POST") == 0 ||
        strcmp(method, "DELETE") == 0 || strcmp(method, "PUT") == 0 ||
        strcmp(method, "HEAD") == 0 || strcmp(method, "CONNECT") == 0 ||
        strcmp(method, "OPTIONS") == 0 || strcmp(method, "TRACE") == 0) {
        return 1;
    }
    return 0;
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

    if (verifyRequestMethod(request_line->request_method) == 0) {
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
    while (*(buf + index) != ' ' && *(buf + index) != '\0') {
        request_line->http_version[http_version_index++] = *(buf + index);
        ++index;
    }


    if (strcmp(request_line->http_version, "HTTP/1.1") == 0) {
        return ERR_HTTP_VERSION_NOT_SUPPORTED;
    }

    return 0;
}

void get_current_line(int fd, char *line) {
    int index = 0;

    // Get first line
    while (1) {
        char temp_buf[1];
        ssize_t count = recv(fd, temp_buf, 1, 0);

        if (count < 0) {
            fprintf(stderr, "Receive error: %s\n", strerror(errno));
            abort();
        } else if (count == 0) {
            puts("Connection lost");
            break;
        }

        if (temp_buf[0] == '\n' || temp_buf[0] == '\0' || temp_buf[0] == '\r') 
            break;

        line[index++] = temp_buf[0];
        temp_buf[0] = '\0';
    }

    line[index] = '\0';
}


void handle_connection(int fd) {
    printf("Established connection to socket: %d\n", fd);

    // Read bytes from network. Typical CURL would be: 
    /*
       GET <url> HTTP/1.1
       Host: <hostname>
       Accept: 
       */

    // Get first line from client
    char raw_request_line[100];
    get_current_line(fd, raw_request_line);

    struct RequestLine request_line;
    request_line.request_method = malloc(10);
    memset(request_line.request_method, '\0', 10);
    request_line.request_path = malloc(200);
    memset(request_line.request_path, '\0', 100);
    request_line.http_version = malloc(200);
    memset(request_line.http_version, '\0', 5);

    int res = getRequestLine(raw_request_line, &request_line);

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

    if (strcmp(request_line.request_method, "GET") == 0) {

        // Read all headers first from socket, then parse after 
        char raw_headers[1000];
        ssize_t c = recv(fd, raw_headers, sizeof(raw_headers), 0);

        if (c < 0) {
            fprintf(stderr, "Receive error: %s\n", strerror(errno));
            abort();
        } else if (c == 0) {
            puts("Connection lost");
        }

        struct Get_Header get_header;
        get_header.user_agent = malloc(100);
        get_header.accept = malloc(100);
        get_header.host = malloc(100);

        parse_get_header(raw_headers, &get_header);

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
        free(get_header.user_agent);
        free(get_header.accept);
        free(get_header.host);
        close(fd);
    } else if (strcmp(request_line.request_method, "POST") == 0) {
        struct Post_Header post_header;
        post_header.user_agent = malloc(20);
        post_header.accept = malloc(20);
        post_header.host = malloc(20);

        char raw_headers[500];
        int raw_headers_pointer = 0;
        char *boundary = malloc(100);

        // First we need to get the headers, stop at boundary
        while (1) {
            char line[100];
            get_current_line(fd, line);
            char *boundary_pointer = strstr(line, "boundary");

            if (boundary_pointer != NULL) {

                // Move pointer to value
                while (*(boundary_pointer) != '=' && *(boundary_pointer) != '\0') {
                    ++boundary_pointer;
                }
                ++boundary_pointer;

                int boundary_index = 0;

                while (*boundary_pointer != ' ' && *boundary_pointer != '\n' && *boundary_pointer != '\r' && *boundary_pointer != '\0') {
                    boundary[boundary_index++] = *boundary_pointer++;
                }
                boundary[boundary_index] = '\0';

                // Increment socket to boundary
                int t;
                for (t=0;t<5;t++) {
                    char current_line[100];
                    get_current_line(fd, current_line);

                    if (strcmp(current_line, boundary) == 0) {
                        break;
                    }
                }
                break;
            }

            // Copy content of line into raw_headers
            char *p = line;
            while (*p != '\0') {
                raw_headers[raw_headers_pointer++] = *p;
                ++p;
            }
            raw_headers[raw_headers_pointer++] = '\n';
        }
        raw_headers[raw_headers_pointer] = '\0';

        // Now that we have read all the headers from the socket. we can parse it
        parse_post_header(raw_headers, &post_header);

        parse_files(fd, &post_header, boundary);

        free(boundary);
    }

    free(request_line.request_method);
    free(request_line.request_path);
    free(request_line.http_version);
}



