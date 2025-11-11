#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include "amis.h"
#include "io.h"
#include "awale.h"

// ========== FONCTION 1 : Envoyer une demande d'ami ==========
void envoyer_demande_ami(Client *demandeur, Client *destinataire) {
    if (!demandeur || !destinataire) return;
    
    // Vérifier si déjà ami
    if (est_ami(demandeur, destinataire->name)) {
        write_client(demandeur->sock, "Ce joueur est déjà votre ami.\n");
        return;
    }
    
    // Vérifier si demande déjà envoyée
    for (int i = 0; i < destinataire->nbDemandesAmisRecues; i++) {
        if (strcmp(destinataire->demandesAmisRecues[i], demandeur->name) == 0) {
            write_client(demandeur->sock, "Demande déjà envoyée à ce joueur.\n");
            return;
        }
    }
    
    // Ajouter la demande
    if (destinataire->nbDemandesAmisRecues < 50) {
        strncpy(destinataire->demandesAmisRecues[destinataire->nbDemandesAmisRecues], 
                demandeur->name, BUF_SIZE - 1);
        destinataire->demandesAmisRecues[destinataire->nbDemandesAmisRecues][BUF_SIZE - 1] = '\0';
        destinataire->nbDemandesAmisRecues++;
        
        char buffer[BUF_SIZE];
        snprintf(buffer, sizeof(buffer), 
                 "Demande d'ami envoyée à %s !\n", destinataire->name);
        write_client(demandeur->sock, buffer);
        
        // Notifier le destinataire
        snprintf(buffer, sizeof(buffer), 
                 "\n[NOUVEAU] %s vous a envoyé une demande d'ami ! "
                 "(Consultez le menu pour répondre)\n", demandeur->name);
        write_client(destinataire->sock, buffer);
    } else {
        write_client(demandeur->sock, "Le joueur a trop de demandes en attente.\n");
    }
}

// ========== FONCTION 2 : Accepter une demande d'ami ==========
void accepter_demande_ami(Client *c, const char *nomDemandeur, Client clients[], int nbClients) {
    if (!c || !nomDemandeur) return;
    
    // Trouver la demande
    int indexDemande = -1;
    for (int i = 0; i < c->nbDemandesAmisRecues; i++) {
        if (strcmp(c->demandesAmisRecues[i], nomDemandeur) == 0) {
            indexDemande = i;
            break;
        }
    }
    
    if (indexDemande == -1) {
        write_client(c->sock, "Demande introuvable.\n");
        return;
    }
    
    // Ajouter dans les deux listes d'amis
    if (c->nbAmis < 100) {
        strncpy(c->amis[c->nbAmis], nomDemandeur, BUF_SIZE - 1);
        c->amis[c->nbAmis][BUF_SIZE - 1] = '\0';
        c->nbAmis++;
    }
    
    // Trouver le demandeur et ajouter la relation réciproque
    Client *demandeur = NULL;
    for (int i = 0; i < nbClients; i++) {
        if (strcmp(clients[i].name, nomDemandeur) == 0) {
            demandeur = &clients[i];
            break;
        }
    }
    
    if (demandeur && demandeur->nbAmis < 100) {
        strncpy(demandeur->amis[demandeur->nbAmis], c->name, BUF_SIZE - 1);
        demandeur->amis[demandeur->nbAmis][BUF_SIZE - 1] = '\0';
        demandeur->nbAmis++;
        
        // Notifier le demandeur
        char buffer[BUF_SIZE];
        snprintf(buffer, sizeof(buffer), 
                 "\n[AMI] %s a accepté votre demande d'ami !\n", c->name);
        write_client(demandeur->sock, buffer);
    }
    
    // Supprimer la demande
    for (int i = indexDemande; i < c->nbDemandesAmisRecues - 1; i++) {
        strcpy(c->demandesAmisRecues[i], c->demandesAmisRecues[i + 1]);
    }
    c->nbDemandesAmisRecues--;
    
    char buffer[BUF_SIZE];
    snprintf(buffer, sizeof(buffer), 
             "Vous êtes maintenant ami avec %s !\n", nomDemandeur);
    write_client(c->sock, buffer);
}

