#include "parse_header.h"

#ifndef PARSE_FILES_HEADER
#define PARSE_FILES_HEADER

/**
 * Take in socket, and post_header which contains empty Linkedlist of files 
 * */
void parse_files(int fd, struct Post_Header *post_header, char *boundary);


#endif
