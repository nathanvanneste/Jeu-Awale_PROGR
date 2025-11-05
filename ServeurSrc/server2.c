#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "server2.h"
#include "client2.h"

static void init(void)
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

static void end(void)
{
#ifdef WIN32
   WSACleanup();
#endif
}

// Variable globale pour le moment.
Client clients[MAX_CLIENTS];
int actual = 0;

static void app(void)
{
   SOCKET sock = init_connection();
   char buffer[BUF_SIZE];
   /* the index for the array */
   int actual = 0;
   int max = sock;
   /* an array for all clients */
   

   fd_set rdfs;

   while(1)
   {
      int i = 0;
      FD_ZERO(&rdfs);

      /* add STDIN_FILENO */
      FD_SET(STDIN_FILENO, &rdfs);

      /* add the connection socket */
      FD_SET(sock, &rdfs);

      /* add socket of each client */
      for(i = 0; i < actual; i++)
      {
         FD_SET(clients[i].sock, &rdfs);
      }

      if(select(max + 1, &rdfs, NULL, NULL, NULL) == -1)
      {
         perror("select()");
         exit(errno);
      }

      /* something from standard input : i.e keyboard */
      if(FD_ISSET(STDIN_FILENO, &rdfs))
      {
         /* stop process when type on keyboard */
         break;
      }
      else if(FD_ISSET(sock, &rdfs))
      {
         /* new client */
         SOCKADDR_IN csin = { 0 };
         socklen_t sinsize = sizeof csin;
         int csock = accept(sock, (SOCKADDR *)&csin, &sinsize);
         if(csock == SOCKET_ERROR)
         {
            perror("accept()");
            continue;
         }

         /* after connecting the client sends its name */
         int name_received = read_client(csock, buffer);
         if(name_received <= 0)
         {
            /* disconnected */
            closesocket(csock);
            continue;
         }

         /* what is the new maximum fd ? */
         max = csock > max ? csock : max;

         FD_SET(csock, &rdfs);

         Client c;
         c.sock = csock;
         strncpy(c.name, buffer, BUF_SIZE - 1);
         c.etat_courant = ETAT_INIT;
         c.indiceDefis = 0;
         c.indiceParties = 0;
         c.nbMessages = 0;
         clients[actual] = c;
         
         
         
         send_menu_to_client(&clients[actual]);
         actual++;
         printf("New client connected: %s\n", buffer);
      }
      else
      {
         int i = 0;
         for(i = 0; i < actual; i++)
         {
            /* a client is talking */
            if(FD_ISSET(clients[i].sock, &rdfs))
            {
               Client client = clients[i];
               int c = read_client(clients[i].sock, buffer);
               /* client disconnected */
               if(c == 0)
               {
                  closesocket(clients[i].sock);
                  remove_client(clients, i, &actual);
                  strncpy(buffer, client.name, BUF_SIZE - 1);
                  strncat(buffer, " disconnected !", BUF_SIZE - strlen(buffer) - 1);
                  send_message_to_all_clients(clients, client, actual, buffer, 1);
                  printf("%s disconnected\n", client.name);
               }
               else
               {
                  // Ici il faut gérer selon l'état et la réponse du client
                  printf("%s: %s\n", client.name, buffer);
                  send_message_to_all_clients(clients, client, actual, buffer, 0);
                  printf("Nombre de client boucle 1 : %d \n", actual);
                  do_action(&clients[i], buffer, actual, clients);
                //  printf("Num envoyé : %d \n", atoi(buffer));
               }
               break;
            }
         }
      }
   }

   clear_clients(clients, actual);
   end_connection(sock);
}

static void clear_clients(Client *clients, int actual)
{
   int i = 0;
   for(i = 0; i < actual; i++)
   {
      closesocket(clients[i].sock);
   }
}

static void remove_client(Client *clients, int to_remove, int *actual)
{
   /* we remove the client in the array */
   memmove(clients + to_remove, clients + to_remove + 1, (*actual - to_remove - 1) * sizeof(Client));
   /* number client - 1 */
   (*actual)--;
}

static void send_message_to_all_clients(Client *clients, Client sender, int actual, const char *buffer, char from_server)
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

static int init_connection(void)
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

static void end_connection(int sock)
{
   closesocket(sock);
}

static int read_client(SOCKET sock, char *buffer)
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

static void write_client(SOCKET sock, const char *buffer)
{
   if(send(sock, buffer, strlen(buffer), 0) < 0)
   {
      perror("send()");
      exit(errno);
   }
}

