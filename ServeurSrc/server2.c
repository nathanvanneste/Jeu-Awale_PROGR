#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "awale.h"
#include "server2.h"
#include "client2.h"


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
   snprintf(buffer, sizeof(buffer), "Pour info : tapez '%s' pour retourner au menu principal\n", CMD_MESSAGE);
   write_client(sock, buffer);
}

int strcmp_message(const char * str) {
   return strcasecmp(str, CMD_MESSAGE);
}

// Fonctions utiles pour la commande retour
void write_message_back(SOCKET sock) {
   char buffer[128];
   snprintf(buffer, sizeof(buffer), "Pour info : tapez '%s' pour retourner au menu principal\n", CMD_BACK);
   write_client(sock, buffer);
}

int strcmp_back(const char * str) {
   return strcasecmp(str, CMD_BACK);
}

Client *find_client_by_name(Client *clients, int nbClients, const char *name) {
    for (int i = 0; i < nbClients; i++) {
        if (strcasecmp(clients[i].name, name) == 0) {
            return &clients[i];
        }
    }
    return NULL;
}

void deconnecter_client(Client *c) {
    if (!c) return;

    c->connecte = false;                 
    printf("%s s’est déconnecté proprement.\n", c->name);

    write_client(c->sock, "Déconnexion en cours... À bientôt !\n");

    // On ferme juste le socket, mais on garde la structure Client et ses données
    closesocket(c->sock);

    // Par sécurité, on remet le descripteur à une valeur neutre
    c->sock = -1;
}

void init(void)
{
#ifdef WIN32
   WSADATA wsa;
   int err = WSAStartup(MAKEWORD(2, 2), &wsa);
   if(err < 0)
   {
      puts("WSAStartup failed !");
      exit(EXIT_FAILURE);
   }
#endif
}

void end(void)
{
#ifdef WIN32
   WSACleanup();
#endif
}

// Variable globale pour le moment.
Client clients[MAX_CLIENTS];
int actual = 0;

void app(void)
{
   SOCKET sock = init_connection();
   char buffer[BUF_SIZE];
   int actual = 0;
   int max = sock;
   fd_set rdfs;

   while (1)
   {
      int i = 0;
      FD_ZERO(&rdfs);

      /* add STDIN_FILENO */
      FD_SET(STDIN_FILENO, &rdfs);

      /* add the connection socket */
      FD_SET(sock, &rdfs);

      /* add socket of each connected client */
      max = sock; // on repart à zéro pour recalculer le max
      for (i = 0; i < actual; i++)
      {
         if (clients[i].connecte && clients[i].sock != -1)
         {
            FD_SET(clients[i].sock, &rdfs);
            if (clients[i].sock > max)
               max = clients[i].sock;
         }
      }

      if (select(max + 1, &rdfs, NULL, NULL, NULL) == -1)
      {
         perror("select()");
         exit(errno);
      }

      /* something from standard input : i.e keyboard */
      if (FD_ISSET(STDIN_FILENO, &rdfs))
      {
         break; // stop server manually
      }

      /*  nouvelle connexion */
      else if (FD_ISSET(sock, &rdfs))
      {
         SOCKADDR_IN csin = {0};
         socklen_t sinsize = sizeof csin;
         int csock = accept(sock, (SOCKADDR *)&csin, &sinsize);
         if (csock == SOCKET_ERROR)
         {
            perror("accept()");
            continue;
         }

         int name_received = read_client(csock, buffer);
         if (name_received <= 0)
         {
            closesocket(csock);
            continue;
         }
         buffer[name_received] = '\0';

         // Vérifier que le pseudo n'est pas une commande

         if (strcasecmp(CMD_BACK, buffer) == 0 || 
            strcasecmp(CMD_MENU, buffer) == 0 || 
            strcasecmp(CMD_MESSAGE, buffer) == 0) {
            write_client(csock, "Ce pseudo n'est pas disponible.\n");
            closesocket(csock);
            continue;
         }

         /* Vérifie si ce pseudo existe déjà */
         Client *existing = find_client_by_name(clients, actual, buffer);

         // Si un client existe déjà mais est connecté, on refuse la connexion
         if (existing != NULL && existing->connecte)
         {
            write_client(csock, "Ce pseudo est déjà utilisé par un joueur connecté.\n");
            closesocket(csock);
            continue;
         }

         if (existing != NULL)
         {
            // Reconnexion
            printf("Client %s se reconnecte.\n", buffer);
            existing->sock = csock;
            existing->connecte = true;
            existing->etat_courant = ETAT_MENU;

            char msg[BUF_SIZE];
            snprintf(msg, sizeof(msg),
                     "Heureux de vous revoir, %s ! Vous êtes reconnecté.\n", existing->name);
            write_client(csock, msg);

            send_menu_to_client(existing);
         }
         else
         {
            // Nouveau joueur
            printf("Nouveau client : %s\n", buffer);

            Client c;
            memset(&c, 0, sizeof(Client));
            c.sock = csock;
            strncpy(c.name, buffer, BUF_SIZE - 1);
            c.etat_courant = ETAT_INIT;
            c.indiceDefis = 0;
            c.indiceParties = 0;
            c.partieEnCours = NULL;
            c.nbMessages = 0;
            c.connecte = true;
            c.nbPartiesHistorique = 0;
            c.indicePartieVisionnee = -1;
            c.nbAmis = 0;
            c.nbDemandesAmisRecues = 0;
            c.partieSpectatee = NULL;       //  Mode spectateur

            clients[actual++] = c;
            write_client(c.sock, "Bienvenue !\n");
            send_menu_to_client(&clients[actual - 1]);
         }

         if (csock > max)
            max = csock;
      }

      /* messages des clients existants */
      else
      {
         for (i = 0; i < actual; i++)
         {
            if (!clients[i].connecte || clients[i].sock == -1)
               continue; // skip clients déconnectés

            if (FD_ISSET(clients[i].sock, &rdfs))
            {
               Client client = clients[i];
               int c = read_client(clients[i].sock, buffer);

               if (c == 0)
               {
                  /* déconnexion involontaire */
                  printf("%s s’est déconnecté (socket fermé)\n", client.name);
                  closesocket(clients[i].sock);
                  clients[i].connecte = false;
                  clients[i].sock = -1;
               }
               else
               {
                  printf("%s: %s\n", client.name, buffer);
                  do_action(&clients[i], buffer, actual, clients);
               }
               break;
            }
         }
      }
   }

   clear_clients(clients, actual);
   end_connection(sock);
}


