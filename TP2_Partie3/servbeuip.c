#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
 
#define PORT 9998              
#define MAGIC "BEUIP"         
#define MAX_USERS 255          
#define BCAST_ADDR "192.168.88.255" 
 

typedef struct {
    struct in_addr ip;
    char pseudo[256];
} User;
 
int sid;
char my_pseudo[256];
User table[MAX_USERS];
int user_count = 0;
 

void add_user(struct in_addr ip, const char* pseudo) {
    for (int i = 0; i < user_count; i++) {
        if (table[i].ip.s_addr == ip.s_addr) {
            strncpy(table[i].pseudo, pseudo, 255); 
            return;
        }
    }
    if (user_count < MAX_USERS) {
        table[user_count].ip = ip;
        strncpy(table[user_count].pseudo, pseudo, 255);
        user_count++;
        #ifdef TRACE
        printf("[TRACE] Nouvel utilisateur : %s (%s)\n", pseudo, inet_ntoa(ip));
        #endif
    }
}
 

void remove_user(struct in_addr ip) {
    for (int i = 0; i < user_count; i++) {
        if (table[i].ip.s_addr == ip.s_addr) {
            table[i] = table[user_count - 1]; 
            user_count--;
            return;
        }
    }
}
 

char* get_pseudo_by_ip(struct in_addr ip) {
    for (int i = 0; i < user_count; i++) {
        if (table[i].ip.s_addr == ip.s_addr) return table[i].pseudo;
    }
    return "Inconnu";
}

void handle_stop(int sig) {
    (void)sig;
    char bye[10];
    sprintf(bye, "0%s", MAGIC); 
 
    struct sockaddr_in bcast;
    bcast.sin_family = AF_INET;
    bcast.sin_port = htons(PORT);
    bcast.sin_addr.s_addr = inet_addr(BCAST_ADDR);
 
    int broadcastEnable = 1;
    setsockopt(sid, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable));
 
    sendto(sid, bye, strlen(bye), 0, (struct sockaddr *)&bcast, sizeof(bcast));
 
    #ifdef TRACE
    printf("\n[TRACE] Signal SIGUSR1 reçu. Broadcast code 0 envoyé. Arrêt.\n");
    #endif
    close(sid);
    exit(0);
}
 
int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Utilisation : %s <pseudo>\n", argv[0]);
        return 1;
    }
    strncpy(my_pseudo, argv[1], 255);
    signal(SIGUSR1, handle_stop); 
 
    if ((sid = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket"); return 2;
    }
 
    struct sockaddr_in serv_addr, cli_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);
 
    if (bind(sid, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind"); return 3;
    }
 
    int bEnable = 1;
    setsockopt(sid, SOL_SOCKET, SO_BROADCAST, &bEnable, sizeof(bEnable));
 
    char init_msg[512];
    sprintf(init_msg, "1%s%s", MAGIC, my_pseudo);
    struct sockaddr_in bcast_addr;
    bcast_addr.sin_family = AF_INET;
    bcast_addr.sin_port = htons(PORT);
    bcast_addr.sin_addr.s_addr = inet_addr(BCAST_ADDR);
 
    sendto(sid, init_msg, strlen(init_msg), 0, (struct sockaddr *)&bcast_addr, sizeof(bcast_addr));
    printf("Serveur BEUIP lancé sur le port %d. Pseudo : %s\n", PORT, my_pseudo);
 
    while (1) {
        char buf[1024];
        socklen_t addr_len = sizeof(cli_addr);
        int n = recvfrom(sid, buf, 1023, 0, (struct sockaddr *)&cli_addr, &addr_len);
        if (n < 0) continue;
        buf[n] = '\0';
 
        if (strncmp(buf + 1, MAGIC, 5) != 0) continue; 
 
        char code = buf[0];
        char* payload = buf + 6;
        int from_local = (strcmp(inet_ntoa(cli_addr.sin_addr), "127.0.0.1") == 0); 
 
        if (code == '1') { 
            add_user(cli_addr.sin_addr, payload);
            char ack[512];
            sprintf(ack, "2%s%s", MAGIC, my_pseudo);
            sendto(sid, ack, strlen(ack), 0, (struct sockaddr *)&cli_addr, addr_len);
        }
        else if (code == '2') { 
            add_user(cli_addr.sin_addr, payload);
        }
        else if (code == '3' && from_local) { 
            printf("\n--- Liste BEUIP ---\n");
            for (int i = 0; i < user_count; i++) {
                printf("[%s] - %s\n", table[i].pseudo, inet_ntoa(table[i].ip));
            }
            printf("-------------------\n");
        }
        else if (code == '4' && from_local) {
            char* target_pseudo = payload;
            char* message = payload + strlen(target_pseudo) + 1; 
 
           
            for (int i = 0; i < user_count; i++) {
                if (strcmp(table[i].pseudo, target_pseudo) == 0) {
                    char msg_9[1024];
                    sprintf(msg_9, "9%s%s", MAGIC, message); 
                    struct sockaddr_in target;
                    target.sin_family = AF_INET;
                    target.sin_port = htons(PORT);
                    target.sin_addr = table[i].ip;
                    sendto(sid, msg_9, strlen(msg_9) + 1, 0, (struct sockaddr *)&target, sizeof(target));
                    break;
                }
            }
        }
        else if (code == '5' && from_local) {
            char msg_9[1024];
            sprintf(msg_9, "9%s%s", MAGIC, payload);
            for (int i = 0; i < user_count; i++) {
             
                if (strcmp(table[i].pseudo, my_pseudo) == 0) continue;
 
                struct sockaddr_in target;
                target.sin_family = AF_INET;
                target.sin_port = htons(PORT);
                target.sin_addr = table[i].ip;
                sendto(sid, msg_9, strlen(msg_9) + 1, 0, (struct sockaddr *)&target, sizeof(target));
            }
        }
        else if (code == '9') { 
            printf("\nMessage de %s : %s\n", get_pseudo_by_ip(cli_addr.sin_addr), payload);
        }
        else if (code == '0') { 
            remove_user(cli_addr.sin_addr);
            #ifdef TRACE
            printf("[TRACE] Départ de l'utilisateur : %s\n", inet_ntoa(cli_addr.sin_addr));
            #endif
        }
    }
    return 0;
}