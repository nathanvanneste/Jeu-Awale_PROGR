#ifndef TYPE_H
#define TYPE_H

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

#include <stdbool.h>

#define BUF_SIZE 1024

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
   ETAT_READ_MESSAGES,
   ETAT_VIEW_HISTORIQUE,
   ETAT_DETAIL_PARTIE_HISTORIQUE,
   ETAT_VIEW_AMIS,
   ETAT_VIEW_DEMANDES_AMIS,
   ETAT_REPONDRE_DEMANDE_AMI,
   ETAT_SPECTATEUR,
   ETAT_EDIT_BIO
} Etat;

typedef struct Client Client;
typedef struct Partie Partie;

// Représentation d'un coup joué
typedef struct {
    int numJoueur;          // 1 ou 2
    int caseJouee;          // 0-11
    int grainesCapturees;   // nombre de graines capturées ce tour
    time_t timestamp;       // horodatage du coup
} CoupHistorique;

typedef struct Partie
{
   struct Client * joueur1;
   struct Client * joueur2;
   int * plateau;
   int cptJoueur1;
   int cptJoueur2;
   int indiceJoueurActuel;
   int sensRotation;
   bool partieEnCours;
   char ** observers;
   
   // Spectateurs
   struct Client *spectateurs[20];  // max 20 spectateurs par partie
   int nbSpectateurs;
   
   // Historique
   CoupHistorique *historiqueCoups;
   int nbCoupsJoues;
   int capaciteHistorique;
}Partie;

typedef struct {
    char nomJoueur1[BUF_SIZE];
    char nomJoueur2[BUF_SIZE];
    int scoreJoueur1;
    int scoreJoueur2;
    char vainqueur[BUF_SIZE];  // nom du gagnant ou "Égalité"
    time_t dateDebut;
    time_t dateFin;
    int sensRotation;          // pour pouvoir rejouer
    int premierJoueur;         // qui a commencé
    CoupHistorique *coups;     // tableau dynamique des coups
    int nbCoups;
    int capaciteCoups;
} PartieTerminee;

typedef struct
{
   char * expediteur;
   char * destinataire;
   char * contenu;
}Message;

#define NB_MAX_DEMANDE_AMI 50
#define NB_MAX_AMI 100
typedef struct Client
{
   SOCKET sock;
   struct Client * lookedPlayer;
   char name[BUF_SIZE];
   Etat etat_courant;
   Partie * parties[1024];
   int indiceParties;
   Partie * partieEnCours;
   Client * defisReceive[1024];
   int indiceDefis;

   Message *messages[256];
   int nbMessages;

   PartieTerminee *historique[256];  // historique des parties terminées
   int nbPartiesHistorique;
   int indicePartieVisionnee;        // pour navigation dans détails

   // Système d'amis
   struct Client * amis[NB_MAX_DEMANDE_AMI];         // liste des amis (noms)
   int nbAmis;
   struct Client * demandesAmisRecues[NB_MAX_DEMANDE_AMI];  // demandes reçues (noms des demandeurs)
   int nbDemandesAmisRecues;

   // Mode spectateur
   Partie *partieSpectatee;          // partie que le client regarde en spectateur (NULL si pas spectateur)

   // Biographie
   char bio[512];                    // biographie du joueur

   bool connecte;
}Client;

#endif