void clear_clients(Client *clients, int actual)
{
   int i = 0;
   for(i = 0; i < actual; i++)
   {
      closesocket(clients[i].sock);
   }
}

void remove_client(Client *clients, int to_remove, int *actual)
{
   /* we remove the client in the array */
   memmove(clients + to_remove, clients + to_remove + 1, (*actual - to_remove - 1) * sizeof(Client));
   /* number client - 1 */
   (*actual)--;
}

void send_message_to_all_clients(Client *clients, Client sender, int actual, const char *buffer, char from_server)
{
   int i = 0;
   char message[BUF_SIZE];
   message[0] = 0;
   for(i = 0; i < actual; i++)
   {
      /* we don't send message to the sender */
      if(sender.sock != clients[i].sock)
      {
         if(from_server == 0)
         {
            strncpy(message, sender.name, BUF_SIZE - 1);
            strncat(message, " : ", sizeof message - strlen(message) - 1);
         }
         strncat(message, buffer, sizeof message - strlen(message) - 1);
         write_client(clients[i].sock, message);
      }
   }
}

int init_connection(void)
{
   SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
   SOCKADDR_IN sin = { 0 };

   if(sock == INVALID_SOCKET)
   {
      perror("socket()");
      exit(errno);
   }

   sin.sin_addr.s_addr = htonl(INADDR_ANY);
   sin.sin_port = htons(PORT);
   sin.sin_family = AF_INET;

   if(bind(sock,(SOCKADDR *) &sin, sizeof sin) == SOCKET_ERROR)
   {
      perror("bind()");
      exit(errno);
   }

   if(listen(sock, MAX_CLIENTS) == SOCKET_ERROR)
   {
      perror("listen()");
      exit(errno);
   }

   return sock;
}

void end_connection(int sock)
{
   closesocket(sock);
}

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

void write_client(SOCKET sock, const char *buffer)
{
   if(send(sock, buffer, strlen(buffer), 0) < 0)
   {
      perror("send()");
      exit(errno);
   }
}

void send_menu_to_client(Client * c)
{
   c->etat_courant = ETAT_MENU;
   // Définir un état menu à envoyer au client, ou bien le garder côté serveur ?
   const char * menu = 
   "== MENU ==\n"
   "1 - Consulter la liste des joueurs\n"
   "2 - Consulter ma liste de défis\n"
   "3 - Consulter mes parties\n"
   "4 - Consulter mes messages\n"
   "5 - Consulter mon historique\n"
   "6 - Voir mes amis en ligne\n"
   "7 - Voir mes demandes d'amis\n"
   "8 - Quitter\n";

   write_client(c->sock, menu);
}

void send_all_players_to_client(Client * c, int nbClient, Client clients[])
{
   // Il faut envoyer la liste des joueurs en sachant que le client peut choisir d'en défier un parmi cette liste
   
   size_t taille_totale = 0;

   for (int i = 0; i < nbClient; i++) {
      if (c->connecte && strcmp(c->name, clients[i].name) != 0) {
         taille_totale += strlen(clients[i].name) + 1; // +1 pour \n ou le '\0'
      }
   }

   char *chaine = calloc(taille_totale, sizeof(char));
   char *p = chaine;

   for (int i = 0; i < nbClient; i++) {
      if (c->connecte && strcmp(c->name, clients[i].name) != 0) {
         p += sprintf(p, "%s%s", clients[i].name, i < nbClient - 1 ? "\n" : "\0");
      }
   }

   if (p == chaine) {
      write_client(c->sock, "Il n'y a aucun autre joueur connecté ! Retour au menu. \n");
      send_menu_to_client(c);
   } else {
      write_client(c->sock, "Voici tous les joueurs connectés, rentrez un pseudo pour plus de détail :\n");
      write_message_menu(c->sock);
      write_client(c->sock, chaine);
      c->etat_courant = ETAT_CHOOSE_PLAYER;
   }
   free(chaine);
}

