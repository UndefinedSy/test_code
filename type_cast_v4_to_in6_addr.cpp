#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <errno.h>
#include <functional>
#include <iostream>
#include <memory>
#include <stdlib.h>

void in6_from_addr_v4mapped(const in_addr_t in, struct in6_addr* in6)
{
    *(u_int32_t *)&((in6)->s6_addr[0]) = 0;
    *(u_int32_t *)&((in6)->s6_addr[4]) = 0;
    *(u_int32_t *)&((in6)->s6_addr[8]) = htonl(0x0000FFFF);
    *(u_int32_t *)&((in6)->s6_addr[12]) = in;
}

void in6_to_addr_v4mapped(struct in6_addr* in6)
{
    return *(const u_int32_t *)&((in6)->s6_addr[12]);
}

void in6_from_sockadd(sockaddr* sa, in6_addr* in6)
{
    if (((const struct sockaddr *)(sa))->sa_family == AF_INET)
    {
	    const struct sockaddr_in *sin = (const struct sockaddr_in *)(sa);
	    in6_from_addr_v4mapped(sin->sin_addr.s_addr, in6);
    }
    else if (((const struct sockaddr *)(sa))->sa_family == AF_INET6)
    {
	    const struct sockaddr_in6 *sin6 = (const struct sockaddr_in6 *)(sa);
	    memcpy((in6), &sin6->sin6_addr, sizeof(sin6->sin6_addr));
    }
    else
    {
	    memset((in6), 0, sizeof(*(in6)));
    }
}

void in6_to_socaddr(struct in6_addr* in6, struct sockaddr* sa)
{
    if (IN6_IS_ADDR_V4MAPPED(in6))
    {
	    struct sockaddr_in *sin = (struct sockaddr_in *)(sa);
	    sin->sin_family = AF_INET;
	    sin->sin_len = sizeof(*sin);
	    sin->sin_addr.s_addr = in6_to_addr_v4mapped(in6);
    }
    else
    {
	    struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)(sa);
	    sin6->sin6_family = AF_INET6;
	    sin6->sin6_len = sizeof(*sin6);
	    sin6->sin6_addr = *in6;
    }
}

#define IN6_TO_SOCKADDR(in6, sa) do { \
    if (IN6_IS_ADDR_V4MAPPED(in6)) { \
	    struct sockaddr_in *sin = (struct sockaddr_in *)(sa); \
	    sin->sin_family = AF_INET; \
	    sin->sin_len = sizeof(*sin); \
	    sin->sin_addr.s_addr = IN6_TO_ADDR_V4MAPPED(in6); \
    } else { \
	    struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *)(sa); \
	    sin6->sin6_family = AF_INET6; \
	    sin6->sin6_len = sizeof(*sin6); \
	    sin6->sin6_addr = *in6; \
    } \
} while (0);

std::unique_ptr<char, std::function<void(void*)>>
AddressStringForIPv6Address(in6_addr *addr)
{
    const int addrlength = 64;
    auto result = std::unique_ptr<char, std::function<void(void*)>>(
                        reinterpret_cast<char*>(malloc(addrlength)),
                        free);
    if (result == NULL)
    {
        std::cout << "Failed to allocate memory" << std::endl;
        return NULL;
    }

    if (IN6_IS_ADDR_V4MAPPED(addr))
    {
        std::cout << "IN6_IS_ADDR_V4MAPPED" << std::endl;
        if (inet_ntop(AF_INET, &addr->s6_addr[12], result.get(), addrlength) == NULL)
        {
            std::cout << "ntop error formatting IPv4 address" << std::endl;
            return NULL;
        }

        std::cout << "IPv4 address from client is: " << result.get() << std::endl;
    }
    else
    {
        std::cout << "this is an ipv6 address" << std::endl;
        if (inet_ntop(AF_INET6, addr, result.get(), addrlength) == NULL)
        {
            std::cout << "ntop error formatting IPv6 address" << std::endl;
            return NULL;
        }

        std::cout << "IPv6 address from client is: " << result.get() << std::endl;
    }

    return result;
}


int main(int argc, char* argv[])
{
    struct sockaddr_in pDest4 = {0};

    pDest4.sin_family = AF_INET;
    pDest4.sin_port = htons(43160);
    pDest4.sin_addr.s_addr = htonl(168288089);
    pDest4.sin_len = sizeof(pDest4);

    struct in6_addr in6 = {0};

    in6_from_sockadd((struct sockaddr*)&pDest4, &in6);

    auto result = AddressStringForIPv6Address(&in6);
    std::cout << "The result is: " << result.get() << std::endl;

}
