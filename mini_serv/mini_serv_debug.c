// mini_serv learning/debug version (not an exam submission)
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/ip.h>
#include <sys/select.h>

int count = 0;
int max_fd = 0;
int ids[65536];
char *msgs[65536];
fd_set rfds, wfds, afds;
char buf_read[1001], buf_write[42];

int extract_message(char **buf, char **msg)
{
    char *newbuf;
    int i;

    *msg = 0;
    if (*buf == 0)
        return (0);
    i = 0;
    while ((*buf)[i])
    {
        if ((*buf)[i] == '\n')
        {
            newbuf = calloc(1, sizeof(*newbuf) * (strlen(*buf + i + 1) + 1));
            if (newbuf == 0)
                return (-1);
            strcpy(newbuf, *buf + i + 1);
            *msg = *buf;
            (*msg)[i + 1] = 0;
            *buf = newbuf;
            return (1);
        }
        i++;
    }
    return (0);
}

char *str_join(char *buf, char *add)
{
    char *newbuf;
    int len;

    if (buf == 0)
        len = 0;
    else
        len = strlen(buf);
    newbuf = malloc(sizeof(*newbuf) * (len + strlen(add) + 1));
    if (newbuf == 0)
        return (0);
    newbuf[0] = 0;
    if (buf != 0)
        strcat(newbuf, buf);
    free(buf);
    strcat(newbuf, add);
    return (newbuf);
}

void fatal_error(void)
{
    write(2, "Fatal error\n", 12);
    exit(1);
}

// This is the original exercise's send behavior. It is not a complete
// solution for partial sends or slow-reading clients.
void notify_other(int author, char *str)
{
    for (int fd = 0; fd <= max_fd; fd++)
    {
        if (FD_ISSET(fd, &wfds) && fd != author)
        {
            int bytes = send(fd, str, strlen(str), 0);
            printf("[SEND] to_fd=%d, bytes=%d\n", fd, bytes);
        }
    }
}

void register_client(int fd)
{
    max_fd = fd > max_fd ? fd : max_fd;
    ids[fd] = count++;
    msgs[fd] = NULL;
    FD_SET(fd, &afds);
    printf("[REGISTER] fd=%d, client_id=%d\n", fd, ids[fd]);
    sprintf(buf_write, "server: client %d just arrived\n", ids[fd]);
    notify_other(fd, buf_write);
}

void remove_client(int fd)
{
    printf("[DISCONNECT] fd=%d, client_id=%d\n", fd, ids[fd]);
    sprintf(buf_write, "server: client %d just left\n", ids[fd]);
    notify_other(fd, buf_write);
    free(msgs[fd]);
    msgs[fd] = NULL;
    FD_CLR(fd, &afds);
    close(fd);
}

void send_msg(int fd)
{
    char *msg;
    int status;

    while ((status = extract_message(&(msgs[fd]), &msg)) == 1)
    {
        sprintf(buf_write, "client %d: ", ids[fd]);
        notify_other(fd, buf_write);
        notify_other(fd, msg);
        free(msg);
    }
    if (status < 0)
        fatal_error();
}

int create_socket(void)
{
    max_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (max_fd < 0 || max_fd >= FD_SETSIZE)
        fatal_error();
    FD_SET(max_fd, &afds);
    return max_fd;
}

void debug_fd_set(const char *name, const fd_set *set)
{
    printf("[%s] ", name);
    for (int fd = 0; fd <= max_fd && fd < FD_SETSIZE; fd++)
    {
        if (FD_ISSET(fd, set))
            printf("%d ", fd);
    }
    printf("\n");
}

int main(int ac, char **av)
{
    if (ac != 2)
    {
        write(2, "Wrong number of arguments\n", 26);
        return 1;
    }
    // Make event logs visible immediately when redirecting to a file.
    setvbuf(stdout, NULL, _IONBF, 0);

    FD_ZERO(&afds);
    int sockfd = create_socket();

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(2130706433);
    servaddr.sin_port = htons(atoi(av[1]));

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)))
        fatal_error();
    if (listen(sockfd, SOMAXCONN))
        fatal_error();

    printf("[LISTENING] sockfd=%d, port=%s\n", sockfd, av[1]);
    debug_fd_set("afds", &afds);

    while (1)
    {
        rfds = wfds = afds;
        int ready = select(max_fd + 1, &rfds, &wfds, NULL, NULL);
        if (ready < 0)
            fatal_error();

        // Only log when a read event occurs, not every time a socket is writable.
        int has_read_event = 0;
        for (int fd = 0; fd <= max_fd; fd++)
        {
            if (FD_ISSET(fd, &rfds))
            {
                has_read_event = 1;
                break;
            }
        }
        if (has_read_event)
        {
            printf("\n=== SELECT READ EVENT ===\n");
            printf("select returned: %d\n", ready);
            debug_fd_set("afds", &afds);
            debug_fd_set("rfds", &rfds);
            debug_fd_set("wfds", &wfds);
        }

        for (int fd = 0; fd <= max_fd; fd++)
        {
            if (!FD_ISSET(fd, &rfds))
                continue;

            if (fd == sockfd)
            {
                printf("[LISTEN] fd=%d ready for accept\n", fd);
                socklen_t addr_len = sizeof(servaddr);
                int client_fd = accept(sockfd, (struct sockaddr *)&servaddr, &addr_len);
                if (client_fd >= 0)
                {
                    printf("[ACCEPT] sockfd=%d, client_fd=%d\n", sockfd, client_fd);
                    if (client_fd >= FD_SETSIZE || client_fd >= 65536)
                    {
                        close(client_fd);
                        fatal_error();
                    }
                    register_client(client_fd);
                    debug_fd_set("afds after registration", &afds);
                    break;
                }
            }
            else
            {
                int read_bytes = recv(fd, buf_read, 1000, 0);
                printf("[RECV] fd=%d, bytes=%d\n", fd, read_bytes);
                if (read_bytes <= 0)
                {
                    remove_client(fd);
                    break;
                }
                buf_read[read_bytes] = '\0';
                msgs[fd] = str_join(msgs[fd], buf_read);
                if (!msgs[fd])
                    fatal_error();
                send_msg(fd);
            }
        }
    }
    return 0;
}
