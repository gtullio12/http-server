#include "parse_header.h"
#include "stdio.h"

void parse_post_header(char *raw_headers, Post_Header *parse_header) {

}

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

void parse_get_header(char *raw_headers, Get_Header *get_header) {
    int index = 0;

    char *pointer = raw_headers;
    parse_field(&pointer, get_header->host);
    parse_field(&pointer, get_header->user_agent);
    parse_field(&pointer, get_header->accept);
}
