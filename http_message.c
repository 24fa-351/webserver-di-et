#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#include "http_message.h"

bool read_request_line(http_client_message_t*msg, int sock_fd);
bool read_headers(http_client_message_t* msg, int sock_fd);
bool read_body(http_client_message_t* msg, int sock_fd);

http_client_message_t *read_http_client_message(int sock_fd)
{
    printf("Reading request from sock_fd %d \n", sock_fd);

    http_client_message_t *msg = malloc(sizeof(http_client_message_t));

    if (read_request_line(msg, sock_fd) == false)
    {
        printf("Failed to read request line\n");
        free_http_client_message(msg);
        return NULL;
    }
    // read_headers also eats the second CRLF
    if (read_headers(msg, sock_fd) == false)
    {
        printf("Failed to read headers\n");
        free_http_client_message(msg);
        return NULL;
    }
    if (read_body(msg, sock_fd) == false)
    {
        printf("Failed to read body\n");
        free(msg);
        return NULL;
    }
    return msg;
}

void print_http_client_message(http_client_message_t *msg)
{
    printf("-~-~-~-~-~-~-~-~-~Client_Message-~-~-~-~-~-~-~-~-~\n");

    if (msg->method)
    {
        printf("||did I make it to this marker?||\n");
        printf("method: %s\n", msg->method);
    }

    if (msg->path)
    {
        printf("path: %s\n", msg->path);
    }
    if (msg->http_version)
    {
        printf("http version: %s\n", msg->http_version);
    }
    if (msg->header_count)
    {
        for (int ix = 0; ix < msg->header_count; ix++)
        {
            printf("Header %d: %s: %s", ix, msg->headers[ix].key, msg->headers[ix].value);
        }
    }
    printf("-~-~-~-~-~-~-~-~-~Client_Message-~-~-~-~-~-~-~-~-~\n");
}

#define FREE_IF_NOT_NULL(ptr) if (ptr) {free(ptr);}
void free_http_client_message(http_client_message_t *msg)
{
    printf("freeing request\n");
    if(msg == NULL)
    {
        return;
    }
    FREE_IF_NOT_NULL(msg->method)
    FREE_IF_NOT_NULL(msg->path)
    FREE_IF_NOT_NULL(msg->http_version)

    for (int ix = 0; ix < msg->header_count; ix++)
    {
        FREE_IF_NOT_NULL(msg->headers[ix].key);
        FREE_IF_NOT_NULL(msg->headers[ix].value);
    }
    FREE_IF_NOT_NULL(msg->headers);

    free(msg);
}

char* read_line(int sock_fd)
{
    printf("Reading from sock_fd %d\n", sock_fd);
    char *line = malloc(10000);
    int len_read = 0;
    while (1)
    {
        char ch;
        int number_bytes_read = read(sock_fd, &ch, 1);
        if (number_bytes_read <= 0)
        {
            return NULL;
        }
        if (ch == '\n')
        {
            break;
        }
        line[len_read] = ch;
        len_read++;
        line[len_read] = '\0';
    }
    if (len_read > 0 && line[len_read - 1] == '\r')
    {
        line[len_read - 1] = '\0';
    }
    line = realloc(line, len_read + 1);

    return line;
}
bool read_request_line(http_client_message_t *msg, int sock_fd)
{
    printf("Reading request line\n");
    char *line = read_line(sock_fd);
    if (line == NULL)
    {
        return false;
    }
    msg->method = malloc(strlen(line) + 1);
    msg->path = malloc(strlen(line) + 1);
    msg->http_version = malloc(strlen(line) + 1);

    int length_parsed;
    int number_parsed;
    number_parsed = sscanf(line, "%s %s %s%n", msg->method, msg->path, msg->http_version, &length_parsed);

    if (number_parsed != 3 || length_parsed != strlen(line))
    {
        printf("Failed to parse request line\n");
        free(line);
        return false;
    }
    if (strcmp(msg->method, "GET") != 0 && strcmp(msg->method, "POST") != 0)
    {
        printf("Invalid method: %s\n", msg->method);
        free(line);
        return false;
    }

    return true;
}

bool read_headers(http_client_message_t* msg, int sock_fd)
{
    printf("Reading headers\n");
    msg->headers = malloc(sizeof(Header) * 100);
    msg->header_count = 0;
    while (1)
    {
        char* line = read_line(sock_fd);
        if(line == NULL)
        {
            // close connection or error
            return false;
        }
        if(strlen(line) == 0)
        {
            free(line);
            break;
        }
        msg->headers[msg->header_count].key = malloc(10000);
        msg->headers[msg->header_count].value = malloc(10000);
        int number_parsed;
        int length_parsed;
        number_parsed = sscanf(line, "%s: %s%n", msg->headers[msg->header_count].key, msg->headers[msg->header_count].value, &length_parsed);
        if (number_parsed != 2 || length_parsed != strlen(line))
        {
            printf("Failed to parse header\n");
            free(line);
            return false;
        }
        
        msg->headers[msg->header_count].key = realloc(msg->headers[msg->header_count].key, strlen(msg->headers[msg->header_count].key) + 1);
        msg->headers[msg->header_count].key = realloc(msg->headers[msg->header_count].value, strlen(msg->headers[msg->header_count].value) + 1);


        msg->header_count++;
        free(line);       
    
    }
    
    return true;
}

bool read_body(http_client_message_t* msg, int sock_fd)
{
    return true;
}

#if 0
void read_http_client_message(int sock_fd, http_client_message_t** msg, http_read_result_t* result)
{
    char buffer[1024];
    int bytes_read = read(sock_fd, buffer, sizeof(buffer));
    if (bytes_read == -1)
    {
        perror("read\n");
        return;
    }

    buffer[bytes_read] = '\0';
    printf("recieved %d bytes: %s\n", bytes_read, buffer);
    
    *msg = malloc(sizeof(http_client_message_t));
    if(*msg == NULL)
    {
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
#endif

/*
number_parsed = sscanf(line, "%s %s %s%n", req->method, req->path, req->version, &length_parsed);
req->method = malloc(strlen(line) + 1);
    req->path = malloc(strlen(line) + 1);
    req->version = malloc(strlen(line) + 1);
bool read_request_line(Request* req, int fd);
bool read_body(Request* req, int fd);

Request* request_read_from_fd(int fd) {
    printf("Reading request from fd %d\n", fd);

    Request* req = malloc(sizeof(Request));

    if (read_request_line(req, fd) == false) {
        printf("Failed to read request line\n");
        request_free(req);
        return NULL;
    }
    // read_headers also eats the second CRLF
    req->headers = read_headers(fd);

    if (req->headers == NULL) {
        printf("Failed to read headers\n");
        request_free(req);
        return NULL;
    }
    if (read_body(req, fd) == false) {
        printf("Failed to read body\n");
        request_free(req);
        return NULL;
    }

    return req;
}

int number_bytes_read = read(fd, &ch, 1);
        if (number_bytes_read <= 0) {
            return NULL;
        }
char* read_http_line(int fd)

if (ch == '\n') {
            break;
        }

*/