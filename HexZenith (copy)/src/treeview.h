#ifndef TREEVIEW_H
#define TREEVIEW_H

#include <gtk/gtk.h>
#include "centres.h"

#define MAX_STR 256
#define MAX_COURS 1000

// Course structure
typedef struct {
    char id_cours[MAX_STR];
    char nom_cours[MAX_STR];
    char type_activite[MAX_STR];
    int jour;
    int mois;
    int annee;
    int heure_debut;
    int heure_fin;
    int niveau;
    int places_max;
    char centre[MAX_STR];
    unsigned int jours_mask;
} Cours;

// Equipment structure
typedef struct {
    char id_equipement[MAX_STR];
    char nom_equipement[MAX_STR];
    char type[MAX_STR];
    int nombre;
} Equipement;

// Event structure
typedef struct {
    char id_event[7];
    char nom_event[300];
    char type_event[100];
    char centre[200];
    int jour;
    int mois;
    int annee;
    int type_acces;
} evenement;

// Trainer structure
typedef struct {
    int CIN_trainer;
    char Nom_trainer[100];
    char Prenom_trainer[100];
    int sexe_trainer;
    char specialite[100];
    int jour;
    int mois;
    int annee;
    int train_priv;
} Entraineur;

// Member structure
typedef struct {
    char cin[20];
    char nom[100];
    char prenom[100];
    char sexe[10];
    char telephone[20];
    char date_inscription[20];
} Membre;

// Private trainer structure
typedef struct {
    int cin;
    char nom[100];
    char prenom[100];
    char specialite[100];
    char jours_disponibles[50];
} EntraineurPrive;

// Global file paths
extern char USER_FILE_PATH[256];
extern char COURS_FILE_PATH[256];
extern char EQUIPEMENT_FILE_PATH[256];
extern char CENTRE_FILE_PATH[256];
extern char ENTRAINEUR_FILE_PATH[256];
extern char MEMBRE_FILE_PATH[256];
extern char EVENT_FILE_PATH[256];
extern char INSCRIPTION_ENTRAINEUR_FILE_PATH[256];
extern char INSCRIPTION_MEMBRE_FILE_PATH[256];
extern char ENTRAINEUR_PRIVE_FILE_PATH[256];

// Course treeview functions
void refresh_trainer_courses_treeview(GtkWidget *liste, const char *id_entraineur_param);
void refresh_admin_courses_treeview(GtkWidget *treeview);
void refresh_member_courses_treeview(GtkWidget *treeview);

// Equipment treeview functions
void refresh_equipment_treeview(GtkWidget *liste);
void refresh_equipment_types_combobox(GtkWidget *combobox, int ajouter_aucune);

// Activity types combobox
void refresh_activity_types_combobox(GtkWidget *combobox);

// Centre treeview functions
void refresh_admin_centres_treeview(GtkWidget *treeview);
void refresh_trainer_centres_treeview(GtkWidget *treeview);
void refresh_centre_types_combobox(GtkWidget *combobox, const char *filename);
void refresh_trainer_centre_types_combobox(GtkWidget *combobox, const char *filename);

// Generic centre treeview refresh (used in callbacks)
void refresh_treeview(GtkTreeView *treeview, Centre centres[], int nb_centres);
void populate_combobox_types(GtkWidget *combobox, Centre centres[], int nb_centres);

// Event treeview functions
void refresh_admin_events_treeview(GtkWidget *treeview);

// Trainer treeview functions
void refresh_admin_trainers_treeview(GtkWidget *treeview);

// Member treeview functions
void refresh_admin_membres_treeview(GtkWidget *treeview);
void refresh_membre_entraineurs_prives_treeview(GtkWidget *treeview);

#endif // TREEVIEW_H
