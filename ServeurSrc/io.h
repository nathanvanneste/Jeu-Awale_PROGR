#ifndef IO_H
#define IO_H

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

#define CMD_MESSAGE "/message"
#define CMD_BACK "/retour"
#define CMD_MENU "/menu"

void write_client(SOCKET sock, const char *buffer);
int read_client(SOCKET sock, char *buffer);

void write_message_menu(SOCKET sock);
int strcmp_menu(const char * str);
void write_message_message(SOCKET sock);
int strcmp_message(const char * str);
void write_message_back(SOCKET sock);
int strcmp_back(const char * str);

#endif