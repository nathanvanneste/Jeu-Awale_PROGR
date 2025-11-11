#ifndef HISTORIQUE_H
#define HISTORIQUE_H

#include "type.h"

#define CAPACITE_INITIALE 50

void enregistrer_coup(Partie *p, int numJoueur, int caseJouee, int grainesCapturees);

// ========== FONCTION 2 : Finaliser la partie ==========
void finaliser_partie_historique(Partie *p, PartieTerminee *hist);

// ========== FONCTION 3 : Sauvegarder dans l'historique des joueurs ==========
void sauvegarder_partie_terminee(Client *c1, Client *c2, PartieTerminee *hist);

// ========== FONCTION 4 : Afficher l'historique (pour menu) ==========
void afficher_historique_parties(Client *c);
// ========== FONCTION 5 : Afficher détails d'une partie ==========
void afficher_detail_partie_historique(Client *c, int indice);

// ========== FONCTION 6 : Replay (simplifié) ==========
void rejouer_partie_en_accelere(Client *client, PartieTerminee *hist);

#endif