void send_all_parties_to_client(Client *c) {
   if (c->indiceParties == 0) {
      write_client(c->sock, "Vous n'avez aucune partie en cours. Retour au menu.\n");
      send_menu_to_client(c);
      return;
   }

   // Comptons les parties
   int nbParties = c->indiceParties;

   char buffer[BUF_SIZE * 2] = "";
   strcat(buffer, "== Vos parties en cours ==\n");

   for (int i = 0; i < nbParties; ++i) {
      Partie *p = c->parties[i];
      if (!p) {
         fprintf(stderr, "Erreur: partie %d NULL\n", i);
         continue;
      }
      if (!p->joueur1 || !p->joueur2) {
         fprintf(stderr, "Erreur: joueur1 ou joueur2 NULL pour partie %d\n", i);
         continue;
      }

      Client *adv = (strcmp(p->joueur1->name, c->name) == 0) ? p->joueur2 : p->joueur1;
      if (!adv) {
         fprintf(stderr, "Erreur: adversaire NULL pour partie %d\n", i);
         continue;
      }

      char ligne[BUF_SIZE];
      snprintf(ligne, sizeof(ligne), "%d - Contre %s\n", i + 1, adv->name);
      strcat(buffer, ligne);
   }


   strcat(buffer, "\nEntrez le numéro de la partie à reprendre :\n");
   write_client(c->sock, buffer);

   write_message_menu(c->sock);

   c->etat_courant = ETAT_CHOOSE_PARTIE;
}

void send_all_defis_to_client(Client * c)
{
   // Il faut envoyer la liste des joueurs en sachant que le client peut choisir d'en défier un parmi cette liste
   
   if (c->indiceDefis == 0) {
      write_client(c->sock, "Vous n'avez reçu aucun défi ! Retour au menu. \n");
      send_menu_to_client(c);
      return;
   }

   size_t taille_totale = 0;

   for (int i = 0; i < c->indiceDefis ; i++) {
      taille_totale += strlen(c->defisReceive[i]->name) + 1; // +1 pour \n ou le '\0'
   }

   char *chaine = calloc(taille_totale, sizeof(char));
   char *p = chaine;

   if (!chaine) {
      // erreur d'allocation : prévenir le client et sortir
      write_client(c->sock, "Erreur serveur : mémoire insuffisante.\n");
      send_menu_to_client(c);
      return;
   }

   for (int i = 0; i < c->indiceDefis; ++i) {
      Client *dc = c->defisReceive[i];
      if (!dc || dc->name[0] == '\0') continue;
      size_t len = snprintf(p, taille_totale + 1 - (p - chaine), "%s", dc->name);
      p += len;
      // ajout d'un saut de ligne sauf pour le dernier
      if (i < c->indiceDefis - 1) {
         *p++ = '\n';
      }
   }
   *p = '\0'; // Terminer
   write_client(
      c->sock,
      "Voici tous les joueurs vous ayant proposé un défi, rentrez un pseudo pour l'accepter ou refuser.\n"
   );
   write_message_menu(c->sock);
   write_client(c->sock, chaine);
   c->etat_courant = ETAT_CHOOSE_DEFI;
   
   free(chaine);
}

void add_client(TabDynamiqueClient * tab, Client c) {
    if (tab->size == tab->length) {
        // Alors il faut augmenter la taille.
        Client * nouvTab = calloc(tab->size * 2 + 1, sizeof(Client));
        for (int i = 0 ; i < tab->length ; ++i) {
            nouvTab[i] = tab->tab[i];
        }

        tab->size = tab->size * 2 + 1;
        free(tab->tab);
        tab->tab = nouvTab;
    }

    tab->tab[tab->length] = c;
    ++tab->length;
}

Client * get_client(TabDynamiqueClient * tab, int index) {
   if (index >= tab->length) {
      return NULL;
   }

   return &tab->tab[index];
}

void notifier_joueur_tour(Partie *p) {
    Client *joueur = (p->indiceJoueurActuel == 1) ? p->joueur1 : p->joueur2;
    char *plateau = plateauToString(p);
    char msg[BUF_SIZE * 2];
    snprintf(msg, sizeof(msg),
        "\nC'est à vous de jouer !\n%s\nChoisissez une case à jouer :\n",
        plateau);
    write_client(joueur->sock, msg);
    
    //  Notifier les spectateurs
    char msgSpectateurs[BUF_SIZE * 2];
    snprintf(msgSpectateurs, sizeof(msgSpectateurs),
        "\nC'est au tour de %s\n%s\n",
        joueur->name, plateau);
    notifier_spectateurs(p, msgSpectateurs);
}