static void send_menu_to_client(Client * c)
{
   c->etat_courant = ETAT_MENU;
   // Définir un état menu à envoyer au client, ou bien le garder côté serveur ?
   const char * menu = 
   "== MENU ==\n"
   "1 - Consulter la liste des joueurs\n"
   "2 - Consulter ma liste de défis\n"
   "3 - Consulter mes parties\n"
   "4 - Consulter mes messages\n"
   "5 - Quitter\n";

   write_client(c->sock, menu);
}

static void send_all_players_to_client(Client * c, int nbClient, Client clients[])
{
   // Il faut envoyer la liste des joueurs en sachant que le client peut choisir d'en défier un parmi cette liste
   
   size_t taille_totale = 0;

   for (int i = 0; i < nbClient; i++) {
      if (strcmp(c->name, clients[i].name) != 0) {
         taille_totale += strlen(clients[i].name) + 1; // +1 pour \n ou le '\0'
      }
   }

   char *chaine = calloc(taille_totale, sizeof(char));
   char *p = chaine;

   for (int i = 0; i < nbClient; i++) {
      if (strcmp(c->name, clients[i].name) != 0) {
         p += sprintf(p, "%s%s", clients[i].name, i < nbClient - 1 ? "\n" : "\0");
      }
   }

   if (p == chaine) {
      write_client(c->sock, "Il n'y a aucun autre joueur connecté ! Retour au menu. \n");
      send_menu_to_client(c);
   } else {
      write_client(c->sock, "Voici tous les joueurs connectés, rentrez un pseudo pour plus de détail :\n");
      write_client(c->sock, chaine);
      c->etat_courant = ETAT_CHOOSE_PLAYER;
   }
   free(chaine);
}

static void send_all_parties_to_client(Client *c) {
   if (c->indiceParties == 0) {
      write_client(c->sock, "Vous n'avez aucune partie en cours. Retour au menu.\n");
      send_menu_to_client(c);
      return;
   }

   // Comptons les parties
   int nbParties = c->indiceParties;
   printf("Il y a %d parties \n", c->indiceParties);

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


   strcat(buffer, "\nChoisissez le numéro de la partie à reprendre :\n");
   write_client(c->sock, buffer);

   c->etat_courant = ETAT_CHOOSE_PARTIE;
}

static void do_choose_partie(Client *c, char *choice) {
    int num = atoi(choice);
    if (num <= 0 || c->indiceParties == 0) {
        write_client(c->sock, "Numéro invalide, réessayez.\n");
        send_all_parties_to_client(c);
        return;
    }

    Partie *p = c->parties[num - 1];
    c->etat_courant = ETAT_PARTIE_EN_COURS;

    char msg[BUF_SIZE];
    Client *adv = (p->joueur1 == c) ? p->joueur2 : p->joueur1;
    snprintf(msg, sizeof(msg), "Vous jouez contre %s !\nC’est votre tour si c’est votre camp.\n", adv->name);
    write_client(c->sock, msg);

    // Appel à une fonction d’affichage de plateau
   // afficher_plateau(c, p);
}


static void send_all_defis_to_client(Client * c)
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
   write_client(c->sock, "Voici tous les joueurs vous ayant proposé un défi, rentrez un pseudo pour l'accepter ou refuser.\n");
   write_client(c->sock, chaine);
   c->etat_courant = ETAT_CHOOSE_DEFI;
   
   free(chaine);
}

