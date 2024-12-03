#include "socket.h"

int Socket::send_data(int p_number, uint8_t *reader, size_t send_size)
{
    if (newSD < 0)
    {
        init_socket(p_number);
    };

    if (send_size)
    {
        ret = send(newSD, reader + send_ofs, send_size - send_ofs, MSG_NOSIGNAL);
        if (ret > 0)
        {
            send_ofs += ret;
            if (send_size <= send_ofs)
            {
                send_size = 0;
                send_ofs = 0;
            }
            return 1;
        }
        else if (errno != EAGAIN)
        {
            // std::cout << "Send failed errno " << errno << std::endl;
            if (errno == EPIPE)
            {
                uninit_socket();
                init_socket(p_number);
            }
            return 0;
        }
    }
    else
    {
        send_ofs = 0;
    }
    return 0;
}

void Socket::init_socket(int p_number)
{
    sockaddr_in servAddr;
    bzero((char *)&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(p_number);
    ////////////////////////////////////////////////////////////////////
    serverSd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSd < 0)
    {
        // std::cerr << "Error establishing the server socket" << std::endl;
        return;
    }

    const int enable = 1;
    int addr_reuse = setsockopt(serverSd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
    if (addr_reuse < 0)
    {
        // std::cerr << "Error setting socket address reuse" << std::endl;
        return;
    }

    ////////////////////////////////////////////////////////////////////
    int bindStatus = bind(serverSd, (struct sockaddr *)&servAddr,
                          sizeof(servAddr));
    if (bindStatus < 0)
    {
        // std::cerr << "Error binding socket to local address" << std::endl;
        return;
    }
    if (listen(serverSd, 1) < 0)
    {
        perror("listen");
        close(serverSd);
        exit(EXIT_FAILURE);
    }
    sockaddr_in newSockAddr;
    socklen_t newSockAddrSize = sizeof(newSockAddr);
    ////////////////////////////////////////////////////////////////////

    newSD = accept(serverSd, (sockaddr *)&newSockAddr, &newSockAddrSize);
    if (newSD < 0)
    {
        // std::cerr << "Error accepting request from client!" << std::endl;
        return;
    }
    std::cout << "Connected with client!" << std::endl;

    int flags = fcntl(newSD, F_GETFL, 0);
    flags |= O_NONBLOCK;
    fcntl(newSD, F_SETFL, flags);
};

void Socket::uninit_socket()
{
    close(newSD);
    close(serverSd);
}