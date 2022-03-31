#include <netinet/in.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <errno.h>
#include <iostream>
#include <unistd.h>

#define PORT 5678

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
    std::cout << "setsockopt success" << opt << std::endl;

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

    struct kevent change_event;
    EV_SET(&change_event,
           server_fd,
           EVFILT_READ,
           EV_ADD | EV_ENABLE,
           /*fflags*/0,
           /*data*/0,
           /*udata*/NULL);
    
    // Register kevent with the kqueue.
    int kq = kqueue();
    if (-1 == kevent(kq, &change_event, 1, NULL, 0, NULL))
    {
        perror("kevent");
        return(-1);
    }
    std::cout << "kevent register success" << std::endl;

    struct kevent events[5];
    for (;;)
    {
        int new_events = kevent(kq, NULL, 0, events, 5, NULL);
        std::cout << "kevent wakeup with new_events " << new_events << std::endl;
        if (new_events == -1)
        {
            std::cout << "new_events is -1 with errno: " << errno << std::endl;
            switch (errno)
            {
                case EINTR:
                    /* retry */
                    break;
                default:
                    perror("kevent");
                    return(-1);
            }
        }

        for (int i = 0; i < new_events; ++i)
        {
            int event_fd = events[i].ident;

            // if (events[i].flags & EV_EOF)
            // {
            //     printf("Client has disconnected.\n");
            //     close(event_fd);
            // }
            // else if (event_fd == server_fd)
            if (event_fd == server_fd)
            {
                int socket_connection_fd = accept(event_fd, NULL, NULL); // Is client_addr helpful?
                if (-1 == socket_connection_fd)
                {
                    perror("Accept socket error");
                    return(-1); // should return or not?
                }
                std::cout << "accept socket success " << socket_connection_fd << std::endl;

                EV_SET(&change_event,
                       socket_connection_fd,
                       EVFILT_READ,
                       EV_ADD,
                       /*fflags*/0,
                       /*data*/0,
                       /*udata*/NULL);
                if (kevent(kq, &change_event, 1, NULL, 0, NULL) < 0)
                {
                    perror("kevent error");
                    return(-1); // should return or not?
                }
                std::cout << "Register new socket fd success" << std::endl;
            }
            
            else if (events[i].filter & EVFILT_READ)
            {
                char buf[1024]; // wierd, the address of this char* will be reuse...
                std::cout << "will read from socket." << std::endl;
                // Read bytes from socket
                size_t bytes_read = recv(event_fd, buf, sizeof(buf), 0);
                if (bytes_read == 0)
                {
                    printf("bytes_read == 0, Client has disconnected.\n");
                    close(event_fd);
                    continue;
                }
                printf("read %zu bytes: %s\n", bytes_read, buf);
            }
        }
    }
    return 0;
}