static void add_client(TabDynamiqueClient * tab, Client c) {
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

static Client * get_client(TabDynamiqueClient * tab, int index) {
   if (index >= tab->length) {
      return NULL;
   }

   return &tab->tab[index];
}

static void do_action(Client * c, char * choice, int nbClient, Client clients[]) {
   printf("Nombre de client do action: %d \n", nbClient);
   printf("DO ACTION, %d\n", c->etat_courant);
   printf("CHOICE : %s \n", choice);
   switch (c->etat_courant) {
      case ETAT_INIT :
         printf("etat init");
         break;
      case ETAT_MENU :
         printf("etat menu");
         do_menu(c, choice, nbClient, clients);

         break;

      case ETAT_CHOOSE_PLAYER :
         do_choose_player(c, choice, nbClient, clients);
         
         break;
      case ETAT_LOOK_PLAYER :
         do_look_player(c, choice);
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
      case ETAT_PARTIE_EN_COURS:
         // Ici tu géreras plus tard le jeu tour par tour
         write_client(c->sock, "Le jeu n'est pas encore implémenté. Retour au menu.\n");
         send_menu_to_client(c);
         break;
      case ETAT_SEND_MESSAGE:
         do_send_message(c, choice);
         break;

      default:
         printf("default");
         break;
   }
   printf("END DO ACTION \n");
}

static void send_look_players_to_client(Client *c, Client clientLooked) {
   c->etat_courant = ETAT_LOOK_PLAYER;

   const char *template =
      "Que voulez-vous faire avec %s ?\n"
      " 1 - Le défier\n"
      " 2 - Envoyer un message\n"
      " 3 - Retour au menu\n";

   // calcul de la taille nécessaire (+1 pour le '\0')
   size_t taille = snprintf(NULL, 0, template, clientLooked.name) + 1;
   char *message = malloc(taille);
   if (!message) {
      return;
   }

   snprintf(message, taille, template, clientLooked.name);

   write_client(c->sock, message);
   free(message);
}

static void do_choose_player(Client * c, char * choice, int nbClient, Client * clients) {
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

static void do_answer_defi(Client *c, char *choice) {
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
    } else {
        write_client(c->sock, "Réponse invalide. Tapez 'oui' ou 'non'.\n");
    }
}


static void do_choose_defis(Client * c, char * choice) {
   for (int i = 0 ; i < c->indiceDefis ; ++i) {
      if (strcmp(c->defisReceive[i]->name, choice) == 0) {
         c->lookedPlayer = c->defisReceive[i];
         c->etat_courant = ETAT_ANSWER_DEFI;

         char msg[BUF_SIZE];
         snprintf(msg, sizeof(msg), "%s vous a défié ! Voulez-vous accepter ? (oui/non)\n", c->lookedPlayer->name);
         write_client(c->sock, msg);
         return;
      }
   }

   write_client(c->sock, "Le pseudo ne correspond pas à un joueur, merci de réessayer !\n");
   send_all_defis_to_client(c);
}


static void init_partie(Client * c1, Client * c2, Partie * p) {
   if (rand() % 2 == 1) {
      p->joueur1 = c1;
      p->joueur2 = c2;
   } else {
      p->joueur2 = c1;
      p->joueur1 = c2;
   }
   //Changer pour un enum avec : demandee, en cours, finie
   p->partieEnCours = false;

   p->cptJoueur1 = 0;
   p->cptJoueur2 = 0;
}

static void do_look_player(Client * c, char * choice) {
   printf("Je suis dans do_look_player");
   int num = atoi(choice);
   // TODO : Controler la valeur du choice;
   switch (num) {
      case 1 :
         printf("On va défier %s\n", c->lookedPlayer->name);

         // TODO, gérer le fait de recevoir un défi.

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

// On l'ajoute au défi
c->lookedPlayer->defisReceive[c->lookedPlayer->indiceDefis++] = c;

         write_client(c->sock, "Défi envoyé ! Retour au menu.");

         // TODO : Rentrer un nom puis accepter / refuser.


         c->lookedPlayer = NULL;
         send_menu_to_client(c);
         break;
         case 2 : 
            
            write_client(c->sock, "✉️ Entrez le message à envoyer :\n");
            c->etat_courant = ETAT_SEND_MESSAGE;
            break;
      default :
         break;
   }
}

static void do_send_message(Client *c, char *choice) {
    if (!c->lookedPlayer) {
        write_client(c->sock, "Erreur : aucun joueur sélectionné.\n");
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

    // Envoi immédiat au destinataire (si on veut un effet "live")
    char buf[BUF_SIZE * 2];
    snprintf(buf, sizeof(buf), "Nouveau message de %s : %s\n", c->name, choice);
    write_client(dest->sock, buf);

    write_client(c->sock, "Message envoyé avec succès ! Retour au menu.\n");
    send_menu_to_client(c);
}

static void send_all_messages_to_client(Client *c) {
    if (c->nbMessages == 0) {
        write_client(c->sock, "📭 Aucun message reçu.\n");
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
    c->etat_courant = ETAT_READ_MESSAGES;

    // TODO : que faire ensuite ? Proposer la possibilité de répondre ? Supprimer un message ? Retour au menu ?
}


static void do_menu(Client * c, char * choice, int nbClient, Client clients[]) {
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
      case 4: send_all_messages_to_client(c); break;
      case 5:
      // TODO, enlever de la liste etc... Ou passer en mode disconnect ?
         write_client(c->sock, "Au revoir !\n");
         closesocket(c->sock);
         break;
      default:
         write_client(c->sock, "Choix invalide. Merci de réessayer.\n");
         send_menu_to_client(c);
         break;
   }
}

int main(int argc, char **argv)
{
   init();

   app();

   end();

   return EXIT_SUCCESS;
}
