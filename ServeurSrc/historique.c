#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "historique.h"
#include "io.h"


// ========== FONCTION 1 : Enregistrer un coup ==========
void enregistrer_coup(Partie *p, int numJoueur, int caseJouee, int grainesCapturees) {
    if (!p) return;
    
    // Initialiser si première fois
    if (!p->historiqueCoups) {
        p->historiqueCoups = calloc(CAPACITE_INITIALE, sizeof(CoupHistorique));
        p->capaciteHistorique = CAPACITE_INITIALE;
        p->nbCoupsJoues = 0;
    }
    
    // Agrandir si nécessaire
    if (p->nbCoupsJoues >= p->capaciteHistorique) {
        int nouvCapacite = p->capaciteHistorique * 2;
        CoupHistorique *nouveau = realloc(p->historiqueCoups, nouvCapacite * sizeof(CoupHistorique));
        if (!nouveau) return;
        p->historiqueCoups = nouveau;
        p->capaciteHistorique = nouvCapacite;
    }
    
    // Enregistrer
    CoupHistorique *coup = &p->historiqueCoups[p->nbCoupsJoues];
    coup->numJoueur = numJoueur;
    coup->caseJouee = caseJouee;
    coup->grainesCapturees = grainesCapturees;
    coup->timestamp = time(NULL);
    
    p->nbCoupsJoues++;
}

// ========== FONCTION 2 : Finaliser la partie ==========
void finaliser_partie_historique(Partie *p, PartieTerminee *hist) {
    if (!p || !hist) return;
    
    strncpy(hist->nomJoueur1, p->joueur1->name, BUF_SIZE - 1);
    strncpy(hist->nomJoueur2, p->joueur2->name, BUF_SIZE - 1);
    hist->scoreJoueur1 = p->cptJoueur1;
    hist->scoreJoueur2 = p->cptJoueur2;
    
    if (p->cptJoueur1 > p->cptJoueur2) {
        strncpy(hist->vainqueur, p->joueur1->name, BUF_SIZE - 1);
    } else if (p->cptJoueur2 > p->cptJoueur1) {
        strncpy(hist->vainqueur, p->joueur2->name, BUF_SIZE - 1);
    } else {
        strncpy(hist->vainqueur, "Égalité", BUF_SIZE - 1);
    }
    
    hist->sensRotation = p->sensRotation;
    hist->dateFin = time(NULL);
    hist->nbCoups = p->nbCoupsJoues;
    
    // Calculer date début
    if (p->nbCoupsJoues > 0) {
        hist->dateDebut = p->historiqueCoups[0].timestamp;
    } else {
        hist->dateDebut = hist->dateFin;
    }
    
    // Copier les coups
    if (p->nbCoupsJoues > 0 && p->historiqueCoups) {
        hist->coups = malloc(p->nbCoupsJoues * sizeof(CoupHistorique));
        if (hist->coups) {
            memcpy(hist->coups, p->historiqueCoups, p->nbCoupsJoues * sizeof(CoupHistorique));
        }
    } else {
        hist->coups = NULL;
    }
}

// ========== FONCTION 3 : Sauvegarder dans l'historique des joueurs ==========
void sauvegarder_partie_terminee(Client *c1, Client *c2, PartieTerminee *hist) {
    if (!c1 || !c2 || !hist) return;
    
    // Pour joueur 1
    if (c1->nbPartiesHistorique < 256) {
        PartieTerminee *hist1 = malloc(sizeof(PartieTerminee));
        if (hist1) {
            memcpy(hist1, hist, sizeof(PartieTerminee));
            if (hist->nbCoups > 0 && hist->coups) {
                hist1->coups = malloc(hist->nbCoups * sizeof(CoupHistorique));
                if (hist1->coups) {
                    memcpy(hist1->coups, hist->coups, hist->nbCoups * sizeof(CoupHistorique));
                }
            }
            c1->historique[c1->nbPartiesHistorique++] = hist1;
        }
    }
    
    // Pour joueur 2
    if (c2->nbPartiesHistorique < 256) {
        PartieTerminee *hist2 = malloc(sizeof(PartieTerminee));
        if (hist2) {
            memcpy(hist2, hist, sizeof(PartieTerminee));
            if (hist->nbCoups > 0 && hist->coups) {
                hist2->coups = malloc(hist->nbCoups * sizeof(CoupHistorique));
                if (hist2->coups) {
                    memcpy(hist2->coups, hist->coups, hist->nbCoups * sizeof(CoupHistorique));
                }
            }
            c2->historique[c2->nbPartiesHistorique++] = hist2;
        }
    }
    
    printf("Partie sauvegardée dans l'historique de %s et %s\n", c1->name, c2->name);
}

