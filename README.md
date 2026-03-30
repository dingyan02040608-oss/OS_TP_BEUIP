# OS_TP_BEUIP - Implémentation du protocole BEUIP (Parties 1 & 2)

## Auteur
* **DING Yan**

## Description
Ce dépôt contient les deux premières parties du TP2 d'Architecture des Systèmes d'Exploitation. L'objectif est de mettre en œuvre une communication réseau UDP et d'implémenter les bases du protocole de messagerie instantanée BEUIP.

## Structure du Projet
Conformément aux exigences de lisibilité et de maintenance, chaque sous-programme contient moins de 20 lignes de code.

### Étape 1 : Client/Serveur UDP
* **`servudp.c`** : Serveur réceptionnant des messages et renvoyant un accusé de réception (AR).
* **`cliudp.c`** : Client utilisant `MSG_CONFIRM` pour l'envoi et affichant l'AR reçu.

### Étape 2 : Protocole BEUIP v1
* **`servbeuip.c`** : Serveur d'authentification et de routage. Gère la présence (Code 1), les confirmations (Code 2), le listage (Code 3) et le transfert de messages (Codes 4, 5, 9).
* **`clibeuip.c`** : Client local pour interagir avec le service via `127.0.0.1`.

## Compilation
Le projet utilise un `Makefile` configuré avec les options strictes `-Wall -Werror`.
Pour compiler :
```bash
make
