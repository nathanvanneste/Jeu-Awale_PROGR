#ifndef AWALE_H
#define AWALE_H

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
#include "type.h"


void plateauToString(const Partie *p, char * plateau, int taillePlateau);

bool jouerCoup(Partie * p, int numJoueur, int indiceCaseJouee);
void afficherPlateau(const Partie *p);
bool jouerCoup(Partie * p, int numJoueur, int indiceCaseJouee);
bool coupNourritAdversaire(Partie *p, int coup, int numJoueur, int adv);
bool peutNourrir(Partie *p, int joueur);
void logiqueJouer(Partie * p, int numJoueur, int indiceCaseJouee);
bool captureAffameAdversaire(const Partie *p, int numJoueur, int dernierIndice);
int totalGrainesCamp(const Partie *p, int joueur);
bool campVide(const Partie *p, int joueur);
void copyPartie(Partie * toCopy, Partie * cp);
void destroyPartie(Partie * p);
void init_partie(Client * joueur1, Client * joueur2, Partie * p);

#endif /* guard */
