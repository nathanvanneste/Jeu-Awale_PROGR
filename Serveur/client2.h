#ifndef CLIENT_H
#define CLIENT_H

#include "server2.h"
#include <stdbool.h>

typedef struct
{
   char * Joueur1;
   char * Joueur2;
   int plateau[12];
   int cptJoueur1;
   int cptJoueur2;
   char * joueurActuel;
   bool partieEnCours;
   char ** observers;
}Partie;

typedef struct
{
   SOCKET sock;
   char name[BUF_SIZE];
   Partie ** parties;
}Client;

typedef struct
{
   char * expediteur;
   char * destinataire;
   char * contenu;
}Message;

#endif /* guard */