void do_partie_en_cours(Client *c, char *input) {
    Partie *p = c->partieEnCours;
    if (!p || !p->partieEnCours) {
        write_client(c->sock, "Erreur : aucune partie en cours.\n");
        send_menu_to_client(c);
        return;
    }

    // Nettoyage de l’entrée utilisateur
    while (*input == ' ' || *input == '\t' || *input == '\n') input++;

    // Vérifie si l’utilisateur veut revenir au menu
    if (strcmp_menu(input) == 0) {
         write_client(c->sock, "Retour au menu principal...\n");
         c->etat_courant = ETAT_MENU;
         // On n'est plus "focus" sur une partie
         c->partieEnCours = NULL;
         send_menu_to_client(c);
         return;
    }

    // Vérifie si l’utilisateur veut envoyer un message
    if (strcmp_message(input) == 0) {
        Client *adv = (p->joueur1 == c) ? p->joueur2 : p->joueur1;
        if (!adv) {
            write_client(c->sock, "Erreur : adversaire introuvable.\n");
            send_menu_to_client(c);
            return;
        }
        c->lookedPlayer = adv;
        c->etat_courant = ETAT_SEND_MESSAGE;
        write_client(c->sock, "Entrez le message à envoyer à votre adversaire :\n");
        return;
    }

    // Sinon, on considère que c’est un coup de jeu
    int caseChoisie = atoi(input);
    int numJoueur = (p->joueur1 == c) ? 1 : 2;

    // Vérifie que c’est bien son tour
    if (p->indiceJoueurActuel != numJoueur) {
        write_client(c->sock, "Ce n’est pas votre tour !\n");
        return;
    }

    // Validation et exécution du coup
    if (!jouerCoup(p, numJoueur, caseChoisie)) {
         // TODO : retourner un état dans jouerCoup pour afficher le bon message d'erreur.
         write_client(c->sock, "Coup invalide. Réessayez :\n");
         return;
    }

    // Notifie les deux joueurs
    char notif[BUF_SIZE * 2];
    Client *adv = (numJoueur == 1) ? p->joueur2 : p->joueur1;
    
    char *plateau = plateauToString(p);
    snprintf(notif, sizeof(notif),
        "%s a joué la case %d.\n%s",
        c->name, caseChoisie, plateau);

    write_client(adv->sock, notif);
    write_client(c->sock, notif);
    
    //  Notifier les spectateurs
    notifier_spectateurs(p, notif);

    // Si la partie continue
    if (p->partieEnCours) {
        notifier_joueur_tour(p);
    } else {
        notifier_fin_partie(p);
    }
}

void do_read_messages(Client * c, char * choice) {
   if (strcmp_menu(choice) == 0) {
      send_menu_to_client(c);
      return;
   }

   write_client(c->sock, "Invalide. Pour le moment il n'y a pas d'autre choix que de retourner au menu.\n");
}

/**
 * Déclenche le bon comportement selon l'état actuel du client
 */
void do_action(Client * c, char * choice, int nbClient, Client clients[]) {
   switch (c->etat_courant) {
      case ETAT_INIT :
         break;
      case ETAT_MENU :
         do_menu(c, choice, nbClient, clients);
         break;
      case ETAT_CHOOSE_PLAYER :
         do_choose_player(c, choice, nbClient, clients);
         break;
      case ETAT_LOOK_PLAYER :
         do_look_player(c, choice, nbClient, clients);
         break;
      case ETAT_CHOOSE_DEFI:
         do_choose_defis(c, choice);
         break;
      case ETAT_ANSWER_DEFI:
         do_answer_defi(c, choice);
         break;
      case ETAT_CHOOSE_PARTIE:
         do_choose_partie(c, choice);
         break;
      case ETAT_SEND_MESSAGE:
         do_send_message(c, choice);
         break;
      case ETAT_PARTIE_EN_COURS:
         do_partie_en_cours(c, choice);
         break;
      case ETAT_READ_MESSAGES:
         do_read_messages(c, choice);
         break;
      case ETAT_VIEW_HISTORIQUE:
         do_view_historique(c, choice);
         break;
      case ETAT_DETAIL_PARTIE_HISTORIQUE:
         do_detail_partie_historique(c, choice);
         break;
      case ETAT_VIEW_AMIS:
         do_view_amis(c, choice, nbClient, clients);
         break;
      case ETAT_VIEW_DEMANDES_AMIS:
         do_view_demandes_amis(c, choice);
         break;
      case ETAT_REPONDRE_DEMANDE_AMI:
         do_repondre_demande_ami(c, choice, nbClient, clients);
         break;
      case ETAT_SPECTATEUR:
         do_spectateur(c, choice);
         break;
      default:
         break;
   }
}

/**
 * Affiche le menu de sélection lorsqu'on est en mode focus sur le joueur
 */
void send_look_players_to_client(Client *c, Client clientLooked) {
   c->etat_courant = ETAT_LOOK_PLAYER;

   char buffer[BUF_SIZE * 2];
   
   // Vérifier si déjà ami
   bool dejaAmi = est_ami(c, clientLooked.name);
   
   if (dejaAmi) {
      snprintf(buffer, sizeof(buffer),
         "Que voulez-vous faire avec %s ? [AMI ✓]\n"
         " 1 - Le défier\n"
         " 2 - Envoyer un message\n",
         clientLooked.name);
   } else {
      snprintf(buffer, sizeof(buffer),
         "Que voulez-vous faire avec %s ?\n"
         " 1 - Le défier\n"
         " 2 - Envoyer un message\n"
         " 3 - Envoyer une demande d'ami\n",
         clientLooked.name);
   }

   write_client(c->sock, buffer);
   write_message_menu(c->sock);
   write_message_back(c->sock);
}

