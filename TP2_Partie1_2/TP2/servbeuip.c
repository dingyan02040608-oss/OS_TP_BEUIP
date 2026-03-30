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
#define BROADCAST_IP "192.168.88.255"
 
struct User {
    struct in_addr ip;
    char pseudo[256];
};
 
struct User table[255];
int user_count = 0;
void add_user(struct in_addr ip, const char* pseudo) {
    for (int i = 0; i < user_count; i++) {
        if (table[i].ip.s_addr == ip.s_addr) return; 
    }
    if (user_count < 255) {
        table[user_count].ip = ip;
        strncpy(table[user_count].pseudo, pseudo, 255);
        user_count++;
        printf("[INFO] Nouvel utilisateur ajoute: %s (%s)\n", pseudo, inet_ntoa(ip));
    }
}
 
 
 
int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Utilisation : %s <pseudo>\n", argv[0]);
        return 1;
    }
    char *my_pseudo = argv[1];
    int sid;
    struct sockaddr_in ServerSock, ClientSock;
    char buffer[MAX_BUFFER];
    socklen_t client_len = sizeof(ClientSock);
 
    if ((sid = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("socket"); return 2;
    }
 
    int broadcastEnable = 1;
    setsockopt(sid, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));
 
    bzero(&ServerSock, sizeof(ServerSock));
    ServerSock.sin_family = AF_INET;
    ServerSock.sin_addr.s_addr = INADDR_ANY;
    ServerSock.sin_port = htons(PORT);
 
    if (bind(sid, (struct sockaddr *)&ServerSock, sizeof(ServerSock)) < 0) {
        perror("bind"); return 3;
    }
 
    char broadcast_msg[MAX_BUFFER];
    sprintf(broadcast_msg, "1%s%s", MAGIC, my_pseudo);
    struct sockaddr_in bcast_addr;
    bzero(&bcast_addr, sizeof(bcast_addr));
    bcast_addr.sin_family = AF_INET;
    bcast_addr.sin_addr.s_addr = inet_addr(BROADCAST_IP);
    bcast_addr.sin_port = htons(PORT);
    sendto(sid, broadcast_msg, strlen(broadcast_msg), 0, (struct sockaddr *)&bcast_addr, sizeof(bcast_addr));
 
    printf("Serveur BEUIP demarre avec le pseudo '%s'. En ecoute sur le port %d...\n", my_pseudo, PORT);
 
    while (1) {
        int n = recvfrom(sid, buffer, MAX_BUFFER - 1, 0, (struct sockaddr *)&ClientSock, &client_len);
        if (n < 0) continue;
        buffer[n] = '\0';
 
        char code = buffer[0];
        char magic[6];
        strncpy(magic, buffer + 1, 5);
        magic[5] = '\0';
        char *payload = buffer + 6;
 
        if (strcmp(magic, MAGIC) != 0) continue; 
 
        if (code == '1') {
            add_user(ClientSock.sin_addr, payload);
            char ar_msg[MAX_BUFFER];
            sprintf(ar_msg, "2%s%s", MAGIC, my_pseudo);
            sendto(sid, ar_msg, strlen(ar_msg), 0, (struct sockaddr *)&ClientSock, client_len);
        } 
        else if (code == '2') {
            add_user(ClientSock.sin_addr, payload);
        }
        else if (code == '3' && strcmp(inet_ntoa(ClientSock.sin_addr), "127.0.0.1") == 0) {
            printf("\n--- Liste des presents ---\n");
            for (int i = 0; i < user_count; i++) {
                printf("%s - %s\n", table[i].pseudo, inet_ntoa(table[i].ip));
            }
            printf("--------------------------\n");
        }
        else if (code == '4' && strcmp(inet_ntoa(ClientSock.sin_addr), "127.0.0.1") == 0) {
            char *target_pseudo = payload;
            char *msg = payload + strlen(target_pseudo) + 1; 
            
            for (int i = 0; i < user_count; i++) {
                if (strcmp(table[i].pseudo, target_pseudo) == 0) {
                    char forward_msg[MAX_BUFFER];
                    sprintf(forward_msg, "9%s%s", MAGIC, msg); 
                    struct sockaddr_in target_addr;
                    bzero(&target_addr, sizeof(target_addr));
                    target_addr.sin_family = AF_INET;
                    target_addr.sin_addr = table[i].ip;
                    target_addr.sin_port = htons(PORT);

                    sendto(sid, forward_msg, strlen(forward_msg), 0, (struct sockaddr *)&target_addr, sizeof(target_addr));
                    printf("Message relaye a %s.\n", target_pseudo);
                    break;
                }
            }
        }
        else if (code == '9') {
            char *sender = "Inconnu";
            for (int i = 0; i < user_count; i++) {
                if (table[i].ip.s_addr == ClientSock.sin_addr.s_addr) {
                    sender = table[i].pseudo; break;
                }
            }
            printf("\nMessage de %s: %s\n", sender, payload);
        }
        else if (code == '5' && strcmp(inet_ntoa(ClientSock.sin_addr), "127.0.0.1") == 0) {
            char forward_msg[MAX_BUFFER];
            sprintf(forward_msg, "9%s%s", MAGIC, payload);
            for (int i = 0; i < user_count; i++) {              
                struct sockaddr_in target_addr;
                bzero(&target_addr, sizeof(target_addr));
                target_addr.sin_family = AF_INET;
                target_addr.sin_addr = table[i].ip;
                target_addr.sin_port = htons(PORT);
                sendto(sid, forward_msg, strlen(forward_msg), 0, (struct sockaddr *)&target_addr, sizeof(target_addr));
            }
            printf("Message diffuse a tout le monde.\n");
        }
        else if (code == '0') {
            for (int i = 0; i < user_count; i++) {
                if (table[i].ip.s_addr == ClientSock.sin_addr.s_addr) {
                    printf("[INFO] Utilisateur quitte: %s\n", table[i].pseudo);
                    table[i] = table[user_count - 1]; 
                    user_count--;
                    break;
                }
            }
        }
    }
    return 0;
}