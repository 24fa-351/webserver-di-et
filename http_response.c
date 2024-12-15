#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

#include "http_response.h"

void response_free(http_server_response_t* msg)
{
    printf("freeing response\n");
    free(msg);
}

void response_print(http_server_response_t* msg)
{
    if(msg->body)
    {
        printf("body: %s\n", msg->body);
    }
    if(msg->http_version)
    {
        printf("http version: %s\n", msg->http_version);
    }
    if (msg->status_code)
    {
        printf("status code: %d\n", msg->status_code);
    }
    if (msg->body_length)
    {
        printf("body length: %d\n", msg->body_length);
    }    
}

// responses: a message, bad request, closed connection
bool response_send(http_server_response_t* msg, int fd)
{
    if (msg == NULL)
    {
        return false;
    }
    dprintf(fd, "%s %d OK\n", msg->http_version, msg->status_code);
    dprintf(fd, "Content-Length: %d\n", msg->body_length);
    write(fd, "\n", 1);
    write(fd, msg->body, msg->body_length);
    return true;
}
// Compare this snippet from webserver-di-et/router.h: