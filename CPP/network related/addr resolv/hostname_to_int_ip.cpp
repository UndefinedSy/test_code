#include <netdb.h>
#include <stdlib.h>

int resolve_hostname(const char* hostname, uint32_t* ip_out, uint16_t* port_out)
{
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;

    struct addrinfo *result;
    int ret = getaddrinfo(hostname, NULL, &hints, &result);
    if (ret != 0) {
        return ret;
    }

    struct sockaddr* temp_addr = NULL;
    for (auto rp = result; rp != NULL; rp = rp->ai_next) {
        temp_addr = rp->ai_addr;
        break;
    }

    freeaddrinfo(result);

    *ip_out = ((sockaddr_in*)temp_addr)->sin_addr.s_addr;
    *port_out = ((sockaddr_in*)temp_addr)->sin_port;

    return 0;
}