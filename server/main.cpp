#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

sockaddr_in set_up_server_addres(sa_family_t type = AF_INET, int port = 2000, in_addr_t addres = INADDR_ANY)
{
    sockaddr_in serverAddres;
    serverAddres.sin_family = type;
    serverAddres.sin_port = htons(port);
    serverAddres.sin_addr.s_addr = addres;
    return serverAddres;
}

int accept_client(int serverSocket)
{
    sockaddr_in clientAddres;
    socklen_t clientSize = sizeof(clientAddres);

    int clientSocket = accept(serverSocket, (sockaddr*) &clientAddres, &clientSize);
    printf("Client connect %s:%d\n", inet_ntoa(clientAddres.sin_addr), ntohs(clientAddres.sin_port));
    return clientSocket;
}

void process_client(int& clientSocket)
{
    char buffer[1024];
    int byteSize = recv(clientSocket, buffer, sizeof(buffer), 0);
    if(byteSize == 0)
    {
        puts("Client disconnect");
        close(clientSocket);
        clientSocket = -1;
    }
    if(byteSize > 0)
    {
        printf("Message byte size: %d\n", byteSize);
        fflush(stdout);
    }
}

void check_new_connection(int serverSocket, int& clientSocket)
{
    int newConnection = accept_client(serverSocket);
    if(clientSocket == -1)
    {
        clientSocket = newConnection;
    }
    else
    {
        puts("Connection limit reached, client disconnect");
        close(newConnection);
    }
}

volatile sig_atomic_t wasSigHup = 0;
void signal_handler(int signal)
{
    wasSigHup = 1;
}

int main()
{
    struct sigaction sa;
    sigaction(SIGHUP, NULL, &sa); // Получение информации о сигнале
    sa.sa_handler = signal_handler;
    sa.sa_flags |= SA_RESTART;
    sigaction(SIGHUP, &sa, NULL); // Запись новой информации

    sigset_t blockedMask;
    sigset_t origMask;
    sigaddset(&blockedMask, SIGHUP);
    sigprocmask(SIG_BLOCK, &blockedMask, &origMask);

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddres = set_up_server_addres();

    bind(serverSocket, (sockaddr*) &serverAddres, sizeof(serverAddres));
    listen(serverSocket, 1);

    printf("Server listen on %s:%d\n", inet_ntoa(serverAddres.sin_addr), ntohs(serverAddres.sin_port));

    int clientSocket = -1;
    fd_set readDescriptors;
    while(true)
    {
        FD_ZERO(&readDescriptors);
        FD_SET(serverSocket, &readDescriptors);

        if(clientSocket != -1)
        {
            FD_SET(clientSocket, &readDescriptors);
        }

        int maxFd = ((clientSocket > serverSocket) ? clientSocket : serverSocket);
        int activity = pselect(maxFd + 1, &readDescriptors, NULL, NULL, NULL, &origMask);

        if(activity == -1)
        {
            if(errno == EINTR)
            {
                printf("Signal recive\n");
                wasSigHup = 0;
                continue;
            }
        }

        if(FD_ISSET(serverSocket, &readDescriptors))
        {
            check_new_connection(serverSocket, clientSocket);
            continue;
        }
        if(clientSocket != -1 && FD_ISSET(clientSocket, &readDescriptors))
        {
            process_client(clientSocket);
            continue;
        }
    }
}