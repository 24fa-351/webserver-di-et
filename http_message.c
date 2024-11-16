#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "http_message.h"

void read_http_client_message(int sock_fd, http_client_message_t** msg, http_read_result_t* result)
{
    char buffer[1024];
    int bytes_read = read(sock_fd, buffer, sizeof(buffer));
    if (bytes_read == -1)
    {
        perror("read\n");
        *result = BAD_REQUEST;
        return;
    }

    buffer[bytes_read] = '\0';
    printf("recieved %d bytes: %s\n", bytes_read, buffer);
    
    *msg = malloc(sizeof(http_client_message_t));
    if(*msg == NULL)
    {
        *result = BAD_REQUEST;
        return;
    }
}

void print_http_client_message()
{
    printf("printing request\n");
}

void http_client_message_free(http_client_message_t* msg)
{
    printf("freeing request\n");
    free(msg);
}