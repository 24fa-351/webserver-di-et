#ifndef ROUTER_H
#define ROUTER_H

#include "http_response.h"
#include "http_message.h"

http_server_response_t* generate_response(http_client_message_t* msg);

#endif