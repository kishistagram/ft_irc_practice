#include <iostream>
#include <cstring>
#include <netinet/in.h>
#include <arpa/inet.h>

/* IP socket address structure */

// struct sockaddr_in 
// {
// 	uint16_t sin_family;       /* Protocol family (always AF_INET) */
// 	uint16_t sin_port;         /* Port number in network byte order */
// 	struct in_addr sin_addr;   /* IP address in network byte order */
// 	unsigned char sin_zero[8]; /* Pad to sizeof(struct sockaddr)  */
// };

// struct sockaddr
// {
// 	uint16_t sa_family;         /* Protocol family */
// 	char 	 sa_data[14];		/* Address data    */
// };


int main()
{
	// IPv4アドレス構造体を作成する
	struct sockaddr_in addr;
	std::memset(&addr, 0, sizeof(addr));

	// 2. アドレス体系を設定する
	addr.sin_family = AF_INET;

	// 3. ポート番号を指定
	addr.sin_port = htons(6667);

	// 4. IPv4アドレスを設定
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	// 5. バイナリアドレスを文字列に変換
	char ip[INET_ADDRSTRLEN];

	if (inet_ntop(AF_INET, &addr.sin_addr, ip, sizeof(ip)) == NULL)
	{
		std::cerr << "inet_ntop failed" << std::endl;
		return 1;
	}

    // 6. 構造体の情報を表示
    std::cout << "Address family: "
              << addr.sin_family << std::endl;

    std::cout << "IP address: "
              << ip << std::endl;

    std::cout << "Port: "
              << ntohs(addr.sin_port) << std::endl;

    std::cout << "sizeof(sockaddr_in): "
              << sizeof(addr) << std::endl;

    std::cout << "sizeof(sockaddr): "
              << sizeof(struct sockaddr) << std::endl;

    return 0;	



}
