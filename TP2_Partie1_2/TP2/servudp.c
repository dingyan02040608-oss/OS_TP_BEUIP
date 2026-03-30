

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_BUFFER 1024

/* parametres :
        P[1] = port d'ecoute
*/
int main(int N, char*P[])
{
    int sid;
    struct sockaddr_in ServerSock, ClientSock;
    char buffer[MAX_BUFFER];
    socklen_t client_len = sizeof(ClientSock);

    if (N != 2) {
        fprintf(stderr,"Utilisation : %s port\n", P[0]);
        return(1);
    }

    /* 1. creation du socket */
    if ((sid=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("socket");
        return(2);
    }

    /* 2. configuration de l'adresse du serveur */
    bzero(&ServerSock, sizeof(ServerSock));
    ServerSock.sin_family = AF_INET;
    ServerSock.sin_addr.s_addr = INADDR_ANY; 
    ServerSock.sin_port = htons(atoi(P[1]));

    /* 3. attachement du socket */
    if (bind(sid, (struct sockaddr *)&ServerSock, sizeof(ServerSock)) < 0) {
        perror("bind");
        return(3);
    }

    printf("Serveur UDP en ecoute sur le port %s...\n", P[1]);

    /* 4. boucle de reception  */
    while (1) {
        int n = recvfrom(sid, buffer, MAX_BUFFER - 1, 0, 
                         (struct sockaddr *)&ClientSock, &client_len);
        if (n < 0) {
            perror("recvfrom");
            continue;
        }
        
        buffer[n] = '\0'; 
        printf("Message recu de %s:%d -> %s\n", 
               inet_ntoa(ClientSock.sin_addr), 
               ntohs(ClientSock.sin_port), 
               buffer);
        char *ack_msg = "Bien recu 5/5 !";
        if(sendto(sid, ack_msg, strlen(ack_msg), 0, (struct sockaddr*)&ClientSock, client_len) < 0){
                perror("sendo AR");
        }
        
    }

    return 0;
}