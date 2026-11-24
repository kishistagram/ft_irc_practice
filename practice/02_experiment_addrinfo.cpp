/* getaddrinfo()を利用してOSからアドレス情報を取得する */


#include <iostream>
#include <cstring>
#include <netdb.h>
#include <sys/socket.h>

int main()
{
    struct addrinfo hints;
    struct addrinfo *result = NULL;

    std::memset(&hints, 0, sizeof(hints));

    // IPv4とIPv6の両方を候補にする
    hints.ai_family = AF_UNSPEC;

    // ストリーム型ソケットを指定
    hints.ai_socktype = SOCK_STREAM;

    // bind()用のアドレスを取得
    hints.ai_flags = AI_PASSIVE;

    int status = getaddrinfo(
        NULL,
        "6667",
        &hints,
        &result
    );

    if (status != 0)
    {
        std::cerr << gai_strerror(status) << std::endl;
        return 1;
    }

    // 取得したアドレス候補を表示
    for (struct addrinfo *p = result;
         p != NULL;
         p = p->ai_next)
    {
        std::cout << "Family: "
                  << p->ai_family << std::endl;

        std::cout << "Socket type: "
                  << p->ai_socktype << std::endl;

        std::cout << "Protocol: "
                  << p->ai_protocol << std::endl;

        std::cout << "Address length: "
                  << p->ai_addrlen << std::endl;

        std::cout << "------------------"
                  << std::endl;
    }

    freeaddrinfo(result);

    return 0;
}
