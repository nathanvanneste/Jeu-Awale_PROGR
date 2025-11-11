#ifndef AMIS_H
#define AMIS_H

#include "type.h"

// ========== FONCTION 1 : Envoyer une demande d'ami ==========
void envoyer_demande_ami(Client *demandeur, Client *destinataire);

// ========== FONCTION 2 : Accepter une demande d'ami ==========
void accepter_demande_ami(Client *c, const char *nomDemandeur, Client clients[], int nbClients);

// ========== FONCTION 3 : Refuser une demande d'ami ==========
void refuser_demande_ami(Client *c, const char *nomDemandeur);

// ========== FONCTION 4 : Vérifier si deux joueurs sont amis ==========
bool est_ami(Client *c, const char *nomAutre);

// ========== FONCTION 5 : Afficher la liste des amis en ligne ==========
void afficher_liste_amis(Client *c, Client clients[], int nbClients);

// ========== FONCTION 6 : Afficher les demandes d'amis reçues ==========
void afficher_demandes_amis(Client *c);

// ========== MODE SPECTATEUR ==========

// Rejoindre une partie en tant que spectateur
bool rejoindre_comme_spectateur(Client *spectateur, Partie *partie);
// Quitter le mode spectateur
void quitter_mode_spectateur(Client *spectateur);

// Envoyer un message à tous les spectateurs d'une partie
void notifier_spectateurs(Partie *p, const char *message);

#endif