void do_choose_player(Client * c, char * choice, int nbClient, Client * clients) {
   // Retour au menu
   if (strcmp_menu(choice) == 0) {
      send_menu_to_client(c);
      return;
   }

   // Ici client a challengé choice
   for (int i = 0 ; i < nbClient ; ++i) {
      if (strcmp(clients[i].name, choice) == 0) {
         // C'est lui qu'on challenge
         // On passe en état LOOK_PLAYER
         c->lookedPlayer = &clients[i];
         send_look_players_to_client(c, clients[i]);
         return;
      }
   }

   // Pas trouvé donc on renvoie la liste des joueurs
   write_client(c->sock, "Le pseudo ne correspond pas à un joueur, merci de réeassayer !");
   send_all_players_to_client(c, nbClient, clients);
}

void do_answer_defi(Client *c, char *choice) {
    if (!c->lookedPlayer) {
        write_client(c->sock, "Erreur : aucun défi sélectionné.\n");
        send_menu_to_client(c);
        return;
    }

    if (strcasecmp(choice, "oui") == 0) {
        // Création d’une partie
        Partie *p = malloc(sizeof(Partie));
        if (!p) {
            write_client(c->sock, "Erreur mémoire lors de la création de la partie.\n");
            send_menu_to_client(c);
            return;
        }

        init_partie(c, c->lookedPlayer, p);
        // TODO : gérer les parties.
        //c->parties = malloc(sizeof(Partie*));
        //c->parties[0] = p;
        c->parties[c->indiceParties++] = p;
        //c->lookedPlayer->parties = malloc(sizeof(Partie*));
        //c->lookedPlayer->parties[0] = p;
        c->lookedPlayer->parties[c->lookedPlayer->indiceParties++] = p;

         // Message de confirmation
        char msg[BUF_SIZE];
        snprintf(msg, sizeof(msg), "Vous avez accepté le défi de %s ! La partie commence.\n", c->lookedPlayer->name);
        write_client(c->sock, msg);

        snprintf(msg, sizeof(msg), "%s a accepté votre défi ! La partie commence.\n", c->name);
        write_client(c->lookedPlayer->sock, msg);

        // Supprimer le défi de la liste
        for (int i = 0; i < c->indiceDefis; ++i) {
            if (c->defisReceive[i] == c->lookedPlayer) {
                memmove(&c->defisReceive[i], &c->defisReceive[i + 1],
                        (c->indiceDefis - i - 1) * sizeof(Client*));
                c->indiceDefis--;
                break;
            }
        }

        // Nettoyage du défi
        c->lookedPlayer = NULL;
        c->etat_courant = ETAT_MENU;
        send_menu_to_client(c);

    } else if (strcasecmp(choice, "non") == 0) {
        // Défi refusé
        char msg[BUF_SIZE];
        snprintf(msg, sizeof(msg), "Vous avez refusé le défi de %s.\n", c->lookedPlayer->name);
        write_client(c->sock, msg);

        snprintf(msg, sizeof(msg), "%s a refusé votre défi.\n", c->name);
        write_client(c->lookedPlayer->sock, msg);

        // Supprimer le défi de la liste
        for (int i = 0; i < c->indiceDefis; ++i) {
            if (c->defisReceive[i] == c->lookedPlayer) {
                memmove(&c->defisReceive[i], &c->defisReceive[i + 1],
                        (c->indiceDefis - i - 1) * sizeof(Client*));
                c->indiceDefis--;
                break;
            }
        }

        c->lookedPlayer = NULL;
        send_menu_to_client(c);
    } else if (strcmp_back(choice) == 0) {
      c->lookedPlayer = NULL;
      c->etat_courant = ETAT_CHOOSE_DEFI;
      send_all_defis_to_client(c);
    } else if (strcmp_menu(choice) == 0) {
      c->lookedPlayer = NULL;
      c->etat_courant = ETAT_MENU;
      send_menu_to_client(c);
    } else {
        write_client(c->sock, "Réponse invalide. Tapez 'oui', 'non', 'menu' ou 'retour'.\n");
    }
}

void do_choose_defis(Client * c, char * choice) {
   // Retour au menu
   if (strcmp_menu(choice) == 0) {
      send_menu_to_client(c);      
      return;
   }

   for (int i = 0 ; i < c->indiceDefis ; ++i) {
      if (strcmp(c->defisReceive[i]->name, choice) == 0) {
         c->lookedPlayer = c->defisReceive[i];
         c->etat_courant = ETAT_ANSWER_DEFI;

         char msg[BUF_SIZE];
         snprintf(msg, sizeof(msg), "%s vous a défié ! Voulez-vous accepter ? (oui/non)\n", c->lookedPlayer->name);
         write_client(c->sock, msg);
         write_message_menu(c->sock);
         write_message_back(c->sock);
         return;
      }
   }

   write_client(c->sock, "Le pseudo ne correspond pas à un joueur, merci de réessayer !\n");
   send_all_defis_to_client(c);
}


