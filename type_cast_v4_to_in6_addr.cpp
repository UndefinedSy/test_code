#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <errno.h>
#include <stdlib.h>

char*
AddressStringForIPv6Address(in6_addr *addr)
{
    const int addrlength = 64;
    char* result = NULL;

    result = (char *) malloc(addrlength);
    if (result == NULL)
    {
        std::cout << "Failed to allocate memory" << std::endl;
        return NULL;
    }
    if (IN6_IS_ADDR_V4MAPPED(addr))
    {
        std::cout << "IN6_IS_ADDR_V4MAPPED" << std::endl;
        if (inet_ntop(AF_INET, &addr->s6_addr[12], result, addrlength) == NULL)
        {
            std::cout << "ntop error formatting IPv4 address" << std::endl;
            return NULL;
        }

        std::cout << "IPv4 address from client is: " << result << std::endl;
    }
    else
    {
        std::cout << "this is an ipv6 address" << std::endl;
        if (inet_ntop(AF_INET6, addr, result, addrlength) == NULL)
        {
            std::cout << "ntop error formatting IPv6 address" << std::endl;
            return NULL;
        }

        std::cout << "IPv6 address from client is: " << result << std::endl;
    }

    return result;
}

char*
QueryNativeTokenForAddressString(struct in6_addr* arg)
{
    char* result = NULL;

    result = AddressStringForIPv6Address(arg);

cleanup:
    return result;
}

int main(int argc, char* argv[])
{
    struct in6_addr  arg;
    struct sockaddr* pDest = (struct sockaddr*)&arg;
    struct sockaddr_in* pDest4 = (struct sockaddr_in*) pDest;

    pDest4->sin_family = AF_INET;
    pDest4->sin_port = htons(43160);
    pDest4->sin_addr.s_addr = htonl(168288089);
    pDest4->sin_len = sizeof(*pDest4);

    char* pClientAddr = QueryNativeTokenForAddressString(&arg);
    std::cout << "The result is: " << pClientAddr << std::endl;
}
