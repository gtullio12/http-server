#include "parse_header.h"
#include "stdio.h"
#include "stdlib.h"

void parse_field(char **pointer, char *field) {
    // Move pointer to value 
    while (**(pointer) != ' ' && **(pointer) != '\0') {
        ++(*pointer);
    }
    ++(*pointer);

    // Parse value
    int field_index = 0;
    while (**pointer != '\n' && **pointer != ' ' && **(pointer) != '\0') {
        field[field_index++] = **pointer;
        ++(*pointer);
    }
    field[field_index] = '\0';
    ++(*pointer);
}

void parse_post_header(char *raw_headers, Post_Header *post_header) {
    char *pointer = raw_headers;
    parse_field(&pointer, post_header->host);
    parse_field(&pointer, post_header->user_agent);
    parse_field(&pointer, post_header->accept);
    char content_length[10];
    parse_field(&pointer, content_length);
    post_header->content_length = atoi(content_length);
}

void parse_get_header(char *raw_headers, Get_Header *get_header) {
    int index = 0;

    char *pointer = raw_headers;
    parse_field(&pointer, get_header->host);
    parse_field(&pointer, get_header->user_agent);
    parse_field(&pointer, get_header->accept);
}