void do_look_player(Client * c, char * choice, int nbClient, Client * clients) {
   printf("Je suis dans do_look_player");

   // Retour au menu
   if (strcmp_menu(choice) == 0) {
      send_menu_to_client(c);
      return;
   }

   if (strcmp_back(choice) == 0) {
      send_all_players_to_client(c, nbClient, clients);
      return;
   }

   int num = atoi(choice);
   // TODO : Controler la valeur du choice;
   switch (num) {
      case 1 :
         printf("On va défier %s\n", c->lookedPlayer->name);

         // Vérification qu'il n'y a pas déjà un défi envoyé par ce joueur vers le lookedPlayer
         for (int i = 0 ; i < c->lookedPlayer->indiceDefis ; ++i) {
            if (strcmp(c->lookedPlayer->defisReceive[i]->name, c->name) == 0) {
               write_client(c->sock, "Vous avez déjà envoyé un défi à ce joueur ! Il faut attendre sa réponse avant de pouvoir l'inviter à nouveau.\n");
               return;
            }
         }

         // On l'ajoute au défi
         c->lookedPlayer->defisReceive[c->lookedPlayer->indiceDefis++] = c;

         const char *template = "Défi envoyé à %s !\n";
         // calcul de la taille nécessaire (+1 pour le '\0')
         size_t taille = snprintf(NULL, 0, template, c->lookedPlayer->name) + 1;
         char *message = malloc(taille);
         if (!message) {
            return;
         }
         snprintf(message, taille, template, c->lookedPlayer->name);
         write_client(c->sock, message);

         const char * template2 = "%s vous a envoyé un défi !\n";

         // calcul de la taille nécessaire (+1 pour le '\0')
         size_t taille2 = snprintf(NULL, 0, template2, c->name) + 1;
         char *message2 = malloc(taille);
         if (!message2) {
            return;
         }
         snprintf(message2, taille2, template2, c->name);
         write_client(c->lookedPlayer->sock, message2);
         write_client(c->sock, "Défi envoyé ! Retour au menu.\n");

         c->lookedPlayer = NULL;
         send_menu_to_client(c);
         break;
      case 2 :             
         write_client(c->sock, "Entrez le message à envoyer :\n");
         write_message_menu(c->sock);
         c->etat_courant = ETAT_SEND_MESSAGE;
         break;
      case 3 :
         // Envoyer une demande d'ami
         envoyer_demande_ami(c, c->lookedPlayer);
         c->lookedPlayer = NULL;
         send_menu_to_client(c);
         break;
      default :
         break;
   }
}

void do_send_message(Client *c, char *choice) {
    if (!c->lookedPlayer) {
        write_client(c->sock, "Erreur : aucun joueur sélectionné.\n");
        send_menu_to_client(c);
        return;
    }

    // si on veut retourner au menu.
    if (strcmp_menu(choice) == 0) {
      c->etat_courant = ETAT_MENU;
      send_menu_to_client(c);
      return;
    }

    // Création du message
    Message *m = malloc(sizeof(Message));
    m->expediteur = strdup(c->name);
    m->destinataire = strdup(c->lookedPlayer->name);
    m->contenu = strdup(choice);

    Client *dest = c->lookedPlayer;
    dest->messages[dest->nbMessages++] = m;

    // Envoi immédiat au destinataire
    char buf[BUF_SIZE * 2];
    snprintf(buf, sizeof(buf), "Nouveau message de %s : %s\n", c->name, choice);
    write_client(dest->sock, buf);

    // Si on vient d'une partie, il faut retourner dessus.
    if (c->partieEnCours != NULL) {
      // on se met alors dans le bon état et on affiche la partie.
      write_client(c->sock, "Message envoyé avec succès ! Retour à la partie.\n");
      c->etat_courant = ETAT_PARTIE_EN_COURS;
      afficher_infos_partie(c, c->partieEnCours);
      return;
    }

    write_client(c->sock, "Message envoyé avec succès ! Retour au menu.\n");
    send_menu_to_client(c);
}

void send_all_messages_to_client(Client *c) {
    if (c->nbMessages == 0) {
        write_client(c->sock, "Aucun message reçu.\n");
        send_menu_to_client(c);
        return;
    }

    char buffer[BUF_SIZE * 4] = "== Vos messages ==\n";
    for (int i = 0; i < c->nbMessages; ++i) {
        Message *m = c->messages[i];
        char ligne[BUF_SIZE];
        snprintf(ligne, sizeof(ligne), "De %s : %s\n", m->expediteur, m->contenu);
        strcat(buffer, ligne);
    }

    write_client(c->sock, buffer);
    write_message_menu(c->sock);
    c->etat_courant = ETAT_READ_MESSAGES;

    // TODO : que faire ensuite ? Proposer la possibilité de répondre ? Supprimer un message ? Retour au menu ?
}

