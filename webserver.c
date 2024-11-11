#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdbool.h>

#include "http_message.h"

#define DEFAULT_PORT 80
#define LISTEN_BACKLOG 5

int respond_to_http_client_message(int sock_fd, http_client_message_t* http_msg)
{
    char* response = "HTTP/1.1 200 OK \r\nContent-Length: 0\r\n\r\n";
    write(sock_fd, response, strlen(response));
    return 0;
}

void handle_connection(int* sock_fd_ptr)
{
    int sock_fd = *sock_fd_ptr;
    free(sock_fd_ptr);
    // bool done = false;

    while (1)
    {
        printf("handling connection on %d\n", sock_fd);
        http_client_message_t* http_msg;
        http_read_result_t result;
        read_http_client_message(sock_fd, &http_msg, &result);

        if (result == BAD_REQUEST)
        {
            printf("Bad request\n");
            close(sock_fd);
            return;
        } else if (result == CLOSE_CONNECTION)
        {
            printf("Close connection\n");
            close(sock_fd);
            return;
        }
        respond_to_http_client_message(sock_fd, http_msg);
        http_client_message_free(http_msg);
    }
    printf("done with connection %d\n", sock_fd);
    close(sock_fd);
}

int main(int argc, char *argv[])
{
    // Default port
    int port = DEFAULT_PORT;
    if (argc == 3 && strcmp(argv[1], "-p") == 0)
    {
        port = atoi(argv[2]);
        if (port <= 0 || port > 65535)
        {
            fprintf(stderr, "Invalid port! Input a port value between 0 - 65535\n");
            return 1;
        }
    }

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1)
    {
        perror("Socket creation fail\n");
        return 1;
    }

    // Setting Up the Server Address
    struct sockaddr_in socket_address;
    memset(&socket_address, '\0', sizeof(socket_address));
    socket_address.sin_family = AF_INET;
    socket_address.sin_addr.s_addr = htonl(INADDR_ANY);
    socket_address.sin_port = htons(port);

    // Binding and Listening
    int returnval;
    returnval = bind(socket_fd, (struct sockaddr *)&socket_address, sizeof(socket_address));
    if (returnval == -1)
    {
        perror("bind");
        return 1;
    }
    printf("Listening on port %d\n", port);
    returnval = listen(socket_fd, LISTEN_BACKLOG);

    // Accepting a Client Connection and Handling It
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);

    while (1)
    {
        pthread_t thread;
        int *client_fd_buf = malloc(sizeof(int));

        *client_fd_buf = accept(socket_fd, (struct sockaddr *)&client_address, &client_address_len);
        if(*client_fd_buf == -1){
            perror("accept");
            return 1;
        }

        printf("accepted connection on %d\n", *client_fd_buf);

        pthread_create(&thread, NULL, (void* (*) (void*))handle_connection, (void*)client_fd_buf);
    }
    return 0;
}


/*
1. different protocol -> HTTP instead of echo
2. implement the various routes (/static)
3. create html to return (inital / debug) -> "return content"
4. be graceful (errors, or at the end of connection)
*/