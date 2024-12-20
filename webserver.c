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
#include "http_response.h"
#include "router.h"

#define DEFAULT_PORT 80
#define LISTEN_BACKLOG 5

int number_of_requests = 0;

void handle_connection(int *sock_fd_ptr)
{
    int sock_fd = *sock_fd_ptr;
    free(sock_fd_ptr);

    printf("handling connection on '%d'\n", sock_fd);
    while (1)
    {
        http_client_message_t* http_msg = read_http_client_message(sock_fd);
        if (http_msg == NULL) 
        {
            printf("Failed to read request\n");
            break;
        }
        number_of_requests++;

        print_http_client_message(http_msg);
        http_server_response_t* rsp = generate_response(http_msg);
        if (!rsp)
        {
            break;
        }
        
        bool send_was_successful = response_send(rsp, sock_fd);
        response_free(rsp);
        if (send_was_successful == false)
        {
            break;
        }
    }
    printf("done with connection %d\n", sock_fd);
    close(sock_fd);
}

int main(int argc, char *argv[])
{
    int port = DEFAULT_PORT;
    if (argc == 2 && !strcmp(argv[1], "--request"))
    {
        http_client_message_t* msg = read_http_client_message(0);
        exit(0);
    }
    if (argc == 2 && !strcmp(argv[1], "--handle"))
    {
        printf("Handling ONE connection from stdin\n");
        int *sock_fd = malloc(sizeof(int));
        *sock_fd = 0;
        handle_connection(sock_fd);
        printf("Done handling connection\n");
        exit(0);
    }

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
        if (*client_fd_buf == -1)
        {
            perror("accept");
            return 1;
        }

        printf("accepted connection on %d\n", *client_fd_buf);

        pthread_create(&thread, NULL, (void *(*)(void *))handle_connection, (void *)client_fd_buf);
    }
    return 0;
}