void notifier_fin_partie(Partie *p) {
    char msg[BUF_SIZE];
    snprintf(msg, sizeof(msg),
        "\n Partie terminée !\n"
        "%s : %d graines\n"
        "%s : %d graines\n",
        p->joueur1->name, p->cptJoueur1,
        p->joueur2->name, p->cptJoueur2);

    write_client(p->joueur1->sock, msg);
    write_client(p->joueur2->sock, msg);
    
    //  Notifier les spectateurs
    notifier_spectateurs(p, msg);
    
    // Déconnecter les spectateurs
    for (int i = p->nbSpectateurs - 1; i >= 0; i--) {
        if (p->spectateurs[i]) {
            write_client(p->spectateurs[i]->sock, "La partie est terminée. Retour au menu.\n");
            p->spectateurs[i]->partieSpectatee = NULL;
            send_menu_to_client(p->spectateurs[i]);
        }
    }
    p->nbSpectateurs = 0;

    send_menu_to_client(p->joueur1);
    send_menu_to_client(p->joueur2);
}

void afficher_infos_partie(Client * c, Partie * p) {
   
   char msg[BUF_SIZE * 2];
   Client *adv = (p->joueur1 == c) ? p->joueur2 : p->joueur1;
   
   char *plateau = plateauToString(p);
   snprintf(msg, sizeof(msg),
      "Vous jouez contre %s.\n\n%s",
      adv->name, plateau);
   write_client(c->sock, msg);

   // On affiche les commandes existantes
   write_client(
      c->sock, 
      "Pour envoyer un message à votre adversaire, tapez \"message\"\nPour retourner au menu, tapez \"menu\"\n"
   );

   // Si c’est son tour
   if ((p->indiceJoueurActuel == 1 && p->joueur1 == c) ||
      (p->indiceJoueurActuel == 2 && p->joueur2 == c)) {
      write_client(c->sock, "\nC’est votre tour ! Choisissez une case à jouer :\n");
   } else {
      write_client(c->sock, "\nC’est au tour de votre adversaire. Patientez.\n");
   } 
}

/**
 * On récupère la partie choisie et on affiche les détails de la partie au joueur
 */
void do_choose_partie(Client *c, char *choice) {
   // Retour au menu
   if (strcmp_menu(choice) == 0) {
      send_menu_to_client(c);
      return;
   }

   int num = atoi(choice);

   if (num <= 0 || num > c->indiceParties) {
      write_client(c->sock, "Numéro invalide, réessayez.\n");
      send_all_parties_to_client(c);
      return;
   }

   Partie *p = c->parties[num - 1];
   c->etat_courant = ETAT_PARTIE_EN_COURS;
   c->partieEnCours = p;

   afficher_infos_partie(c, p);
}

/**
 *  Gère l'état VIEW_HISTORIQUE (liste des parties)
 */
void do_view_historique(Client *c, char *choice) {
   // Retour au menu
   if (strcmp_menu(choice) == 0) {
      send_menu_to_client(c);
      return;
   }
   
   // Vérifier si c'est un numéro de partie
   int num = atoi(choice);
   if (num > 0 && num <= c->nbPartiesHistorique) {
      // Afficher le détail de cette partie
      afficher_detail_partie_historique(c, num - 1);
      c->etat_courant = ETAT_DETAIL_PARTIE_HISTORIQUE;
      c->indicePartieVisionnee = num - 1;
   } else {
      write_client(c->sock, "Numéro invalide. Réessayez.\n");
      afficher_historique_parties(c);
   }
}

/**
 *  Gère l'état DETAIL_PARTIE_HISTORIQUE (détails d'une partie)
 */
void do_detail_partie_historique(Client *c, char *choice) {
   // Retour au menu
   if (strcmp_menu(choice) == 0) {
      send_menu_to_client(c);
      return;
   }
   
   // Retour à la liste d'historique
   if (strcmp_back(choice) == 0) {
      afficher_historique_parties(c);
      c->etat_courant = ETAT_VIEW_HISTORIQUE;
      return;
   }
   
   // Replay de la partie
   if (strcasecmp(choice, "replay") == 0) {
      if (c->indicePartieVisionnee >= 0 && 
          c->indicePartieVisionnee < c->nbPartiesHistorique) {
         
         PartieTerminee *partie = c->historique[c->indicePartieVisionnee];
         rejouer_partie_en_accelere(c, partie);
      }
      
      // Réafficher les détails
      afficher_detail_partie_historique(c, c->indicePartieVisionnee);
      return;
   }
   
   write_client(c->sock, "Commande non reconnue. ('menu', 'retour' ou 'replay')\n");
}

// ========== GESTION DES AMIS ==========

