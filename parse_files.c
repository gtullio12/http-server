#include "parse_header.h"
#include "connection_handler.h"
#include "stdio.h"
#include "parse_files.h"
#include "parse_header.h"
#include <string.h>  /* strerror() */

void parse_files(int fd, struct Post_Header *post_header, char *boundary) {

    char boundary_delimeter[200];
    snprintf(boundary_delimeter, sizeof(boundary_delimeter), "--%s--", boundary);

    while (1) {
        char current_line[200];
        get_current_line(fd, current_line);

        if (strcmp(current_line, boundary_delimeter) == 0) { // We haven't hit end of payload
            printf("current line -> %s\n", current_line);
            puts("Hit boundary");
            break;
        } else {
        }
    }
}
