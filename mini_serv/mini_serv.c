// 1. INCLUDES (6)
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <netinet/ip.h>
#include <sys/select.h>

// 2. GLOBAL VARIABLES (6 GROUPS)
int		count = 0;
int		max_fd = 0;
int		ids[65536];
char	*msgs[65536];
fd_set	rfds, wfds, afds;
char	buf_read[1001], buf_write[42];

/*
rfds -> 

wfds -> サーバが書き込み可能なFD

*/

// 3. PROVIDED HELPERS MAIN.C (2)
// START COPY-PASTE FROM GIVEN MAIN

int extract_message(char **buf, char **msg)
{
	char	*newbuf;
	int	i;

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
	char	*newbuf;
	int		len;

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

// END COPY-PASTE

// 4. SERVER FUNCTIONS (6)
void	fatal_error()
{
	write(2, "Fatal error\n", 12);
	exit(1);
}

void	notify_other(int author, char *str)
{
	for (int fd = 0; fd <= max_fd; fd++)
	{
		if (FD_ISSET(fd, &wfds) && fd != author)
			send(fd, str, strlen(str), 0);
	}
}

void	register_client(int fd)
{
	max_fd = fd > max_fd ? fd : max_fd;
	ids[fd] = count++;
	msgs[fd] = NULL;
	FD_SET(fd, &afds);
	sprintf(buf_write, "server: client %d just arrived\n", ids[fd]);
	notify_other(fd, buf_write);
}

void	remove_client(int fd)
{
	sprintf(buf_write, "server: client %d just left\n", ids[fd]);
	notify_other(fd, buf_write);
	free(msgs[fd]);
	FD_CLR(fd, &afds);
	close(fd);
}

void	send_msg(int fd)
{
	char *msg;

	while (extract_message(&(msgs[fd]), &msg))
	{
		sprintf(buf_write, "client %d: ", ids[fd]);
		notify_other(fd, buf_write);
		notify_other(fd, msg);
		free(msg);
	}
}

int		create_socket()
{
	max_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (max_fd < 0)
		fatal_error();
	FD_SET(max_fd, &afds);
	return max_fd;
}
void debug_fd_set(char *name, fd_set *set)
{
    printf("[%s] ", name);

    for (int fd = 0; fd <= max_fd; fd++)
    {
        if (FD_ISSET(fd, set))
            printf("%d ", fd);
    }

    printf("\n");
}

// 5. MAIN (1)
int		main(int ac, char **av)
{
	// SETUP (6 steps): argc -> zero -> socket -> address -> bind -> listen
	
	// 引数チェック
	if (ac != 2)
	{
		write(2, "Wrong number of arguments\n", 26);
		exit(1);
	}

	// fd_set(監視対象FD）の初期化
	FD_ZERO(&afds);
	// socketの作成
	int sockfd = create_socket();

	// START COPY-PASTE FROM MAIN

	// socket用のアドレス構造体を初期化
	struct sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));

	// 設定をする
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(2130706433);
	servaddr.sin_port = htons(atoi(av[1])); // replace 8080

	// sockfdにローカルIPアドレスとポート番号を割り当てている
	if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)))
		fatal_error();
	//　接続を待ち受ける状態にする、ソケットをクライアントからのTCP接続を受け付けられる状態に変更する
	if (listen(sockfd, SOMAXCONN)) // the main uses 10, SOMAXCONN is 180 on my machine
		fatal_error();

	// END COPY-PASTE

	/*
		#include <sys/select.h>

		int select(int nfds, fd_set *readfds, fd_set *writefds,
			fd_set *exceptfds, struct timeval *timeout);
		複数のFDを同時に監視し、I/O操作が可能になったFDを教えてくれるシステムコール
		対応するI/O操作(read()や充分に小さいwrite())をブロックせずに実行できる場合、そのFDはreadyとみなされる
		select()の制限については、監視できるFD番号はFD_SETSIZE未満、一般的には1024
		poll(), epoll()にはこの制約がない
	*/

	/*
	rfds -> このFDからデータを読み取れる　対応する操作はrecv()
	wfds -> このFDへデータを書き込める  対応する操作はsend()
	*/

	// LOOP (6 steps): while -> copy -> select -> for -> accept / recv
	
	while (1)
	{
		rfds = wfds = afds;

		int ready = select(max_fd + 1, &rfds, &wfds, NULL, NULL);
		if (ready < 0)
    		fatal_error();
		
		int has_read_event = 0;


		// FDを0からmax_fdまで順番に調べる
		for (int fd = 0; fd <= max_fd; fd++)
		{
			// 読み取り可能なFDか調べる
			if (!FD_ISSET(fd, &rfds))
			{
				has_read_event = 1;
				break;
			}

			if (has_read_event)
			{
				printf("\n=== SELECT READ EVENT ===\n");
				printf("select returned: %d\n", ready);
				debug_fd_set("afds", &afds);
				debug_fd_set("rfds", &rfds);
				debug_fd_set("wfds", &wfds);
			}

			// 待ちソケットか既存クライアントかを判断する
			if (fd == sockfd)
			{
			    printf("[LISTEN] fd=%d is ready for accept\n", fd);

				// 新規接続を受け付け、クライアントを登録する
				// New client connection
				socklen_t addr_len = sizeof(servaddr);
				int client_fd = accept(sockfd, (struct sockaddr *)&servaddr, &addr_len);

				if (client_fd >= 0)
				{
					printf("[ACCEPT] sockfd=%d, new client_fd=%d\n", sockfd, client_fd);
					register_client(client_fd);
        			debug_fd_set("afds after registration", &afds);
					break ;
				}
			}
			else
			{
				//	 Existing client sent data
				int read_bytes = recv(fd, buf_read, 1000, 0);

				printf("[RECV] fd=%d, bytes=%d\n", fd, read_bytes);

				if (read_bytes <= 0)
				{
					remove_client(fd);
					break ;
				}
				buf_read[read_bytes] = '\0';
				msgs[fd] = str_join(msgs[fd], buf_read);
				send_msg(fd);
			}
		}
	}
	return 0;
}