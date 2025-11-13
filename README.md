# Jeu-Awale_PROGR

# Bienvenue sur notre jeu d'Awale

## Lancer le jeu
Dans un terminal :

make; ./server; # pour lancer le serveur

./client IP_SERVER PSEUDO # pour rejoindre avec un client

## Règles du jeu

## Fonctionnalités présentes

Une fois connecté au serveur, un menu s'affiche détaillant les possibilités : 

- 1 : Consulter la liste des joueurs
- 2 : Consulter les défis reçus
- 3 : Consulter ses parties en cours
- 4 : Consulter ses messages reçus
- 5 : Consulter son historique des parties terminées
- 6 : Voir ses amis
- 7 : Voir les demandes d'ami reçues
- 8 : Définir sa biographie
- 9 : Quitter

### 1 : Consulter la liste des joueurs

Vous pourrez voir tous les joueurs actuellement connectés au serveur et en choisir un pour interagir avec. Parmi les interactions possibles : 
- Envoyer un défi
- Envoyer un message
- Envoyer une demande d'ami
- Consulter sa bio

### 2 : Consulter les défis reçus

Affiche tous les pseudos des joueurs vous ayant défié. 
Après avoir rentré le pseudo, il est possible d'accepter le défi, de le refuser ou de ne rien faire et de retourner au menu. Si le défi est accepté, la partie est créée et accessible depuis le menu.

Il est possible de n'envoyer qu'un seul défi à un joueur donné. Par exemple si vous avez déjà défié "joueur1", il ne sera plus possible de le défier tant qu'il n'a pas accepté ce premier défi.

### 3 : Consulter ses parties en cours

Toutes les parties en cours du joueur sont affichées. Il en sélectionne une grâce à un indice puis peut jouer cette partie. 

Pendant une partie, il peut rapidement envoyé un message à l'adversaire, retourner au menu ou bien jouer sa partie.

### 4 : Consulter ses messages reçus

Tous les messages reçus s'affichent avec l'expéditeur et le corps de ce message.

Lorsqu'un message est reçu, une notification contenant l'expéditeur et le corps du message est affiché dans le terminal.

### 5 : Consulter son historique des parties terminées

Lorsqu'un joueur a terminé une partie, il peut consulter les coups qui ont été joués et par qui. Il peut également voir combien de graines ont été capturées à chaque tour.

### 6 : Voir ses amis

Affiche tous les amis du client, ceux connectés comme ceux déconnectés.

Si parmi ceux connectés quelqu'un joue actuellement une partie, il est possible de la spectate.

### 7 : Voir les demandes d'ami reçues

Affiche toutes les demandes d'ami reçues. Il est possible de les accepter ou refuser ou bien de ne rien faire et de retourner au menu.

### 8 : Définir sa biographie

Un joueur peut ici définir sa biographie d'une taille de 500 caractères. Les autres joueurs peuvent la consulter via la liste des joueurs.

### Gestion de la déconnexion

Lorsqu'un client se déconnecte, ses informations ne sont pas perdues. Il est possible de se reconnecter en utilisant le même pseudo. Le client pourra retrouver ses parties, ses amis, ses messages...