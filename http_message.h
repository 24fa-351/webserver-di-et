#ifndef HTTP_MESSAGE_H
#define HTTP_MESSAGE_H

typedef struct msg
{
    char* method;
    char* path;
    char* http_version;
    char* body;
    int body_length;
    char* headers;
}http_client_message_t;

typedef enum
{
    BAD_REQUEST,
    CLOSE_CONNECTION,
    MESSAGE
}http_read_result_t;

// responses: a message, bad request, closed connection
// allocates and returns a message
void read_http_client_message(int client_sock, http_client_message_t** msg, http_read_result_t* result);

void http_client_message_free(http_client_message_t* msg);

void write_http_client_message(http_client_message_t* msg);
#endif