#include <iostream>
#include <memory>
#include <stdexcept>

#include "client.h"


int main(int argc, const char* argv[])
{
    try
    {
        rpc_client rpc_client_instance;
        rpc_client_instance.build_connection(argv[1]);
        if (rpc_client_instance.send_message(argv[2]) == -1)
        {
            std::cout << "send message \"" << argv[2] << "\"" << std::endl;
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}