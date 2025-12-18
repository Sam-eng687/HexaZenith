#ifndef CENTRES_H
#define CENTRES_H

#define MAX_CENTRES 1000
#define ID_LEN 100
#define NOM_LEN 100
#define TYPE_LEN 100
#define VILLE_LEN 100

typedef struct {
    char id[ID_LEN];
    char nom[NOM_LEN];
    char type[TYPE_LEN];
    int capacite;
    int ouvert;      // 1 = open, 0 = closed
    int parking;     // 1 = has parking, 0 = no parking
    int wifi;        // 1 = has wifi, 0 = no wifi
    int cafeteria;   // 1 = has cafeteria, 0 = no cafeteria
    char ville[VILLE_LEN];
} Centre;

// File operations
int charger_centres(const char *filename, Centre centres[], int *nb_centres);
int sauvegarder_centres(const char *filename, Centre centres[], int nb_centres);

// CRUD operations
int ajouter_centre(Centre centres[], int *nb_centres, Centre nouveau);
void supprimer_centre(Centre centres[], int *nb_centres, int index);
int modifier_centre(Centre centres[], int nb_centres, const char *id, Centre modifications);

// Search and filter operations
void rechercher_par_id(Centre centres[], int nb_centres, const char *id, Centre result[], int *result_count);
void filtrer_par_type(Centre centres[], int nb_centres, const char *type, Centre result[], int *result_count);
void filtrer_par_ville(Centre centres[], int nb_centres, const char *ville, Centre result[], int *result_count);

// Statistics
int nombre_centres_ouverts(Centre centres[], int nb_centres);
int capacite_totale(Centre centres[], int nb_centres);
void centres_par_ville(Centre centres[], int nb_centres, char villes[][VILLE_LEN], int counts[], int *nb_villes);

// Global declarations (used across files)
extern Centre centres[];
extern int nb_centres;

#endif // CENTRES_H
