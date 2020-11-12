#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <unistd.h>

#include "client.h"

#define PORT 5678

rpc_client::rpc_client()
    : to_server_socket_(-1)
{}

rpc_client::~rpc_client()
{
    if (to_server_socket_ > 0)
        close(to_server_socket_);
}

void
rpc_client::build_connection(std::string server_addr)
{
    try
    {
        int sock = -1;
        if (-1 == (sock = socket(AF_INET, SOCK_STREAM, 0)))
        {
            throw std::runtime_error("socket creation error");
        }

        struct sockaddr_in serv_addr;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(PORT);

        if(inet_pton(AF_INET, server_addr.c_str(), &serv_addr.sin_addr) < 1)
        {
            throw std::runtime_error("Invalid address / Address not supported");
        }

        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            throw std::runtime_error("Connection Failed");
        }
        std::swap(to_server_socket_, sock);
        if (sock > 0)
            close(sock);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}

void
rpc_client::reset_connection()
{
    if (to_server_socket_ > 0)
    {
        close(to_server_socket_);
        to_server_socket_ = -1;
    }
}

int
rpc_client::send_message(std::string msg)
{
    return send(to_server_socket_, msg.c_str(), msg.length(), 0);
}