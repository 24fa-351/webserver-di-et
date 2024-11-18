#ifndef HTTP_MESSAGE_H
#define HTTP_MESSAGE_H

typedef struct 
{
    char* key;
    char* value;
}Header;

typedef struct msg
{
    char* method;
    char* path;
    char* http_version;
    int header_count;
    Header* headers;
}http_client_message_t;

// responses: a message, bad request, closed connection
// allocates and returns a message
http_client_message_t* read_http_client_message(int sock_fd);

void free_http_client_message(http_client_message_t* msg);

void print_http_client_message(http_client_message_t* msg);

#endif