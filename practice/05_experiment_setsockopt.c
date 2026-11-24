/*
コンピュータシステム p.768
"20行目でサーバを終了して再起動したあtおすぐにコネクション要求を受け付け始められるように設定するために
setsockopt関数を用いていることに注意せよ。
デフォルトでは再起動されたサーバはクライアントからのコネクション要求を約３０行間拒否する、
このことはでバグの深刻な障害となる"
*/


/*
#include <sys/socket.h>

int setsocket(int sockfd,
			  int level,
			  int optname,
			  void *optval,
			  socklen_t *optlen);

sockfd	->　オープンされているソケットディスクリプタではなければならない
leve	-> 指定したオプションを解釈するシステム内のコード、すなわち一般的なソケットのコードか、
		プロトコル依存ののコード（ex)IPv4, IPv6, TCP)を指定
optval -> setsocketにオプションの新しい値を与える、変数へのポインタ。
この変数の大きさは最後の引数で指定され、値引数となる。 

*/

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

void print_socket_option(int fd)
{
	int value = 0;
	socklen_t len = sizeof(value);

	if (getsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
					&value, &len) == -1)
	{
		perror("getsockopt");
		return;
	}

	printf("SO_REUSEADDR : %d\n", value);
	printf("Option size : %u bytes\n", (unsigned int)len);
	printf("Status : %s\n", value ? "ENABLED" : "DISABLED");
}



int main(void)
{
	int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (fd == -1)
	{
		perror("socket");
		return 1;
	}

	printf("\n====== BEFORE SETSOCKOPT ======\n");
	print_socket_option(fd);

	int yes = 1;

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
	{
		perror("setsockopt");
		close(fd);
		return 1;
	}

	printf("\n===== AFTER SETSOCKOPT =====\n");
	print_socket_option(fd);

	close(fd);
	return 0;
}