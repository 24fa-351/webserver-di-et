#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

#include "http_message.h"

bool read_request_line(http_client_message_t *msg, int sock_fd);
bool read_headers(http_client_message_t *msg, int sock_fd);
bool read_body(http_client_message_t *msg, int sock_fd);
char* get_header_value(http_client_message_t* msg, char* key);

http_client_message_t *read_http_client_message(int sock_fd)
{
    printf("http_client_message_t sock_fd '%d'\n", sock_fd);

    http_client_message_t *msg = malloc(sizeof(http_client_message_t));

    if (read_request_line(msg, sock_fd) == false)
    {
        printf("Failed to read request line(http_message.c, 1st if statement)\n");
        free_http_client_message(msg);
        return NULL;
    }

    // read_headers also eats the second CRLF
    if (read_headers(msg, sock_fd) == false)
    {
        printf("Failed to read headers(http_message.c, 2nd if statement)\n");
        free_http_client_message(msg);
        return NULL;
    }

    if (read_body(msg, sock_fd) == false)
    {
        printf("Failed to read body(http_message.c, 3rd if statement)\n");
        free_http_client_message(msg);
        // free(msg);
        return NULL;
    }
    return msg;
}

void print_http_client_message(http_client_message_t *msg)
{
    printf("-~-~-Client_Message_TOP-~-~\n");
    if (msg->method)
    {
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
            printf("Header %d: %s = '%s'\n", ix, msg->headers[ix].key, msg->headers[ix].value);
        }
    }
    printf("-~-~-Client_Message_BOTTOM-~-~-\n");
}

#define FREE_IF_NOT_NULL(ptr) \
    if (ptr)                  \
    {                         \
        free(ptr);            \
    }
void free_http_client_message(http_client_message_t *msg)
{
    printf("free_http_client_message\n");
    if (msg == NULL)
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

char *read_line(int sock_fd)
{
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
    printf(__FILE__ ": %d read line: '%s'\n", __LINE__ , line);
    return line;
}

char* get_header_value(http_client_message_t* msg, char* key)
{
    for (int ix = 0; ix < msg->header_count; ix++)
    {
        if (strcmp(msg->headers[ix].key, key) == 0)
        {
            return msg->headers[ix].value;
        }
    }
    return NULL;
}

bool read_request_line(http_client_message_t *msg, int sock_fd)
{
    printf("read request line\n");

    char *line = read_line(sock_fd);
    while(1)
    {
        if (line == NULL)
        {
            return false;
        }
        if (strlen(line) > 0)
        {
            break;
        }
        free(line);
    }
    msg->method = malloc(strlen(line) + 1);
    msg->path = malloc(strlen(line) + 1);
    msg->http_version = malloc(strlen(line) + 1);

    int length_parsed;
    int number_parsed;
    number_parsed = sscanf(line, "%s %s %s%n", msg->method, msg->path, msg->http_version, &length_parsed);



    if (number_parsed != 3 || length_parsed != strlen(line))
    {
        printf("%d parsed, %d length parsed, %lu line len\n", number_parsed, length_parsed, strlen(line));
        printf("Failed to parse request line(http_message.c)\n");
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

bool read_headers(http_client_message_t *msg, int sock_fd)
{
    msg->headers = malloc(sizeof(Header) * 100);
    msg->header_count = 0;
    while (1)
    {
        char *line = read_line(sock_fd);
        if (line == NULL)
        {
            // close connection or error
            return false;
        }
        if (strlen(line) == 0)
        {
            free(line);
            break;
        }
        
        msg->headers[msg->header_count].key = malloc(10000);
        msg->headers[msg->header_count].value = malloc(10000);

        int number_parsed;
        int length_parsed;

        number_parsed = sscanf(line, "%s %s %n", msg->headers[msg->header_count].key, msg->headers[msg->header_count].value, &length_parsed);
        if (number_parsed != 2 || length_parsed != strlen(line))
        {
            printf("Failed to parse header(read_headers();)\n");
            free(line);
            return false;
        }

        if (msg->headers[msg->header_count].key[strlen(msg->headers[msg->header_count].key) - 1] == ':')
        {
            msg->headers[msg->header_count].key[strlen(msg->headers[msg->header_count].key) - 1] = '\0';
        }

        printf("key: '%s' '%s' \n", msg->headers[msg->header_count].key, msg->headers[msg->header_count].value);

        msg->headers[msg->header_count].key = realloc(msg->headers[msg->header_count].key, strlen(msg->headers[msg->header_count].key) + 1);
        msg->headers[msg->header_count].value = realloc(msg->headers[msg->header_count].value, strlen(msg->headers[msg->header_count].value) + 1);

        msg->header_count++;
        free(line);
    }
    return true;
}

bool read_body(http_client_message_t *msg, int sock_fd) //
{
    printf("read_body\n");

    msg->body = NULL;
    msg->body_length = 0;

    char *body_length_str = get_header_value(msg, "Content-Length");
    printf("content_length_str: '%s'\n", body_length_str);

    if (body_length_str == NULL)
    {
        return true;
    }

    if(sscanf(body_length_str, "%d", &msg->body_length) != 1)
    {
        printf("Failed to parse body length\n");
        return false;
    }
    if(msg->body_length == 0)
    {
        printf("no body\n");
        return true;
    }
    msg->body = malloc(msg->body_length + 1);
    int bytes_read = read(sock_fd, msg->body, msg->body_length);
    if (bytes_read != msg->body_length)
    {
        printf("Failed to read body\n");
        free(msg->body);
        msg->body = NULL;
        msg->body_length = 0;
        return false;
    }
    msg->body[msg->body_length] = '\0';
    return true;
}
