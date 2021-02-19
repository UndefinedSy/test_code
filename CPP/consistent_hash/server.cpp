#include <netinet/in.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/types.h>


#include <errno.h>
#include <iostream>
#include <stdexcept>
#include <unistd.h>

#include "server.h"

#define PORT 5678

void setup_tcp_server()
{
    try
    {
        int server_fd = -1;
        if (-1 == (server_fd = socket(AF_INET, SOCK_STREAM, 0)))
        {
            throw std::runtime_error("socket failed");
        }

        int opt = 1;
        if (-1 == setsockopt(server_fd, 
                            SOL_SOCKET,
                            SO_REUSEADDR,
                            &opt, sizeof(opt)))
        {
            throw std::runtime_error("setsockopt");
        }

        // Forcefully attaching socket to the port 8080
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(PORT);
        if (-1 == bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)))
        {
            throw std::runtime_error("bind failed");
        }

        if (listen(server_fd, /*should be configure-able*/5) < 0)
        {
            throw std::runtime_error("listen");
        }

        struct kevent change_event;
        EV_SET(&change_event,
            server_fd,
            EVFILT_READ,
            EV_ADD | EV_ENABLE,
            /*fflags*/0,
            /*data*/0,
            /*udata*/NULL);

        int kq = kqueue();
        if (-1 == kevent(kq, &change_event, 1, NULL, 0, NULL))
        {
            throw std::runtime_error("Register kevent with the kqueue");
        }

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
                    case EINTR: /* retry */
                        break;
                    default:
                        throw std::runtime_error("kevent");
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
                        throw std::runtime_error("Accept socket error");
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
                        throw std::runtime_error("Register connection_fd failed.");
                    }
                    std::cout << "Register new socket fd success" << std::endl;
                }
                else if (events[i].filter & EVFILT_READ)
                {
                    char buf[1024]; // wierd, the address of this char* will be reuse...
                    // Read bytes from socket
                    size_t bytes_read = recv(event_fd, buf, sizeof(buf), 0);
                    if (bytes_read == 0)
                    {
                        std::cout << "bytes_read == 0, Client has disconnected." << std::endl;
                        close(event_fd);
                        continue;
                    }
                    std::cout << "read " << bytes_read << " content is: " << buf << std::endl;
                }
            }
        }
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
}