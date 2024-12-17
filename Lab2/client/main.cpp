#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

int main()
{
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddres;
    serverAddres.sin_family = AF_INET;
    serverAddres.sin_port = htons(2000);
    serverAddres.sin_addr.s_addr = INADDR_ANY;

    connect(clientSocket, (sockaddr*) &serverAddres, sizeof(serverAddres));

    while(true)
    {
        char message[1024];
        printf("Enter message: ");
        scanf("%s", &message);
        send(clientSocket, message, strlen(message), 0);
    }
}