#include <stdio.h>
#include <string.h> //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>    //close
#include <arpa/inet.h> //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros

#define TRUE 1
#define FALSE 0
#define PORT 5050
#define MAX_CLIENTS 30

int main()
{
    int opt = TRUE;
    int master_socket, addrlen, new_socket, client_sockets[MAX_CLIENTS], activity, i,
        valread, sd;

    int max_sd;
    struct sockaddr_in address;

    char buffer[1024];

    fd_set readfds;

    char *message = "LEHA Deamon v0.1 \r\n";

    for (i = 0; i < MAX_CLIENTS; i++)
    {
        client_sockets[i] = 0;
    }

    if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)
    {
        perror("Sockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);
    address.sin_addr.s_addr = INADDR_ANY;

    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Bind failure");
        exit(EXIT_FAILURE);
    }
    printf("Listener on port: %d \n", PORT);

    if (listen(master_socket, 3) < 0)
    {
        perror("Listen failure");
        exit(EXIT_FAILURE);
    }

    addrlen = sizeof(address);
    puts("Waiting for connections...");

    while (TRUE)
    {
        FD_ZERO(&readfds);

        FD_SET(master_socket, &readfds);
        max_sd = master_socket;

        for (i = 0; i < MAX_CLIENTS; i++)
        {
            sd = client_sockets[i];

            if (sd > 0)
            {
                FD_SET(sd, &readfds);
            }

            if (sd > max_sd)
            {
                max_sd = sd;
            }
        }

        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (errno != EINTR))
        {
            perror("Select failure");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(master_socket, &readfds))
        {
            if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0){
                perror("Accept failure");
                exit(EXIT_FAILURE);
            }

            printf("New connection, socket fd is %d, ip is %s, port is %d\n", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            if(send(new_socket, message, strlen(message), 0) != strlen(message)){
                perror("Send broken");
            }

            puts("Welcome message sent successfully");

            for(i = 0; i < MAX_CLIENTS; i++){
                if(client_sockets[i] == 0){
                    client_sockets[i] = new_socket;
                    printf("Adding to list of sockets as %d\n", i);

                    break;
                }
            }
        }

        for(i = 0; i < MAX_CLIENTS; i++){
            sd = client_sockets[i];

            if(FD_ISSET(sd, &readfds)){
                if((valread = read( sd, buffer, 1023)) == 0){
                    getpeername(sd, (struct sockaddr *)&address, (socklen_t*)&addrlen);

                    printf("Host disconnected, ip %s , port %d \n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

                    close(sd);
                    client_sockets[i] = 0;
                }
                else
                {
                    buffer[valread] = '\0';
                    send(sd, buffer, strlen(buffer), 0);
                }
                
            }
        }

        sleep(1);
    }

    return 0;
}