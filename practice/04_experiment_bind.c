#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/*
getsockname()
カーネルが管理しているソケットのローカルアドレス情報を取得するシステムコール

#include <sys/socket.h>

int getsockname(int sockfd,
                struct sockaddr *addr,
                socklen_t *addrlen);



sockfd -> 情報を取得したいソケットのFD
addr 	-> 取得した情報を格納する構造体へのポインタ
addrlen	-> 構造体のサイズを格納する変数へのポインタ


*/

void print_socket_info(int fd)
{
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);
	char ip[INET_ADDRSTRLEN];

	memset(&addr, 0, sizeof(addr));

	if (getsockname(fd, (struct sockaddr *)&addr, &len) == -1)
	{
		perror("getsockname");
		return;
	}

	if (inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip)) == NULL)
	{
		perror("inet_ntop");
		return;
	}

	printf("fd	: %d\n", fd);
	printf("family	: %d\n", addr.sin_family);
	printf("IP	: %s\n", ip);
	printf("Port	: %u\n", (unsigned int)ntohs(addr.sin_port));
}

int main(void)
{
	int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (fd == -1)
	{
		perror("socket");
		return 1;
	}

	printf("===== BEFORE BIND =====\n");
	print_socket_info(fd);

	struct sockaddr_in address;

	memset(&address, 0, sizeof(address));

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
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

	close(fd);
	return 0;
}