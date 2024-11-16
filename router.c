#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "router.h"

http_response_t* handle_calc(http_client_message_t* msg);
http_response_t* handle_static(http_client_message_t* msg);
http_response_t* handle_stats(http_client_message_t* msg);



typedef http_response_t* (*Generator)(http_client_message_t* msg);


typedef struct route {
    char* method;
    char* path;
    bool fullpath;
    Generator function;
} Route;

Route handle_routes[] = {
    {"GET", "/calc/", false, handle_calc},
    {"GET", "/stats", true, handle_stats}, 
    {"GET", "/static/", false, handle_static}
    };

int num_routes = 3;

http_response_t* generate_response(http_client_message_t* msg)
{
    for (int ix = 0; ix < num_routes; ix++)
    {
        Route* potentialroute = &handle_routes[ix]; //compare
        if (strcmp(potentialroute->method, msg->method))
        {
            continue;
        }
        if (strcmp(potentialroute->path, msg->path) && potentialroute->fullpath == false)
        {
            continue;
        }
        if (strncmp(potentialroute->path, msg->path, strlen(potentialroute->path)))
        {
            continue;
        }

        // “/calc/4/5” ==== “/calc/
        // potential route is proper route and handler

        return (*potentialroute->function)(msg);
    }
    
    return NULL; // create and return a 404 response.
}

http_response_t* handle_calc(http_client_message_t* msg){
    printf("Idk how to generate a calc response\n");
    return NULL;
}

http_response_t* handle_static(http_client_message_t* msg){
    printf("Idk how to generate a static response\n");
    return NULL;
}

http_response_t* handle_stats(http_client_message_t* msg){
    printf("Idk how to generate a stats response\n");
    return NULL;
}



