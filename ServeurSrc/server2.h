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

void do_view_historique(Client *c, char *choice);
void do_detail_partie_historique(Client *c, char *choice);
void do_view_amis(Client *c, char *choice, int nbClient, Client clients[]);
void do_view_demandes_amis(Client *c, char *choice);
void do_repondre_demande_ami(Client *c, char *choice, int nbClient, Client clients[]);
void do_spectateur(Client *c, char *choice);
void do_edit_bio(Client *c, char *choice);
void init(void);
void end(void);
void app(void);
int init_connection(void);
void end_connection(int sock);

void clear_clients(Client *clients, int actual);

void send_menu_to_client(Client * c);
void send_all_defis_to_client(Client * c);

void do_action(Client * c, char * choice, int nbClient, Client clients[]);
void do_menu(Client * c, char * choice, int nbClient, Client clients[]);
void do_choose_player(Client * c, char * choice, int nbClient, Client clients[]);
void do_look_player(Client * c, char * choice, int nbClient, Client * clients);

void do_choose_defis(Client * c, char * choice);
void do_answer_defi(Client *c, char *choice);

void do_send_message(Client *c, char *choice);



void notifier_fin_partie(Partie *p);
void notifier_joueur_tour(Partie *p);
void afficher_infos_partie(Client * c, Partie * p);
void do_choose_partie(Client *c, char *choice);

#endif /* guard */
