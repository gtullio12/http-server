#ifndef HTTP_ERRORS_H
#define HTTP_ERRORS_H

typedef enum {
    ERR_HTTP_VERSION_NOT_SUPPORTED = -1,
    ERR_BAD_REQUEST = -2,
    ERR_NOT_FOUND = -3,
    // ...
} http_error_t;

#endif  // HTTP_ERRORS_H
