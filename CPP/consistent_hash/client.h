#ifndef _RPC_CLIENT_H_
#define _RPC_CLIENT_H_

#include <string>

int build_connection(std::string server_addr);

class rpc_client
{
private:
    int to_server_socket_;

public:
    rpc_client();
    ~rpc_client();

    void build_connection(std::string server_addr);
    void reset_connection();
    int send_message(std::string msg);
};

#endif