
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/uio.h>
#include <sys/time.h>


int main()
{
    int clientSocket = 0;
    struct sockaddr_in servAddr;
    char buffer[1024] = {0};
    if ((clientSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }
    memset(&servAddr, '0', sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(12345);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &servAddr.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    strcpy(buffer, "Hello from client");
    
    sendto(clientSocket, buffer, strlen(buffer), 0, (const struct sockaddr *)&servAddr, sizeof(servAddr));
    
    if(connect(clientSocket, (const struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
    {
        printf("Error connecting to server\n");
        return -1;
    }

    sendmsg(clientSocket, buffer, strlen(buffer));
    
    send(clientSocket, "Hello, World!", strlen("Hello, World!"), 0);
    
    int valread = read(clientSocket, buffer, 1024);

}