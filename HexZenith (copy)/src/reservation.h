#ifndef RESERVATION_H_INCLUDED
#define RESERVATION_H_INCLUDED

// Use structures from treeview.h
#include "treeview.h"

// Fonction pour lire un cours depuis le fichier
Cours lire_cours_reservation(char *filename, char *id_cours);

// Fonction pour obtenir le nom du niveau
const char* get_niveau_nom_reservation(int niveau);

// Fonctions pour la gestion des réservations d'équipements
int ajouter_reservation_equipement(char *id_entraineur, char *id_cours, char *equip_id, char *equip_nom, char *equip_type, int quantite);
int supprimer_reservation_equipement(char *id_entraineur, char *id_cours, char *equip_id);
int supprimer_toutes_reservations_cours(char *id_entraineur, char *id_cours);

#endif // RESERVATION_H_INCLUDED
