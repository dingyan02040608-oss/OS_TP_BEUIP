#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
 
#include "gescom.h"
#include "creme.h"
 
#define _GNU_SOURCE
 
char** Mots = NULL;
int NMots = 0;
 
#define NBMAXC 15 
 
typedef struct {
    char* nom;
    int (*fonction)(int, char**);
} CommandeInterne;
 
static CommandeInterne Tabcom[NBMAXC];
static int Nbcom = 0;
 
static pid_t beuip_serv_pid = 0;
 
static int Sortie(int N, char* p[]) {
    if (beuip_serv_pid > 0) {
        kill(beuip_serv_pid, SIGUSR1);
    }
    (void)N;
    (void)p;
    exit(0);
}
 
static int CD(int N, char* p[]) {
    if (N < 2) {
        fprintf(stderr, "cd: chemin manquant\n");
        return -1;
    }
    if (chdir(p[1]) != 0) {
        perror("cd");
        return -1;
    }
    return 0;
}
 
static int PWD(int N, char* p[]) {
    (void)N;
    (void)p;
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
        return 0;
    } else {
        perror("pwd");
        return -1;
    }
}
 
static int VERS(int N, char* p[]) {
    (void)N;
    (void)p;
    printf("biceps version 2.0 (Projet Polytech OS User)\n");
    return 0;
}
 
static int BEUIP_CMD(int N, char* p[]) {
    if (N < 2) {
        printf("Utilisation : beuip start <pseudo> OR beuip stop\n");
        return -1;
    }
 
    if (strcmp(p[1], "start") == 0 && N == 3) {
        if (beuip_serv_pid > 0) {
            printf("Serveur déjà en cours d'exécution (PID: %d)\n", beuip_serv_pid);
            return -1;
        }
        beuip_serv_pid = fork();
        if (beuip_serv_pid == 0) {
            execl("./servbeuip", "servbeuip", p[2], NULL);
            perror("Erreur execl");
            exit(EXIT_FAILURE);
        }
        printf("Service BEUIP démarré pour '%s' (PID: %d)\n", p[2], beuip_serv_pid);
    } 
    else if (strcmp(p[1], "stop") == 0) {
        if (beuip_serv_pid > 0) {        
            kill(beuip_serv_pid, SIGUSR1);
            beuip_serv_pid = 0;
            printf("Signal d'arrêt envoyé au serveur BEUIP.\n");
        } else {
            printf("Aucun serveur BEUIP à arrêter.\n");
        }
    }
    return 0;
}
 
static int MESS_CMD(int N, char* p[]) {
    if (N < 2) {
        printf("Utilisation :\n  mess list\n  mess all <msg>\n  mess <pseudo> <msg>\n");
        return -1;
    }
 
    if (strcmp(p[1], "list") == 0) {   
        send_beuip_cmd('3', "127.0.0.1", "", NULL);
    } 
    else if (strcmp(p[1], "all") == 0 && N == 3) {      
        send_beuip_cmd('5', "127.0.0.1", p[2], NULL);
    } 
    else if (N == 3) {       
        send_beuip_cmd('4', "127.0.0.1", p[1], p[2]);
    } else {
        printf("Paramètres invalides pour 'mess'.\n");
        return -1;
    }
    return 0;
} 
 
int analyseCom(char *s) {
    int j = 0;
    char* token;
    int max = 100;
 
    if (Mots != NULL) {
        for (int i = 0; i < NMots; i++) {
            free(Mots[i]);
        }
        free(Mots);
    }
 
    Mots = (char**)malloc(max * sizeof(char*));
    if (Mots == NULL) return 0;
 
    while ((token = strsep(&s, " \t\n")) != NULL) {
        if (strlen(token) > 0) {
            Mots[j] = strdup(token);
            j++;
        }
    }
    Mots[j] = NULL;
    NMots = j;
    return NMots;
}
 
static void ajouteCom(char* nom, int(*fonction)(int, char**)) {
    if (Nbcom >= NBMAXC) return;
    Tabcom[Nbcom].nom = nom;
    Tabcom[Nbcom].fonction = fonction;
    Nbcom++;
}
 
void majComInt(void) {
    ajouteCom("exit", Sortie);
    ajouteCom("cd", CD);
    ajouteCom("pwd", PWD);
    ajouteCom("vers", VERS);
    ajouteCom("beuip", BEUIP_CMD); 
    ajouteCom("mess", MESS_CMD);   
}
 
void listeComInt(void) {
    printf("Commandes internes v2.0 : \n");
    for (int i = 0; i < Nbcom; i++) {
        printf("- %s\n", Tabcom[i].nom);
    }
}
 
int execComInt(int N, char **p) {
    if (N == 0 || p[0] == NULL) return 0;
    for (int i = 0; i < Nbcom; i++) {
        if (strcmp(p[0], Tabcom[i].nom) == 0) {
            Tabcom[i].fonction(N, p);
            return 1;
        }
    }
    return 0;
}
 
int execComExt(char **p) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return -1;
    } else if (pid == 0) {
        #ifdef TRACE
        printf("[TRACE] Exécution commande externe : %s\n", p[0]);
        #endif
        execvp(p[0], p);
        fprintf(stderr, "%s: commande introuvable\n", p[0]);
        exit(EXIT_FAILURE);
    } else {
        int status;
        waitpid(pid, &status, 0);
    }
    return 0;
}