// ========== FONCTION 3 : Refuser une demande d'ami ==========
void refuser_demande_ami(Client *c, const char *nomDemandeur) {
    if (!c || !nomDemandeur) return;
    
    // Trouver et supprimer la demande
    int indexDemande = -1;
    for (int i = 0; i < c->nbDemandesAmisRecues; i++) {
        if (strcmp(c->demandesAmisRecues[i], nomDemandeur) == 0) {
            indexDemande = i;
            break;
        }
    }
    
    if (indexDemande == -1) {
        write_client(c->sock, "Demande introuvable.\n");
        return;
    }
    
    // Supprimer la demande
    for (int i = indexDemande; i < c->nbDemandesAmisRecues - 1; i++) {
        strcpy(c->demandesAmisRecues[i], c->demandesAmisRecues[i + 1]);
    }
    c->nbDemandesAmisRecues--;
    
    char buffer[BUF_SIZE];
    snprintf(buffer, sizeof(buffer), 
             "Demande d'ami de %s refusée.\n", nomDemandeur);
    write_client(c->sock, buffer);
}

// ========== FONCTION 4 : Vérifier si deux joueurs sont amis ==========
bool est_ami(Client *c, const char *nomAutre) {
    if (!c || !nomAutre) return false;
    
    for (int i = 0; i < c->nbAmis; i++) {
        if (strcmp(c->amis[i], nomAutre) == 0) {
            return true;
        }
    }
    return false;
}

// ========== FONCTION 5 : Afficher la liste des amis en ligne ==========
void afficher_liste_amis(Client *c, Client clients[], int nbClients) {
    if (!c) return;
    
    char buffer[BUF_SIZE * 2];
    
    if (c->nbAmis == 0) {
        write_client(c->sock, "\nVous n'avez pas encore d'amis.\n");
        write_client(c->sock, "Consultez la liste des joueurs pour envoyer des demandes !\n\n");
        write_client(c->sock, "Tapez '/menu' pour retour\n");
        return;
    }
    
    snprintf(buffer, sizeof(buffer), 
             "\n=== VOS AMIS (%d) ===\n", c->nbAmis);
    write_client(c->sock, buffer);
    
    int amisEnLigne = 0;
    int amisHorsLigne = 0;
    int numeroAffichage = 1;
    
    // Parcourir la liste des amis
    for (int i = 0; i < c->nbAmis; i++) {
        bool enLigne = false;
        
        // Vérifier si l'ami est connecté
        for (int j = 0; j < nbClients; j++) {
            if (strcmp(clients[j].name, c->amis[i]) == 0 && clients[j].connecte) {
                enLigne = true;
                
                // Afficher l'état de l'ami
                const char *statut = "Disponible";
                bool enPartie = false;
                
                if (clients[j].etat_courant == ETAT_PARTIE_EN_COURS) {
                    statut = "En partie";
                    enPartie = true;
                } else if (clients[j].etat_courant == ETAT_CHOOSE_PLAYER ||
                           clients[j].etat_courant == ETAT_LOOK_PLAYER) {
                    statut = "Cherche un adversaire";
                }
                
                if (enPartie) {
                    // Afficher avec numéro pour permettre de spectater
                    snprintf(buffer, sizeof(buffer), 
                             "  %d. %s - [EN LIGNE] %s 🎮 [Spectater disponible]\n", 
                             numeroAffichage++, c->amis[i], statut);
                } else {
                    snprintf(buffer, sizeof(buffer), 
                             "  ✓ %s - [EN LIGNE] %s\n", c->amis[i], statut);
                }
                write_client(c->sock, buffer);
                amisEnLigne++;
                break;
            }
        }
        
        if (!enLigne) {
            snprintf(buffer, sizeof(buffer), 
                     "  • %s - [Hors ligne]\n", c->amis[i]);
            write_client(c->sock, buffer);
            amisHorsLigne++;
        }
    }
    
    snprintf(buffer, sizeof(buffer), 
             "\nEn ligne: %d | Hors ligne: %d\n", amisEnLigne, amisHorsLigne);
    write_client(c->sock, buffer);
    
    if (numeroAffichage > 1) {
        write_client(c->sock, "\nTapez un numéro pour spectater une partie, '/menu' pour retour\n");
    } else {
        write_client(c->sock, "\nAucun ami en partie à spectater. Tapez '/menu' pour retour\n");
    }
}

