#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
 
#define MAX_BUFFER 1024
#define PORT 9998
#define MAGIC "BEUIP"
 
int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Utilisation :\n");
        printf("  %s 3 (Liste)\n", argv[0]);
        printf("  %s 4 <pseudo> <message> (Message prive)\n", argv[0]);
        printf("  %s 5 <message> (Message a tous)\n", argv[0]);
        printf("  %s 0 (Quitter le reseau)\n", argv[0]);
        return 1;
    }
    int sid;
    struct sockaddr_in servaddr;
    char buffer[MAX_BUFFER];
    char code = argv[1][0];
 
    if ((sid = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("socket"); return 2;
    }
    int broadcastEnable = 1;
    setsockopt(sid, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); 
    servaddr.sin_port = htons(PORT);
 
    int len = 0;
    if (code == '3') {
        len = sprintf(buffer, "3%s", MAGIC);
    } 
    else if (code == '4' && argc == 4) {
        len = sprintf(buffer, "4%s%s", MAGIC, argv[2]);
        int pseudo_len = strlen(argv[2]);
        strcpy(buffer + 6 + pseudo_len + 1, argv[3]);
        len = 6 + pseudo_len + 1 + strlen(argv[3]);
    } 
    else if (code == '5' && argc == 3) {
        len = sprintf(buffer, "5%s%s", MAGIC, argv[2]);
    }
    else if (code == '0') {
        len = sprintf(buffer, "0%s", MAGIC);
        servaddr.sin_addr.s_addr = inet_addr("192.168.88.255");
    } else {
        printf("Parametres invalides.\n");
        return 1;
    }
 
    sendto(sid, buffer, len, 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
    printf("Commande envoyee.\n");
 
    return 0;
}

