#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "original.h"
#define N 4192

int main(int argc, char **argv)
{
    int port_number = atoi(argv[1]);

    if (strcmp(argv[2], "server") == 0)
    {
        if (argc != 3)
        {
            perror("usage: ./original port_number server");
            exit(1);
        }

        int s = make_socket_for_server(port_number);

        while (1)
        {
            conversation(s, N);
        }
        close(s);
    }
    else if (strcmp(argv[2], "client") == 0)
    {
        if (argc != 4)
        {
            perror("usage: ./original port_number client IP_number");
            exit(1);
        }
        char * ip_number = argv[3];
        int s = make_socket_for_client(port_number, ip_number);

        while (1)
        {
            conversation(s, N);
        }
        close(s);
    }
    else
    {
        perror("usage: ./original port_number client IP_number or ./original_server port_number server");
        exit(1);
    }
}