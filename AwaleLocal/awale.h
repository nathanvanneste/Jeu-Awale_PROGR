#ifndef AWALE_H
#define AWALE_H

#include <stdbool.h>


// TODO : faire un tableau de joueur et garder l'info de qui doit jouer avec un cpt <= 1 ? qui sert d'indice
typedef struct
{
   char * joueur1;
   char * joueur2;
   int * plateau;
   int cptJoueur1;
   int cptJoueur2;
   char ** joueurActuel;
   int indiceJoueurActuel;
   bool partieEnCours;
   int sensRotation;
}Partie;

bool jouerCoup(Partie * p, int numJoueur, int indiceCaseJouee);

#endif /* guard */
