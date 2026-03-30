# OS_TP_BEUIP - Shell Biceps v2 & Protocole BEUIP

## Auteur
* **DING Yan**

## Description
Ce projet consiste en l'implémentation d'un interpréteur de commandes (Shell) nommé `biceps` intégrant un protocole de communication réseau UDP propriétaire nommé `BEUIP`. Le système permet la découverte automatique d'utilisateurs et l'échange de messages (privés ou globaux) sur un réseau local.

## Structure du Code
Conformément aux consignes de qualité, le code est modulaire et chaque sous-programme respecte la limite de **20 lignes** pour garantir la lisibilité.

* **`bicep.c`** : Gère la boucle principale du Shell, l'affichage du prompt dynamique et l'historique des commandes.
* **`gescom.c / gescom.h`** : Gère la table des commandes internes (`cd`, `pwd`, `exit`, `beuip`, `mess`) et l'exécution des commandes externes via `fork`/`exec`.
* **`creme.c / creme.h`** : Bibliothèque réseau encapsulant la forge des paquets au format BEUIP.
* **`servbeuip.c`** : Serveur d'arrière-plan gérant l'authentification, la table des utilisateurs (IP/Pseudo) et le routage des messages.

## Fonctionnalités Implémentées
Le projet valide les trois étapes du TP :

1.  **Communication UDP de base (Étape 1)** : Échange de datagrammes avec gestion des accusés de réception (AR).
2.  **Protocole BEUIP v1 (Étape 2)** : 
    * Diffusion de présence (Code 1) et réponse automatique (Code 2).
    * Maintenance d'une table de 255 utilisateurs maximum.
3.  **Intégration Shell & Automatisation (Étape 3)** :
    * Commande `beuip start <pseudo>` : Lance le serveur en arrière-plan.
    * Commande `beuip stop` : Envoie un signal `SIGUSR1` pour déclencher un broadcast de départ (Code 0) avant de fermer le service.
    * Commande `mess list` : Affiche les utilisateurs connectés.
    * Commande `mess <pseudo> <msg>` : Envoi de messages privés routés par le serveur.

## Compilation et Rendu
Le projet utilise un `Makefile` avec les options strictes `-Wall -Werror`. Toute compilation produisant un avertissement est considérée comme un échec (0).

Pour compiler :
```bash
make
