#include "awale.h"

#include "historique.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void init_partie(Client * joueur1, Client * joueur2, Partie * p) 
{

    if (rand() % 2 == 1) {
        p->joueur1 = joueur1;
        p->joueur2 = joueur2;
    } else {
        p->joueur2 = joueur1;
        p->joueur1 = joueur2;
    }
    //Changer pour un enum avec : demandee, en cours, finie
    p->partieEnCours = true;

    p->sensRotation = (rand() % 2) * 2 - 1; // renvoie -1 ou +1 : (0 ou 1) * 2 donc (0 ou 2) - 1

    p->indiceJoueurActuel = rand() % 2 + 1;

    p->cptJoueur1 = 0;
    p->cptJoueur2 = 0;

    for (int i = 0; i < NB_CASE_PLATEAU ; ++i) {
        p->plateau[i] = 4;
    }

    // Initialiser l'historique
    p->historiqueCoups = NULL;
    p->nbCoupsJoues = 0;
    p->capaciteHistorique = 0;

    // Init spec
    p->nbSpectateurs = 0;
}

void destroyPartie(Partie * p)
{
    free(p->plateau);
    
    // Libérer l'historique
    if (p->historiqueCoups) {
        free(p->historiqueCoups);
        p->historiqueCoups = NULL;
    }
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

    cp->joueur1 = toCopy->joueur1;
    cp->joueur2 = toCopy->joueur2;
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

     //  Vérifie l’affamement avant capture 
    if (captureAffameAdversaire(p, numJoueur, dernierIndice)) {
        printf("Coup affamant détecté : capture annulée.\n");
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

    // Sauvegarder score avant pour calculer les captures
    int scoreAvant = (numJoueur == 1) ? p->cptJoueur1 : p->cptJoueur2;

    logiqueJouer(p, numJoueur, indiceCaseJouee);

    // Enregistrer le coup dans l'historique
    int scoreApres = (numJoueur == 1) ? p->cptJoueur1 : p->cptJoueur2;
    int grainesCapturees = scoreApres - scoreAvant;
    enregistrer_coup(p, numJoueur, indiceCaseJouee, grainesCapturees);

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

    // Si partie terminée, sauvegarder dans l'historique
    if (!p->partieEnCours) {
        PartieTerminee *hist = malloc(sizeof(PartieTerminee));
        if (hist) {
            finaliser_partie_historique(p, hist);
            sauvegarder_partie_terminee(p->joueur1, p->joueur2, hist);
            free(hist->coups);  // Libérer le tableau temporaire
            free(hist);
        }
    }

    return true;
}

void afficherPlateau(const Partie *p)
{
    printf("\nPlateau actuel :\n");
    printf("   [Joueur 2 : %s]\n   ", p->joueur2->name);
    for (int i = 11; i >= 6; --i) {
        printf("%2d ", p->plateau[i]);
    }
    printf("\n");
    printf("   ");
    for (int i = 0; i < 6; ++i) {
        printf("%2d ", p->plateau[i]);
    }
    printf("\n   [Joueur 1 : %s]\n", p->joueur1->name);
    printf("\nPoints -> %s : %d   |   %s : %d\n",
           p->joueur1->name, p->cptJoueur1, p->joueur2->name, p->cptJoueur2);
    printf("--\n");
}

/**
 * Créé un string contenant toutes les infos du plateau
 */
void plateauToString(const Partie *p, char * buffer, int taillePlateau) {
    if (!p || !p->plateau) return;

    int offset = 0;

    // Ligne d’en-tête : indices du joueur 2 (cases 11 → 6)
    offset += snprintf(buffer + offset, taillePlateau - offset,
        "\n==================== PLATEAU D'AWALE ====================\n"
        "              Camp du joueur 2 : %s\n"
        "----------------------------------------------------------\n"
        "   Indices : [11] [10] [ 9] [ 8] [ 7] [ 6]\n"
        "   Graines : ", 
        p->joueur2 ? p->joueur2->name : "Inconnu");

    for (int i = 11; i >= 6; --i) {
        offset += snprintf(buffer + offset, taillePlateau - offset, "[%2d] ", p->plateau[i]);
    }

    // Ligne séparatrice
    offset += snprintf(buffer + offset, taillePlateau - offset,
        "\n----------------------------------------------------------\n");

    // Camp du joueur 1
    offset += snprintf(buffer + offset, taillePlateau - offset,
        "   Graines : ");
    for (int i = 0; i < 6; ++i) {
        offset += snprintf(buffer + offset, taillePlateau - offset, "[%2d] ", p->plateau[i]);
    }

    offset += snprintf(buffer + offset, taillePlateau - offset,
        "\n   Indices : [ 0] [ 1] [ 2] [ 3] [ 4] [ 5]\n"
        "----------------------------------------------------------\n"
        "              Camp du joueur 1 : %s\n\n",
        p->joueur1 ? p->joueur1->name : "Inconnu");

    // Scores et joueur actuel
    offset += snprintf(buffer + offset, taillePlateau - offset,
        "Scores : %s = %d graines | %s = %d graines\n",
        p->joueur1 ? p->joueur1->name : "J1", p->cptJoueur1,
        p->joueur2 ? p->joueur2->name : "J2", p->cptJoueur2);

    const char *joueurActuel =
        (p->indiceJoueurActuel == 1 && p->joueur1) ? p->joueur1->name :
        (p->indiceJoueurActuel == 2 && p->joueur2) ? p->joueur2->name :
        "Inconnu";

    if (!p->partieEnCours) {
        offset += snprintf(buffer + offset, taillePlateau - offset,
            "La partie est terminée !\n"
            "==========================================================\n");
    } else {
        offset += snprintf(buffer + offset, taillePlateau - offset,
            "→ C’est au tour de : %s\n"
            "==========================================================\n",
            joueurActuel);    
    }
}