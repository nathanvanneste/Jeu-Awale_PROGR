#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "awale.h"

void initPartie(Partie * p, const char * joueur1, const char * joueur2) 
{

    p->sensRotation = (rand() % 2) * 2 - 1; // renvoie -1 ou +1 : (0 ou 1) * 2 donc (0 ou 2) - 1

    p->indiceJoueurActuel = rand() % 2 + 1;

    p->cptJoueur1 = 0;
    p->cptJoueur2 = 0;

    p->plateau = calloc(12, sizeof(int));

    for (int i = 0; i < 12 ; ++i) {
        p->plateau[i] = 0;
    }

    p->joueur1 = malloc(strlen(joueur1) + 1);
    strcpy(p->joueur1, joueur1);

    p->joueur2 = malloc(strlen(joueur2) + 1);
    strcpy(p->joueur2, joueur2);
}

void destroyPartie(Partie * p)
{
    free(p->plateau);
    free(p->joueur1);
    free(p->joueur2);
}

void copyPartie(Partie * toCopy, Partie * cp)
{
    cp->partieEnCours = toCopy->partieEnCours;
    
    cp->sensRotation = toCopy->sensRotation;
    
    cp->indiceJoueurActuel = toCopy->indiceJoueurActuel;

    cp->cptJoueur1 = toCopy->cptJoueur1;
    cp->cptJoueur2 = toCopy->cptJoueur2;

    cp->plateau = calloc(12, sizeof(int));
    for (int i = 0; i < 12 ; ++i) {
        cp->plateau[i] = toCopy->plateau[i];
    }

    cp->joueur1 = malloc(strlen(toCopy->joueur1) + 1);
    strcpy(cp->joueur1, toCopy->joueur1);

    cp->joueur2 = malloc(strlen(toCopy->joueur2) + 1);
    strcpy(cp->joueur2, toCopy->joueur2);
}

bool campVide(const Partie *p, int joueur) {
    int debut = (joueur == 1) ? 0 : 6;
    int fin = (joueur == 1) ? 6 : 12;
    for (int i = debut; i < fin; ++i)
    {
        if (p->plateau[i] > 0) {
            return false;
        }
    }
    
    return true;
}

int totalGrainesCamp(const Partie *p, int joueur) {
    int debut = (joueur == 1) ? 0 : 6;
    int fin = (joueur == 1) ? 6 : 12;
    int total = 0;
    for (int i = debut; i < fin; ++i) {
        total += p->plateau[i];
    }
    return total;
}

//  Vérifie si une capture affamerait complètement l’adversaire 
bool captureAffameAdversaire(const Partie *p, int numJoueur, int dernierIndice) {
    int adv = (numJoueur == 1) ? 2 : 1;
    int grainesAdvAvant = totalGrainesCamp(p, adv);
    if (grainesAdvAvant == 0) {
        return false;
    }

    int testIndice = dernierIndice;
    int grainesCaptureesPotentielles = 0;
    while ((p->plateau[testIndice] == 2 || p->plateau[testIndice] == 3) &&
           ((numJoueur == 2 && testIndice < 6) || (numJoueur == 1 && testIndice >= 6))) {
        grainesCaptureesPotentielles += p->plateau[testIndice];
        testIndice = (testIndice - p->sensRotation + 12) % 12;
    }

    return (grainesCaptureesPotentielles == grainesAdvAvant);
}

void logiqueJouer(Partie * p, int numJoueur, int indiceCaseJouee) {
    int graines = p->plateau[indiceCaseJouee];
    p->plateau[indiceCaseJouee] = 0;
    int dernierIndice = indiceCaseJouee;

    for (int i = 0; i < graines; ++i) {
        dernierIndice = (dernierIndice + p->sensRotation + 12) % 12;

        // Si on fait un tour complet, on ne redépose pas dans le trou d’origine
        if (dernierIndice == indiceCaseJouee) {
            dernierIndice = (dernierIndice + p->sensRotation + 12) % 12;
        }

        ++p->plateau[dernierIndice];
    }

    p->plateau[indiceCaseJouee] = 0;

    // Vérifie l’affamement avant capture 
    if (captureAffameAdversaire(p, numJoueur, dernierIndice)) {
        printf("Coup affamant détecté : capture annulée.\n");
        p->indiceJoueurActuel = (p->indiceJoueurActuel % 2) + 1;
        return;
    }

    int * cpt = numJoueur == 1 ? &(p->cptJoueur1) : &(p->cptJoueur2);
        
    // On parcourt depuis la dernière case dans le sens inverse
    while ((p->plateau[dernierIndice] == 2 || p->plateau[dernierIndice] == 3) &&
        ((numJoueur == 2 && dernierIndice < 6) || (numJoueur == 1 && dernierIndice >= 6))) {
        *cpt += p->plateau[dernierIndice];
        
        p->plateau[dernierIndice] = 0;
        dernierIndice = (dernierIndice + p->sensRotation * -1 + 12) % 12;
    }
}

