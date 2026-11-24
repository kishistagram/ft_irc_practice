#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>




/*確認方法

Terminal 1 - サーバ
cc -Wall -Wextra -Werror experiment_accept.c -o accept
./accept

Terminal 2: Clinet A
nc -v 127.0.0.1 6667

Terminal 3: Clinet B
nc -v 127.0.0.1 6667

Terminal 4: Client C
nc -v 127.0.0.1 6667

Terminal 5: オブザーバー
ss -tanp '( sport = :6667 or dport = :6667 )'

*/

/*　出力

===== SERVER READY =====
Server fd : 3
Server IP : 127.0.0.1
Server port : 6667

Waiting for client 1...

====== CLINET 1 CONNECTED ======
Server fd : 3
Client fd : 4
clinet IP : 127.0.0.1
Client port : 53162
Address size: 16 bytes

Waiting for client 2...

====== CLINET 2 CONNECTED ======
Server fd : 3
Client fd : 5
clinet IP : 127.0.0.1
Client port : 43258
Address size: 16 bytes

Waiting for client 3...

====== CLINET 3 CONNECTED ======
Server fd : 3
Client fd : 6
clinet IP : 127.0.0.1
Client port : 57986
Address size: 16 bytes

====== ALL CLIENTS CONNCECTED =====
Press ENTER to close all sockets.



*/

int main(void)
{
	int server_fd;
	int client_fds[3];
	int yes = 1;

	struct sockaddr_in server_addr;

	// STEP1: ソケットの作成
	server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (server_fd == -1)
	{
		perror("socket");
		return 1;
	}

	// step2: setsockopt()の設定
	if (setsockopt(server_fd, 
					SOL_SOCKET,
					SO_REUSEADDR,
					&yes, sizeof(yes))
					== -1)
	{
		perror("setsocketopt");
		close(server_fd);
		return 1;
	}

	// step 3: bind()
	memset(&server_addr, 0, sizeof(server_addr));

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	server_addr.sin_port = htons(6667);

	if (bind(server_fd, (struct sockaddr *)&server_addr,
			sizeof(server_addr))
			== -1)
	{
		perror("bind");
		close(server_fd);
		return 1;
	}

	// step4: listen()
	if (listen(server_fd, 5) ==-1)
	{
		perror("listen");
		close(server_fd);
		return 1;
	}

	printf("===== SERVER READY =====\n");
	printf("Server fd : %d\n", server_fd);
	printf("Server IP : 127.0.0.1\n");
	printf("Server port : 6667\n");

	// step5: accept()
	for (int i = 0; i < 3; i++)
	{
		struct sockaddr_in client_addr;
		socklen_t client_len = sizeof(client_addr);
		char ip[INET_ADDRSTRLEN];

		printf("\nWaiting for client %d...\n", i + 1);
		
		client_fds[i] = accept(server_fd,
							(struct sockaddr *)&client_addr,
							&client_len);
		
		if (client_fds[i] == -1)
		{
			perror("accept");

			for(int j = 0; j < i; j++)
				close(client_fds[j]);
			close(server_fd);
			return 1;
		}

		printf("\n====== CLINET %d CONNECTED ======\n", i + 1);
		printf("Server fd : %d\n", server_fd);
		printf("Client fd : %d\n", client_fds[i]);

		if (inet_ntop(AF_INET, &client_addr.sin_addr, ip, sizeof(ip)) != NULL)
			printf("clinet IP : %s\n", ip);
		else
			perror("inet_ntop");
		
		printf("Client port : %u\n", (unsigned int)ntohs(client_addr.sin_port));
		printf("Address size: %u bytes\n", (unsigned int)client_len);
	}

	printf("\n====== ALL CLIENTS CONNCECTED =====\n");
	printf("Press ENTER to close all sockets.\n");

	getchar();

	for (int i = 0; i < 3; i++)
		close(client_fds[i]);
	close(server_fd);

	return 0;
}