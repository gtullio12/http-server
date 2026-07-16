#ifndef PARSE_HEADER_H
#define PARSE_HEADER_H

typedef struct {
    char *boundary;
} Content_Type;

typedef struct {
    char *name;
    char *filename;
} Content_Disposition;

typedef struct Post_Header_File {
    Content_Disposition content_disposition;
    char *content_type;
    struct Post_Header_File *next;
} Post_Header_File;

typedef struct {
    char* host;
    char *user_agent;
    char *accept;
    int content_length;
    Content_Type content_type;
    struct Post_Header_File *files;
} Post_Header;

typedef struct Get_Header {
    char* host;
    char *user_agent;
    char *accept;
} Get_Header;

void parse_post_header(char *raw_headers, Post_Header *parse_header);
void parse_get_header(char *raw_headers, Get_Header *get_header);

#endif