//  Vérifie si le joueur peut nourrir l’adversaire (simulateur de coups) 
bool peutNourrir(Partie *p, int joueur) {
    for (int i = 0; i < 6; ++i) {
        int caseJoueur = (joueur == 1) ? i : i + 6;
        if (p->plateau[caseJoueur] == 0) {
            continue;
        }

        Partie tmp;
        copyPartie(p, &tmp);
        logiqueJouer(&tmp, joueur, caseJoueur);
        bool nourri = !campVide(&tmp, (joueur == 1) ? 2 : 1);
        destroyPartie(&tmp);

        if (nourri) {
            return true;
        }
    }
    return false;
}

bool coupNourritAdversaire(Partie *p, int coup, int numJoueur, int adv) {
    Partie copie;
    copyPartie(p, &copie); // copie profonde
    logiqueJouer(&copie, numJoueur, coup);
    return !campVide(&copie, adv);
}

/**
* TODO : retourner un entier et indiquer grâce à cet entier le statut du coup qui vient d'être joué : 
* - à rejouer car incorrect (mauvais choix entier, pas son tour, devoir de nourrir adversaire), - partie finie pour différentes causes

* Un joueur peut jouer uniquement de son côté donc 0-5 pour j1 et 6-11 pour j2
*
* Si un coup devait prendre toutes les graines adverses, alors le coup peut être joué, 
* mais aucune capture n'est faite : il ne faut pas « affamer » l'adversaire.
*
* On ne saute pas de case lorsqu'on égrène sauf lorsqu'on a douze graines ou plus, c'est-à-dire 
* qu'on fait un tour complet : on ne remplit pas la case dans laquelle on vient de prendre les graines.
* 
* Il faut « nourrir » l'adversaire, c'est-à-dire que, quand celui-ci n'a plus de graines, il 
* faut absolument jouer un coup qui lui permette de rejouer ensuite. Si ce n'est pas possible, 
* la partie s'arrête et le joueur qui allait jouer capture les graines restantes.
*/
bool jouerCoup(Partie * p, int numJoueur, int indiceCaseJouee) {

    if (!p->partieEnCours || numJoueur != p->indiceJoueurActuel ||
    (numJoueur == 1 && (indiceCaseJouee < 0 || indiceCaseJouee > 5)) ||
    (numJoueur == 2 && (indiceCaseJouee < 6 || indiceCaseJouee > 11))) {
        printf("Erreur, coup invalide !");
        return false;
    }

    if (p->plateau[indiceCaseJouee] == 0) {
        return false;
    }

    int adv = (numJoueur == 1) ? 2 : 1;

    if (campVide(p, adv) && !coupNourritAdversaire(p, indiceCaseJouee, numJoueur, adv)) {
        printf("Il faut jouer un coup pour nourrir l'adversaire !\n");
        return false;
    }

    logiqueJouer(p, numJoueur, indiceCaseJouee);

    p->indiceJoueurActuel = (p->indiceJoueurActuel % 2) + 1;

    //Vérifier que l'adversaire ne peut plus le nourrir si camp vide?
    //  Conditions de fin de partie 
    if (p->cptJoueur1 >= 25 || p->cptJoueur2 >= 25) {
        printf("Fin de partie : un joueur a atteint 25 graines.\n");
        p->partieEnCours = false;
    } else if (campVide(p, 1) && !peutNourrir(p, 2)) {
        printf("Le camp du joueur 1 est vide et ne peut plus être nourri : fin de partie.\n");
        int *cpt = &p->cptJoueur2;
        for (int i = 0; i < 12; ++i) {
            *cpt += p->plateau[i];
            p->plateau[i] = 0;
        }
        p->partieEnCours = false;
    } else if (campVide(p, 2) && !peutNourrir(p,1)) {
        printf("Le camp du joueur 2 est vide et ne peut plus être nourri : fin de partie.\n");
        int *cpt = &p->cptJoueur1;
        for (int i = 0; i < 12; ++i) {
            *cpt += p->plateau[i];
            p->plateau[i] = 0;
        }
        p->partieEnCours = false;
    }

    return true;
}

