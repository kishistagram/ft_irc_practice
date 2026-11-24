#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

const char *get_tcp_state(int state)
{
    switch (state)
    {
        case TCP_CLOSE:
            return "TCP_CLOSE";
        case TCP_LISTEN:
            return "TCP_LISTEN";
        case TCP_ESTABLISHED:
            return "TCP_ESTABLISHED";
        case TCP_SYN_SENT:
            return "TCP_SYN_SENT";
        case TCP_SYN_RECV:
            return "TCP_SYN_RECV";
        case TCP_CLOSE_WAIT:
            return "TCP_CLOSE_WAIT";
        default:
            return "OTHER";
    }
}

void print_socket_info(int fd)
{
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    char ip[INET_ADDRSTRLEN];

    int accepting = 0;
    socklen_t optlen = sizeof(accepting);

    struct tcp_info info;
    socklen_t infolen = sizeof(info);

    memset(&addr, 0, sizeof(addr));
    memset(&info, 0, sizeof(info));

    // Get local address
    if (getsockname(fd, (struct sockaddr *)&addr,
                    &addrlen) == -1)
    {
        perror("getsockname");
        return;
    }

    // Check whether the socket is listening
    if (getsockopt(fd, SOL_SOCKET, SO_ACCEPTCONN,
                    &accepting, &optlen) == -1)
    {
        perror("getsockopt SO_ACCEPTCONN");
        return;
    }

    // Get TCP state
    if (getsockopt(fd, IPPROTO_TCP, TCP_INFO,
                    &info, &infolen) == -1)
    {
        perror("getsockopt TCP_INFO");
        return;
    }

    if (inet_ntop(AF_INET, &addr.sin_addr,
                  ip, sizeof(ip)) == NULL)
    {
        perror("inet_ntop");
        return;
    }

    printf("fd            : %d\n", fd);
    printf("IP            : %s\n", ip);
    printf("Port          : %u\n",
           (unsigned int)ntohs(addr.sin_port));
    printf("SO_ACCEPTCONN : %d\n", accepting);
    printf("TCP State     : %s (%d)\n",
           get_tcp_state(info.tcpi_state),
           info.tcpi_state);
}

int main(void)
{
    // STEP 1: socket()
    int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (fd == -1)
    {
        perror("socket");
        return 1;
    }

    printf("\n===== AFTER SOCKET =====\n");
    print_socket_info(fd);

    int yes = 1;

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
                   &yes, sizeof(yes)) == -1)
    {
        perror("setsockopt");
        close(fd);
        return 1;
    }

    // STEP 2: bind()
    struct sockaddr_in address;

    memset(&address, 0, sizeof(address));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = htons(6667);

    if (bind(fd, (struct sockaddr *)&address,
             sizeof(address)) == -1)
    {
        perror("bind");
        close(fd);
        return 1;
    }

    printf("\n===== AFTER BIND =====\n");
    print_socket_info(fd);

    printf("\nPress ENTER to execute listen()\n");
    getchar();

    // STEP 3: listen()
    if (listen(fd, 5) == -1)
    {
        perror("listen");
        close(fd);
        return 1;
    }

    printf("\n===== AFTER LISTEN =====\n");
    print_socket_info(fd);

    printf("\nConnect clients using nc.\n");
    printf("Press ENTER to execute accept()\n");
    getchar();

    // STEP 4: accept()
    int client_fd = accept(fd, NULL, NULL);

    if (client_fd == -1)
    {
        perror("accept");
        close(fd);
        return 1;
    }

    printf("\n===== AFTER ACCEPT =====\n");

    printf("\n[Listening Socket]\n");
    print_socket_info(fd);

    printf("\n[Connected Socket]\n");
    print_socket_info(client_fd);

    printf("\nPress ENTER to close sockets\n");
    getchar();

    close(client_fd);
    close(fd);

    return 0;
}