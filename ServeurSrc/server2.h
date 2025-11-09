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

#define BUF_SIZE    1024

#define CMD_MESSAGE "/message"
#define CMD_BACK "/retour"
#define CMD_MENU "/menu"

#include "client2.h"

static void init(void);
static void end(void);
static void app(void);
static int init_connection(void);
static void end_connection(int sock);
static int read_client(SOCKET sock, char *buffer);
void write_client(SOCKET sock, const char *buffer);
static void send_message_to_all_clients(Client *clients, Client client, int actual, const char *buffer, char from_server);
static void remove_client(Client *clients, int to_remove, int *actual);
static void clear_clients(Client *clients, int actual);

static void send_menu_to_client(Client * c);
static void send_all_defis_to_client(Client * c);

static void do_action(Client * c, char * choice, int nbClient, Client clients[]);
static void do_menu(Client * c, char * choice, int nbClient, Client clients[]);
static void do_choose_player(Client * c, char * choice, int nbClient, Client clients[]);
static void do_look_player(Client * c, char * choice, int nbClient, Client * clients);

static void do_choose_defis(Client * c, char * choice);
static void do_answer_defi(Client *c, char *choice);

static void do_send_message(Client *c, char *choice);


char *plateauToString(const Partie *p);

void notifier_fin_partie(Partie *p);
static void afficher_infos_partie(Client * c, Partie * p);
static void do_choose_partie(Client *c, char *choice);

typedef struct
{
    int size;
    int length;
    Client * tab;
}TabDynamiqueClient;

static void add_client(TabDynamiqueClient * tab, Client c);

static Client * get_client(TabDynamiqueClient * tab, int index);

#endif /* guard */
