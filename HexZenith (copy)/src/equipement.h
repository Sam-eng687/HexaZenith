#ifndef EQUIPEMENT_H_INCLUDED
#define EQUIPEMENT_H_INCLUDED
#include <stdio.h>

// Use the Equipement structure from treeview.h
// No need to redefine it here, just include treeview.h
#include "treeview.h"

// Fonctions CRUD pour la gestion des équipements
int ajouter_equipement(char *filename, Equipement e);
int modifier_equipement(char *filename, char *id, Equipement nouv);
int supprimer_equipement(char *filename, char *id);
Equipement chercher_equipement(char *filename, char *id);
int equipement_id_existe(char *filename, char *id);

#endif // EQUIPEMENT_H_INCLUDED