void afficherPlateau(const Partie *p)
{
    printf("\nPlateau actuel :\n");
    printf("   [Joueur 2 : %s]\n   ", p->joueur2);
    for (int i = 11; i >= 6; --i) {
        printf("%2d ", p->plateau[i]);
    }
    printf("\n");
    printf("   ");
    for (int i = 0; i < 6; ++i) {
        printf("%2d ", p->plateau[i]);
    }
    printf("\n   [Joueur 1 : %s]\n", p->joueur1);
    printf("\nPoints -> %s : %d   |   %s : %d\n",
           p->joueur1, p->cptJoueur1, p->joueur2, p->cptJoueur2);
    printf("--\n");
}

char *plateauToString(const Partie *p) {
    static char buffer[256];
    char temp[32];
    buffer[0] = 0;

    strcat(buffer, "   ");
    for (int i = 11; i >= 6; --i) {
        snprintf(temp, sizeof(temp), "%2d ", p->plateau[i]);
        strcat(buffer, temp);
    }
    strcat(buffer, "\n   ");
    for (int i = 0; i < 6; ++i) {
        snprintf(temp, sizeof(temp), "%2d ", p->plateau[i]);
        strcat(buffer, temp);
    }
    strcat(buffer, "\n");

    return buffer;
}


int main()
{
    srand(time(NULL));
    Partie p;

    initPartie(&p, "TOTO", "RIGOLO");
    p.partieEnCours = true;

    p.plateau[11] = 1;
    p.plateau[6] = 1;
    //p.plateau[5] = 1;


    printf("=== Bienvenue dans le jeu d'Awalé ===\n");
    printf("Le sens de rotation est : %s\n",
           p.sensRotation == -1 ? "horaire" : "anti-horaire");
    printf("%s commence !\n", p.indiceJoueurActuel == 1 ? p.joueur1 : p.joueur2);

    while (p.partieEnCours) {
        afficherPlateau(&p);

        int joueur = p.indiceJoueurActuel;
        const char *nom = (joueur == 1) ? p.joueur1 : p.joueur2;
        int debut = (joueur == 1) ? 0 : 6;
        int fin = (joueur == 1) ? 5 : 11;

        int caseChoisie = -1;
        bool coupValide = false;

        while (!coupValide) {
            printf("\n%s (%s), choisissez une case à jouer [%d-%d] : ",
                   nom, (joueur == 1) ? "Joueur 1" : "Joueur 2", debut, fin);
            if (scanf("%d", &caseChoisie) != 1) {
                while (getchar() != '\n'); // nettoyage buffer
                continue;
            }

            if (caseChoisie < debut || caseChoisie > fin) {
                printf("Case invalide, hors de votre camp.\n");
                continue;
            }

            if (p.plateau[caseChoisie] == 0) {
                printf("Case vide, choisissez une autre.\n");
                continue;
            }

            coupValide = jouerCoup(&p, joueur, caseChoisie);
            if (!coupValide) {
                printf("Coup non valide selon les règles, réessayez.\n");
            }
        }
    }

    // Fin de partie
    printf("\nPartie terminée !\n");
    afficherPlateau(&p);
    printf("Score final :\n");
    printf("%s : %d graines\n", p.joueur1, p.cptJoueur1);
    printf("%s : %d graines\n", p.joueur2, p.cptJoueur2);

    if (p.cptJoueur1 > p.cptJoueur2)
        printf("%s remporte la partie !\n", p.joueur1);
    else if (p.cptJoueur2 > p.cptJoueur1)
        printf("%s remporte la partie !\n", p.joueur2);
    
    destroyPartie(&p);
    return 0;
}