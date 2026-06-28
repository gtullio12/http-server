#ifndef HTTP_ERRORS_H
#define HTTP_ERRORS_H

typedef enum {
    ERR_HTTP_VERSION_NOT_SUPPORTED,
    ERR_BAD_REQUEST,
    ERR_NOT_FOUND,
    // ...
} http_error_t;

#endif  // HTTP_ERRORS_H
