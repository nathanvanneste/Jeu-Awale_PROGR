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
                  do_action(clients[i], atoi(buffer), actual, clients);
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
   "== MENU ==\n 1 - Consulter la liste des joureurs\n 2 - Quitter\n";

   write_client(c->sock, menu);
}

static void send_all_players_to_client(Client c, int nbClient, Client clients[])
{
   // Il faut envoyer la liste des joueurs en sachant que le client peut choisir d'en défier un parmi cette liste
   
   size_t taille_totale = 0;

   for (int i = 0; i < nbClient; i++) {
      if (strcmp(c.name, clients[i].name) != 0) {
         taille_totale += strlen(clients[i].name) + 1; // +1 pour \n ou le '\0'
      }
   }

   char *chaine = calloc(taille_totale, sizeof(char));
   char *p = chaine;

   for (int i = 0; i < nbClient; i++) {
      if (strcmp(c.name, clients[i].name) != 0) {
         p += sprintf(p, "%s%s", clients[i].name, i < nbClient - 1 ? "\n" : "\0");
      }
   }

   if (p == chaine) {
      write_client(c.sock, "Il n'y a aucun autre joueur connecté ! \n");
   } else {
      write_client(c.sock, chaine);
   }
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

static void do_action(Client c, int choice, int nbClient, Client clients[]) {
   printf("Nombre de client do action: %d \n", nbClient);
   printf("DO ACTION, %d\n", c.etat_courant);
   printf("CHOICE : %d \n", choice);
   switch (c.etat_courant) {
      case ETAT_INIT :
         printf("etat init");
         break;
      case ETAT_MENU :
         printf("etat menu");
         do_menu(c, choice, nbClient, clients);

         break;
      default:
         printf("default");
         break;
   }
   printf("END DO ACTION \n");
}

static void do_menu(Client c, int choice, int nbClient, Client clients[]) {
   printf("Je suis dans do_menu");

   // TODO : Controler la valeur du choice;
   switch (choice) {
      case 1 :
         send_all_players_to_client(c, nbClient, clients);
         break;
      default :
   }
}

int main(int argc, char **argv)
{
   init();

   app();

   end();

   return EXIT_SUCCESS;
}