// ========== FONCTION 4 : Afficher l'historique (pour menu) ==========
void afficher_historique_parties(Client *c) {
    if (!c) return;
    
    char buffer[BUF_SIZE * 2];
    
    if (c->nbPartiesHistorique == 0) {
        snprintf(buffer, sizeof(buffer), "Vous n'avez aucune partie dans votre historique.\n");
        write_client(c->sock, buffer);
        return;
    }
    
    snprintf(buffer, sizeof(buffer),
             "\n=== HISTORIQUE (%d parties) ===\n", c->nbPartiesHistorique);
    write_client(c->sock, buffer);
    
    for (int i = 0; i < c->nbPartiesHistorique; i++) {
        PartieTerminee *p = c->historique[i];
        if (!p) continue;
        
        const char *adv = (strcasecmp(p->nomJoueur1, c->name) == 0) ? p->nomJoueur2 : p->nomJoueur1;
        int scoreJ = (strcasecmp(p->nomJoueur1, c->name) == 0) ? p->scoreJoueur1 : p->scoreJoueur2;
        int scoreA = (strcasecmp(p->nomJoueur1, c->name) == 0) ? p->scoreJoueur2 : p->scoreJoueur1;
        
        const char *resultat = (strcasecmp(p->vainqueur, "Égalité") == 0) ? "=" :
                               (strcasecmp(p->vainqueur, c->name) == 0) ? "V" : "D";
        
        snprintf(buffer, sizeof(buffer),
                 "%d. vs %-15s | %2d-%2d | %s | %d coups\n",
                 i + 1, adv, scoreJ, scoreA, resultat, p->nbCoups);
        write_client(c->sock, buffer);
    }
    
    snprintf(buffer, sizeof(buffer),
             "\nTapez un numéro pour détails, 'menu' pour retour\n");
    write_client(c->sock, buffer);
}

// ========== FONCTION 5 : Afficher détails d'une partie ==========
void afficher_detail_partie_historique(Client *c, int indice) {
    if (!c || indice < 0 || indice >= c->nbPartiesHistorique) {
        write_client(c->sock, "Partie invalide.\n");
        return;
    }
    
    PartieTerminee *p = c->historique[indice];
    if (!p) return;
    
    char buffer[BUF_SIZE * 2];
    
    snprintf(buffer, sizeof(buffer),
             "\n=== PARTIE vs %s ===\n"
             "%s vs %s\n"
             "Score : %d - %d\n"
             "Vainqueur : %s\n"
             "Coups joués : %d\n\n",
             (strcasecmp(p->nomJoueur1, c->name) == 0) ? p->nomJoueur2 : p->nomJoueur1,
             p->nomJoueur1, p->nomJoueur2,
             p->scoreJoueur1, p->scoreJoueur2,
             p->vainqueur, p->nbCoups);
    write_client(c->sock, buffer);
    
    if (p->nbCoups > 0) {
        snprintf(buffer, sizeof(buffer), "Coups (5 premiers) :\n");
        write_client(c->sock, buffer);
        
        for (int i = 0; i < (p->nbCoups < 5 ? p->nbCoups : 5); i++) {
            CoupHistorique *coup = &p->coups[i];
            snprintf(buffer, sizeof(buffer),
                     "  %d. J%d case %d -> %d graines\n",
                     i + 1, coup->numJoueur, coup->caseJouee, coup->grainesCapturees);
            write_client(c->sock, buffer);
        }
        
        if (p->nbCoups > 5) {
            snprintf(buffer, sizeof(buffer), "  ... et %d autres\n", p->nbCoups - 5);
            write_client(c->sock, buffer);
        }
    }
    
    snprintf(buffer, sizeof(buffer), "\nTapez '/retour', '/menu' ou bien '/replay' pour avoir le détail des coups\n");
    write_client(c->sock, buffer);
}

// ========== FONCTION 6 : Replay (simplifié) ==========
void rejouer_partie_en_accelere(Client *client, PartieTerminee *hist) {
    if (!client || !hist || !hist->coups || hist->nbCoups == 0) return;
    
    char buffer[BUF_SIZE];
    
    snprintf(buffer, sizeof(buffer), "\n=== REPLAY ===\n");
    write_client(client->sock, buffer);
    
    snprintf(buffer, sizeof(buffer), "%s vs %s\n", hist->nomJoueur1, hist->nomJoueur2);
    write_client(client->sock, buffer);
    
    for (int i = 0; i < hist->nbCoups; i++) {
        CoupHistorique *c = &hist->coups[i];
        snprintf(buffer, sizeof(buffer), 
                 "Coup %d: J%d case %d (%d captures)\n",
                 i+1, c->numJoueur, c->caseJouee, c->grainesCapturees);
        write_client(client->sock, buffer);
    }
    
    snprintf(buffer, sizeof(buffer), 
             "Score final: %d - %d\n\n", 
             hist->scoreJoueur1, hist->scoreJoueur2);
    write_client(client->sock, buffer);
}
