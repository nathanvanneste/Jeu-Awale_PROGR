#ifndef CLIENT_H
#define CLIENT_H

#include "server2.h"
#include <stdbool.h>

typedef enum {
   ETAT_INIT,      // vaut 0
   ETAT_MENU,  // vaut 1
   ETAT_2    // vaut 2
} Etat;

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
   Etat etat_courant;
   Partie ** parties;
}Client;

typedef struct
{
   char * expediteur;
   char * destinataire;
   char * contenu;
}Message;

#endif /* guard */
