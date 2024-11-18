#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <stdbool.h>


typedef struct response
{
    int status_code;
    char* http_version;
    char* body;
    int body_length;
    // char* headers;
}http_server_response_t;

// responses: a message, bad request, closed connection
// allocates and returns a message
void response_free(http_server_response_t* msg);

void response_print(http_server_response_t* msg);

bool response_send(http_server_response_t* msg, int fd);

#endif