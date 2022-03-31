#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>


#include <unistd.h>
#include <cstdio>
#include <cstring>

#define PORT 5678
   
int main(int argc, const char* argv[])
{
    int sock = -1;
    if (-1 == (sock = socket(AF_INET, SOCK_STREAM, 0)))
    {
        printf("Socket creation error\n");
        return -1;
    }
    printf("Socket creation success.\n");

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) < 1)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }
    printf("will try to connect %s:%d\n", argv[1], PORT);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }
    printf("connect success.\n");

    const char* to_send = argv[2];
    printf("will send: %s.\n", to_send);
    send(sock, to_send, strlen(to_send), 0);
    printf("send success.\n");

    close(sock); 

    return 0;
}