// ========== FONCTION 6 : Afficher les demandes d'amis reçues ==========
void afficher_demandes_amis(Client *c) {
    if (!c) return;
    
    char buffer[BUF_SIZE];
    
    if (c->nbDemandesAmisRecues == 0) {
        write_client(c->sock, "\nAucune demande d'ami en attente.\n\n");
        write_client(c->sock, "Tapez '/menu' pour retour\n");
        return;
    }
    
    snprintf(buffer, sizeof(buffer), 
             "\n=== DEMANDES D'AMIS REÇUES (%d) ===\n", c->nbDemandesAmisRecues);
    write_client(c->sock, buffer);
    
    for (int i = 0; i < c->nbDemandesAmisRecues; i++) {
        snprintf(buffer, sizeof(buffer), 
                 "  %d. %s\n", i + 1, c->demandesAmisRecues[i]);
        write_client(c->sock, buffer);
    }
    
    write_client(c->sock, "\nTapez le numéro pour répondre, '/menu' pour retour\n");
}

// ========== MODE SPECTATEUR ==========

// Rejoindre une partie en tant que spectateur
bool rejoindre_comme_spectateur(Client *spectateur, Partie *partie) {
    if (!spectateur || !partie) return false;
    
    // Vérifier si la partie est en cours
    if (!partie->partieEnCours) {
        write_client(spectateur->sock, "Cette partie est terminée.\n");
        return false;
    }
    
    // Vérifier si pas déjà spectateur de cette partie
    for (int i = 0; i < partie->nbSpectateurs; i++) {
        if (partie->spectateurs[i] == spectateur) {
            write_client(spectateur->sock, "Vous êtes déjà spectateur de cette partie.\n");
            return false;
        }
    }
    
    // Vérifier la capacité
    if (partie->nbSpectateurs >= 20) {
        write_client(spectateur->sock, "La partie a atteint le nombre maximum de spectateurs.\n");
        return false;
    }
    
    // Ajouter le spectateur
    partie->spectateurs[partie->nbSpectateurs++] = spectateur;
    spectateur->partieSpectatee = partie;
    spectateur->etat_courant = ETAT_SPECTATEUR;
    
    char buffer[BUF_SIZE];
    snprintf(buffer, sizeof(buffer), 
             "\n=== MODE SPECTATEUR ===\n"
             "Vous regardez : %s vs %s\n"
             "Score : %d - %d\n\n",
             partie->joueur1->name, partie->joueur2->name,
             partie->cptJoueur1, partie->cptJoueur2);
    write_client(spectateur->sock, buffer);
    
    // Afficher le plateau actuel
    char plateauStr[2048];
    plateauToString(partie, plateauStr, sizeof(plateauStr));
    write_client(spectateur->sock, plateauStr);
    
    write_client(spectateur->sock, "\nTapez '/menu' pour quitter le mode spectateur\n\n");
    
    // Notifier les joueurs
    snprintf(buffer, sizeof(buffer), 
             "[SPECTATEUR] %s regarde maintenant votre partie\n", 
             spectateur->name);
    write_client(partie->joueur1->sock, buffer);
    write_client(partie->joueur2->sock, buffer);
    
    return true;
}

// Quitter le mode spectateur
void quitter_mode_spectateur(Client *spectateur) {
    if (!spectateur || !spectateur->partieSpectatee) return;
    
    Partie *partie = spectateur->partieSpectatee;
    
    // Retirer le spectateur de la liste
    for (int i = 0; i < partie->nbSpectateurs; i++) {
        if (partie->spectateurs[i] == spectateur) {
            // Décaler les éléments suivants
            for (int j = i; j < partie->nbSpectateurs - 1; j++) {
                partie->spectateurs[j] = partie->spectateurs[j + 1];
            }
            partie->nbSpectateurs--;
            break;
        }
    }
    
    spectateur->partieSpectatee = NULL;
    
    // Notifier les joueurs
    char buffer[BUF_SIZE];
    snprintf(buffer, sizeof(buffer), 
             "[SPECTATEUR] %s ne regarde plus votre partie\n", 
             spectateur->name);
    write_client(partie->joueur1->sock, buffer);
    write_client(partie->joueur2->sock, buffer);
    
    write_client(spectateur->sock, "Vous avez quitté le mode spectateur.\n");
}

// Envoyer un message à tous les spectateurs d'une partie
void notifier_spectateurs(Partie *p, const char *message) {
    if (!p || !message) return;
    
    for (int i = 0; i < p->nbSpectateurs; i++) {
        if (p->spectateurs[i] && p->spectateurs[i]->connecte) {
            write_client(p->spectateurs[i]->sock, message);
        }
    }
}
