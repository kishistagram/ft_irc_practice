#include <stdio.h>
#include <stdint.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void print_bytes(uint32_t value)
{
    unsigned char *p = (unsigned char *)&value;

    for (int i = 0; i < 4; i++)
        printf("%02X ", p[i]);
    printf("\n");
}

void inspect_address(const char *name, uint32_t host_value)
{
    struct sockaddr_in address = {0};
    char ip_str[INET_ADDRSTRLEN];
    uint32_t network_value = htonl(host_value);

    printf("\n===== %s =====\n", name);

    printf("[Before htonl]\n");
    printf("Decimal : %u\n", host_value);
    printf("Hex     : 0x%08X\n", host_value);
    printf("Memory  : ");
    print_bytes(host_value);

    printf("\n[After htonl]\n");
    printf("Decimal : %u\n", network_value);
    printf("Hex     : 0x%08X\n", network_value);
    printf("Memory  : ");
    print_bytes(network_value);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = network_value;

    inet_ntop(AF_INET, &address.sin_addr,
              ip_str, sizeof(ip_str));

    printf("\n[Final sockaddr_in]\n");
    printf("sin_family      : %d\n", address.sin_family);
    printf("sin_addr.s_addr : 0x%08X\n",
           address.sin_addr.s_addr);
    printf("IP address      : %s\n", ip_str);
}

int main(void)
{
    inspect_address("INADDR_ANY", INADDR_ANY);
    inspect_address("INADDR_LOOPBACK", INADDR_LOOPBACK);

    return 0;
}