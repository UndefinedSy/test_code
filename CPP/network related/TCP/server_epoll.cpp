#include <netinet/in.h>
#include <sys/epoll.h> 
#include <sys/socket.h>
#include <sys/types.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>

#define PORT 5678
#define MAX_EVENTS 5

int main()
{
    int server_fd = -1;
    if (-1 == (server_fd = socket(AF_INET, SOCK_STREAM, 0)))
    {
        perror("socket failed");
        return(-1);
    }
    std::cout << "socket create success - " << server_fd << std::endl;

    int opt = 1;
    if (-1 == setsockopt(server_fd, 
                         SOL_SOCKET,
                         SO_REUSEADDR,
                         &opt, sizeof(opt)))
    {
        perror("setsockopt");
        return(-1);
    }
    std::cout << "setsockopt success " << opt << std::endl;

    // Forcefully attaching socket to the port 8080
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);
    if (-1 == bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)))
    {
        perror("bind failed");
        return(-1);
    }
    std::cout << "bind success" << std::endl;

    if (listen(server_fd, /*should be configure-able*/5) < 0)
    {
        perror("listen");
        return(-1);
    }
    std::cout << "listen success" << std::endl;

    // Create epoll fd
    int epoll_fd = epoll_create1(0);
    if (-1 == epoll_fd)
    {
        perror("epoll_fd");
        return(-1);
    }
    std::cout << "epoll_fd create success" << std::endl;

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = server_fd;

    if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, 0, &ev) == -1)
    {
        perror("epoll_ctl: listen_sock");
        close(epoll_fd);
        return -1;
    }
    std::cout << "add server socket fd to the epool." << std::endl;

    struct epoll_event events[MAX_EVENTS];

    for (;;)
    {
        int new_events = epoll_wait(epoll_fd, events, MAX_EVENTS, 3600);
        if(new_events == -1)
		{
			perror("epoll_wait");
			break;
		}
        std::cout << "epoll_wait returns " << new_events << " events." << std::endl;

        for (int i = 0; i < new_events; ++i)
        {
            if (events[i].data.fd == server_fd)
            {
                int socket_connection_fd = accept(server_fd, NULL, NULL); // Is client_addr helpful?
                if (-1 == socket_connection_fd)
                {
                    perror("Accept socket error");
                    return(-1);
                }
                std::cout << "accept socket success " << socket_connection_fd << std::endl;

                fcntl(socket_connection_fd, F_SETFL, O_NONBLOCK);
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = socket_connection_fd;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_connection_fd, &ev) == -1)
                {
                    perror("epoll_ctl: socket_connection_fd");
                    return(-1);
                }
                std::cout << "Register new socket fd success" << std::endl;
            }
            else // if (events[i].events & EPOLLIN)
            {
                char* buf = (char*)calloc(1024, sizeof(char));
                int buflen;
                int tot_length = 0;
                std::cout << "will read from socket." << std::endl;
                // Read bytes from socket

                bool rs = 1;
                while(rs)
                {
                    buflen = recv(events[i].data.fd, buf, sizeof(buf), 0);
                    if(buflen < 0)
                    {
                        if(errno == EAGAIN)
                            break;
                        else
                        {
                            perror("recv: event_fd");
                            return -1;
                        }
                    }
                    else if(buflen == 0)
                    {
                        std::cout << "bytes_read == 0, Client has disconnected." << std::endl;
                        close(events[i].data.fd);
                        break;
                    }

                    tot_length += buflen;

                    if(buflen == sizeof(buf))
                    {
                        
                        std::cout << "full buf, need to read the incoming fd again. The content of the buffer is: " << buf << std::endl;
                    }
                    else
                        rs = 0;
                }

                printf("read %zu bytes: %s\n", tot_length, buf);
                std::cout << std::flush;
                free(buf);
            }
        }
    }
    return 0;
}