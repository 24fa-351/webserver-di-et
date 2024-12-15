#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "router.h"

http_server_response_t *handle_calc(http_client_message_t *msg);
http_server_response_t *handle_static(http_client_message_t *msg);
http_server_response_t *handle_stats(http_client_message_t *msg);

typedef http_server_response_t *(*Generator)(http_client_message_t *msg);

typedef struct route
{
    char *method;
    char *path;
    bool fullpath;
    Generator function;
} Route;

Route handle_routes[] ={{"GET", "/calc/", false, handle_calc},
                        {"GET", "/stats", true, handle_stats},
                        {"GET", "/static/", false, handle_static}};

int num_routes = 3;

http_server_response_t *generate_response(http_client_message_t *msg)
{
    for (int ix = 0; ix < num_routes; ix++)
    {
        Route *potentialroute = &handle_routes[ix]; // compare

        // if the method doesnt match
        if (strcmp(potentialroute->method, msg->method))
        {
            continue;
        }
        // if its a patrial path but the parital path doesnt match.
        if (potentialroute->fullpath == false && strncmp(potentialroute->path, msg->path, strlen(potentialroute->path)))
        {
            continue;
        }
        // if its a full path but the full path doesnt match.
        if (potentialroute->fullpath == true && strcmp(potentialroute->path, msg->path))
        {
            continue;
        }
        return (*potentialroute->function)(msg);
    }
    return NULL; // create and return a 404 response.
}

http_server_response_t *handle_calc(http_client_message_t *msg)
{
    int num1;
    int num2;
    char operator;
    if (sscanf(msg->path, "/calc/%d%c%d", &num1, &operator, & num2) != 3)
    {
        printf("didn't match %s\n", msg->path);
        return NULL;
    }
    int result;
    switch (operator)
    {
    case '+':
        result = num1 + num2;
        break;
    case '-':
        result = num1 - num2;
        break;
    case '*':
        result = num1 * num2;
        break;
    case '/':
        if (num2 == 0)
        {
            printf("Divide by zero\n");
            return NULL;
        }
        result = num1 / num2;
        break;
    default:
        printf("Invalid operator '%c'\n", operator);
        return NULL;
    }
    char result_str[100];
    snprintf(result_str, sizeof(result_str), "{ \"result\": %d }\n", result);
    http_server_response_t *response = (http_server_response_t *)malloc(sizeof(http_server_response_t));

    response->body = strdup(result_str);
    response->body_length = strlen(result_str);
    response->status_code = 200;
    response->http_version = strdup("HTTP/1.1");
    return response;
}

http_server_response_t *handle_static(http_client_message_t *msg)
{
    char file_path[1000];
    snprintf(file_path, sizeof(file_path), ".%s", msg->path);

    struct stat status_buffer;
    int status;

    status = stat(file_path, &status_buffer);

    if (status != 0)
    {
        // file not found
        perror("stat");
        return NULL;
    }
    int fd = open(file_path, O_RDONLY);
    char *file_contents = (char *)malloc(status_buffer.st_size);
    size_t number_of_bytes_read = read(fd, file_contents, status_buffer.st_size);
    printf("path: %s\n", file_path);

    http_server_response_t *response = (http_server_response_t *)malloc(sizeof(http_server_response_t));

    response->body = file_contents;
    response->body_length = status_buffer.st_size;
    response->status_code = 200;
    response->http_version = strdup("HTTP/1.1");
    return response;
}

http_server_response_t *handle_stats(http_client_message_t *msg)
{
    // number of requests
    // total of received bytes
    // total of sent bytes
    extern int number_of_requests; 

    // extern int number_of_bytes_read;
    // extern int number_of_bytes_sent;

    // Number of requests
    char *buffer = (char *)malloc(1000);
    snprintf(buffer, 1000, "number_of_requests: %d\n", number_of_requests);

    // Number of bytes read
    

    // Number of bytes sent
    snprintf(buffer + strlen(buffer), 1000 - strlen(buffer), "number_of_bytes_sent: %d\n", 0);




    http_server_response_t *response = (http_server_response_t *)malloc(sizeof(http_server_response_t));
    response->body = buffer;
    response->body_length = strlen(buffer);
    response->status_code = 200;
    response->http_version = strdup("HTTP/1.1");
    return response;
}
