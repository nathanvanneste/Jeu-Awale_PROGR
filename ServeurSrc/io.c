#include "io.h"
#include "type.h"

#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

/**
 * On envoie buffer sur la socket. Par convention, on a donné -1 comme valeurs pour une socket "fermée"
 */
void write_client(SOCKET sock, const char *buffer)
{
   if (sock != - 1) {
      if(send(sock, buffer, strlen(buffer), 0) < 0) {
         perror("send()");
         exit(errno);
      }
   }
}

/**
 * On lit ce qui est écrit sur la socket et on l'écrit dans buffer
 * @return int nombre d'octet lu
 */
int read_client(SOCKET sock, char *buffer)
{
   int n = 0;

   if((n = recv(sock, buffer, BUF_SIZE - 1, 0)) < 0)
   {
      perror("recv()");
      /* if recv error we disconnect the client */
      n = -1;
   }

   buffer[n >= 0 ? n : 0] = 0;

   return n;
}

// === COMMANDES ===

// Fonctions utiles pour la commande menu
void write_message_menu(SOCKET sock) {
   char buffer[128];
   snprintf(buffer, sizeof(buffer), "Pour info : tapez '%s' pour retourner au menu principal\n", CMD_MENU);
   write_client(sock, buffer);
}

int strcmp_menu(const char * str) {
   return strcasecmp(str, CMD_MENU);
}

// Fonctions utiles pour la commande message
void write_message_message(SOCKET sock) {
   char buffer[128];
   snprintf(buffer, sizeof(buffer), "Pour info : tapez '%s' pour envoyer un message\n", CMD_MESSAGE);
   write_client(sock, buffer);
}

int strcmp_message(const char * str) {
   return strcasecmp(str, CMD_MESSAGE);
}

// Fonctions utiles pour la commande retour
void write_message_back(SOCKET sock) {
   char buffer[128];
   snprintf(buffer, sizeof(buffer), "Pour info : tapez '%s' pour revenir au choix précédent\n", CMD_BACK);
   write_client(sock, buffer);
}

int strcmp_back(const char * str) {
   return strcasecmp(str, CMD_BACK);
}