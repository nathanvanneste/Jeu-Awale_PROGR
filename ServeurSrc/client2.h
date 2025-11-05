#ifndef CLIENT_H
#define CLIENT_H

#include "server2.h"
#include <stdbool.h>

typedef enum {
   ETAT_INIT,      // vaut 0
   ETAT_MENU,  // vaut 1
   ETAT_CHOOSE_PLAYER,    // vaut 2
   ETAT_LOOK_PLAYER,
   ETAT_CHOOSE_DEFI,
   ETAT_ANSWER_DEFI,
   ETAT_CHOOSE_PARTIE,
   ETAT_PARTIE_EN_COURS,
   ETAT_SEND_MESSAGE,
   ETAT_READ_MESSAGES
} Etat;

typedef struct Client Client;
typedef struct Partie Partie;

typedef struct Partie
{
   struct Client * joueur1;
   struct Client * joueur2;
   int plateau[12];
   int cptJoueur1;
   int cptJoueur2;
   char * joueurActuel;
   bool partieEnCours;
   char ** observers;
}Partie;

typedef struct
{
   char * expediteur;
   char * destinataire;
   char * contenu;
}Message;

typedef struct Client
{
   SOCKET sock;
   struct Client * lookedPlayer;
   char name[BUF_SIZE];
   Etat etat_courant;
   Partie * parties[1024];
   int indiceParties;
   Client * defisReceive[1024];
   int indiceDefis;

   Message *messages[256];
   int nbMessages;
}Client;


#endif /* guard */
