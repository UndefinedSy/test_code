#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <string>
#include <iostream>

int
main()
{
    int domain = AF_INET;
    std::string ip_str = "10.28.34.2:33071";
    
    int port_start = ip_str.find(':');
    if (port_start == std::string::npos)
        std::cout << "port part not found!\n";
    
    auto port = std::stoi(ip_str.substr(port_start+1));
    auto ip = ip_str.substr(0, port_start);

    uint32_t buf;
    int s = inet_pton(domain, ip.c_str(), &buf);    // exception
    if (s <= 0) {
        if (s == 0)
            fprintf(stderr, "Not in presentation format");
        else
            perror("inet_pton");
        exit(EXIT_FAILURE);
    }

    char str[INET_ADDRSTRLEN];
    if (inet_ntop(domain, &buf, str, INET_ADDRSTRLEN) == NULL) {
        perror("inet_ntop");
        exit(EXIT_FAILURE);
    }
    printf("%s:%u\n", str, port);

    exit(EXIT_SUCCESS);
}