void do_view_amis(Client *c, char *choice, int nbClient, Client clients[]) {
   // Retour au menu
   if (strcmp_menu(choice) == 0) {
      send_menu_to_client(c);
      return;
   }
   
   // Vérifier si c'est un numéro (pour spectater)
   int num = atoi(choice);
   if (num > 0) {
      // Trouver l'ami en partie correspondant
      int numeroAffichage = 1;
      for (int i = 0; i < c->nbAmis; i++) {
         for (int j = 0; j < nbClient; j++) {
            if (strcmp(clients[j].name, c->amis[i]) == 0 && 
                clients[j].connecte &&
                clients[j].etat_courant == ETAT_PARTIE_EN_COURS) {
               
               if (numeroAffichage == num) {
                  // Trouvé ! Rejoindre comme spectateur
                  if (clients[j].partieEnCours) {
                     if (rejoindre_comme_spectateur(c, clients[j].partieEnCours)) {
                        return; // Le client est maintenant en mode spectateur
                     }
                  } else {
                     write_client(c->sock, "Erreur : partie introuvable.\n");
                  }
                  afficher_liste_amis(c, clients, nbClient);
                  return;
               }
               numeroAffichage++;
               break;
            }
         }
      }
      write_client(c->sock, "Numéro invalide.\n");
   }
   
   // Réafficher la liste
   afficher_liste_amis(c, clients, nbClient);
}

void do_view_demandes_amis(Client *c, char *choice) {
   // Retour au menu
   if (strcmp_menu(choice) == 0) {
      send_menu_to_client(c);
      return;
   }
   
   // Vérifier si c'est un numéro
   int num = atoi(choice);
   if (num > 0 && num <= c->nbDemandesAmisRecues) {
      // Demander accepter ou refuser
      c->etat_courant = ETAT_REPONDRE_DEMANDE_AMI;
      c->indicePartieVisionnee = num - 1;  // Réutiliser ce champ pour stocker l'indice
      
      char buffer[BUF_SIZE];
      snprintf(buffer, sizeof(buffer), 
               "Répondre à la demande de %s ?\n"
               "Tapez 'oui' pour accepter, 'non' pour refuser\n",
               c->demandesAmisRecues[num - 1]);
      write_client(c->sock, buffer);
      write_message_menu(c->sock);
      write_message_back(c->sock);
      return;
   }
   
   write_client(c->sock, "Numéro invalide. Tapez 'menu' pour retour.\n");
}

void do_repondre_demande_ami(Client *c, char *choice, int nbClient, Client clients[]) {
   // Retour au menu
   if (strcmp_menu(choice) == 0) {
      send_menu_to_client(c);
      return;
   }
   
   // Retour aux demandes
   if (strcmp_back(choice) == 0) {
      afficher_demandes_amis(c);
      c->etat_courant = ETAT_VIEW_DEMANDES_AMIS;
      return;
   }
   
   if (c->indicePartieVisionnee < 0 || c->indicePartieVisionnee >= c->nbDemandesAmisRecues) {
      write_client(c->sock, "Erreur : demande invalide.\n");
      send_menu_to_client(c);
      return;
   }
   
   const char *nomDemandeur = c->demandesAmisRecues[c->indicePartieVisionnee];
   
   if (strcasecmp(choice, "oui") == 0) {
      accepter_demande_ami(c, nomDemandeur, clients, nbClient);
      c->indicePartieVisionnee = -1;
      send_menu_to_client(c);
   } else if (strcasecmp(choice, "non") == 0) {
      refuser_demande_ami(c, nomDemandeur);
      c->indicePartieVisionnee = -1;
      send_menu_to_client(c);
   } else {
      write_client(c->sock, "Réponse invalide. Tapez 'oui', 'non', 'menu' ou 'retour'.\n");
   }
}

void do_spectateur(Client *c, char *choice) {
   // Quitter le mode spectateur
   if (strcmp_menu(choice) == 0) {
      quitter_mode_spectateur(c);
      send_menu_to_client(c);
      return;
   }
   
   // En mode spectateur, on ignore les autres commandes
   // Le spectateur reçoit automatiquement les mises à jour de la partie
   write_client(c->sock, "Mode spectateur : vous ne pouvez pas interagir. Tapez '/menu' pour quitter.\n");
}

void do_menu(Client * c, char * choice, int nbClient, Client clients[]) {
   printf("Je suis dans do_menu");
   int num = atoi(choice);
   // TODO : Controler la valeur du choice;🤣
   switch (num) {
      case 1 :
         send_all_players_to_client(c, nbClient, clients);
         break;
      case 2 :
         send_all_defis_to_client(c);
         break;
      case 3:
         send_all_parties_to_client(c);
         break;
      case 4: 
         send_all_messages_to_client(c);
         break;
      case 5:
         // Historique des parties
         afficher_historique_parties(c);
         c->etat_courant = ETAT_VIEW_HISTORIQUE;
         break;
      case 6:
         //  Liste des amis
         afficher_liste_amis(c, clients, nbClient);
         c->etat_courant = ETAT_VIEW_AMIS;
         break;
      case 7:
         //  Demandes d'amis
         afficher_demandes_amis(c);
         c->etat_courant = ETAT_VIEW_DEMANDES_AMIS;
         break;
      case 8:
         deconnecter_client(c);
         break;
      default:
         write_client(c->sock, "Choix invalide. Merci de réessayer.\n");
         send_menu_to_client(c);
         break;
   }
}

int main(void)
{
   init();

   app();

   end();

   return EXIT_SUCCESS;
}
