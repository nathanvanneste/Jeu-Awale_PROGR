#ifndef SERVER_H
#define SERVER_H

#ifdef WIN32

#include <winsock2.h>

#else

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> /* close */
#include <netdb.h> /* gethostbyname */
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;

#endif

#define CRLF        "\r\n"
#define PORT         1977
#define MAX_CLIENTS     100

#include "type.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

/**
 * Initialisation du serveur
 */
void init(void);

/**
 * Fermeture du serveur
 */
void end(void);

/**
 * Lancement et code logique du serveur
 */
void app(void);

/**
 * Ferme toutes les sockets de comm (socket du client)
 */
void clear_clients(Client *clients, int actual);

/**
 * Création de la socket d'écoute
 */
int init_connection(void);

/**
 * Fermeture de la socket d'écoute
 */
void end_connection(int sock);

/* === State manager === */

/**
 * Déclenche le bon comportement selon l'état actuel du client
 */
void do_action(Client * c, char * choice, int nbClient, Client clients[]);

/* === Gestion du menu principal ===*/

/**
 * Envoie le menu au client et le positionne dans l'état menu
 */
void send_menu_to_client(Client * c);
/**
 * Gère les inputs rentrés lorsque le menu était affiché ; déclenche le comportement adéquat selon le chiffre
 */
void do_menu(Client * c, char * choice, int nbClient, Client clients[]);

/* === 1 - liste des joueurs & looked player === */

/**
 * Envoie la liste de tous les joueurs connectés au client (excepté lui)
 */
void send_all_players_to_client(Client * c, int nbClient, Client clients[]);

/**
 * Le joueur veut plus de détail sur "input"
 * Recherche d'un client avec pseudo correspondant, sinon affichage de la liste des joueurs à nouveau
 */
void do_choose_player(Client * c, char * input, int nbClient, Client * clients);

/**
 * Affiche le menu de sélection lorsqu'on est en mode focus sur le joueur
 */
void send_look_players_to_client(Client *c);

/**
 * Lorsqu'on a choisi un joueur parmi la liste, affichage du menu des actions possibles sur ce joueur :
 * Envoyer un défi, envoyer un message, voir sa bio ou demander en ami
 */
void do_look_player(Client * c, char * choice, int nbClient, Client * clients);

/**
 * Envoi du message passé en paramètre au lookedPlayer
 */
void do_send_message(Client *c, char * input);

/* === 2 - Défis === */

/**
 * Envoie la liste de ses défis au client
 */
void send_all_defis_to_client(Client * c);
/**
 * Après avoir consulté la liste des défis et en avoir choisis un, proposer accepter/refuser ou retour menu
 * Et définir le bon état
 */
void do_choose_defis(Client * c, char * choice);

/**
 * Réponse à un défi après l'avoir sélectionné. Soit oui, soit non ou bien commande de retour ou de menu
 */
void do_answer_defi(Client *c, char *choice);

/* === 3 - Parties en cours === */

/**
 * Envoie toutes les parties en cours du client passé en paramètre
 */
void send_all_parties_to_client(Client *c);

/**
 * On récupère la partie choisie et on affiche les détails de la partie au joueur
 */
void do_choose_partie(Client *c, char *choice);

/**
 * Fonction qui gère les inputs lorsqu'un joueur est en train de joueur une partie
 */
void do_partie_en_cours(Client *c, char *input);

/* === 4 - Consulter messages === */

/**
 * Envoie de tous ses messages reçus au client passé en paramètre
 */
void send_all_messages_to_client(Client *c);

/**
 * Lorsque le client consulte ses messages
 */
void do_read_messages(Client * c, char * choice);

/* === 5 - Historique === */

/**
 *  Gère l'état VIEW_HISTORIQUE (liste des parties)
 */
void do_view_historique(Client *c, char *choice);

/**
 *  Gère l'état DETAIL_PARTIE_HISTORIQUE (détails d'une partie)
 */
void do_detail_partie_historique(Client *c, char *choice);

/* === 6 & 7 - amis & demandes d'ami === */


void do_view_amis(Client *c, char *choice, int nbClient, Client clients[]);

void do_view_demandes_amis(Client *c, char *choice);

/**
 * Permet de répondre à une demande d'ami
 */
void do_repondre_demande_ami(Client *c, char *choice);

/**
 * Lorsque le client c est dans l'état spectateur
 */
void do_spectateur(Client *c, char *choice);

/* === 8 - Bio === */

/**
 * Permet d'éditer la bio en écrasant l'ancienne par une nouvelle
 */
void do_edit_bio(Client *c, char *choice);

/* === Divers === */

/**
 * Notifie les joueurs jouant la partie de sa terminaison et, éventuellement les spectateurs
 */
void notifier_fin_partie(Partie *p);
/**
 * Notifie le joueur à qui c'est le tour qu'il doit jouer
 * Notifie également tous les spectateurs du nouvel état de la partie
 */
void notifier_joueur_tour(Partie *p); 
/**
 * Affiche les infos de la partie en paramètre pour le client passé en paramètre également
 */
void afficher_infos_partie(Client * c, Partie * p);
/**
 * Ferme la socket associée au client mais le laisse dans la liste des clients pour une éventuelle reconnexion
 */
void deconnecter_client(Client *c);

/**
 * Retourne le pointeur du client qui a le nom passé en paramètre
 */
Client *find_client_by_name(Client *clients, int nbClients, const char *name);

#endif /* guard */
