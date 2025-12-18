#ifndef INSCRIPTIONS_H
#define INSCRIPTIONS_H

#define MAX_INSCRIPTIONS 1000
#define ID_LEN 100
#define SPECIALITE_LEN 100
#define ROLE_LEN 50

typedef struct {
    char trainer_id[ID_LEN];
    char centre_id[ID_LEN];
    char specialite[SPECIALITE_LEN];
    char role[ROLE_LEN];  // "Principal" or "Assistant"
    int disponible;       // 1 = available, 0 = not available
} Inscription;

// Global variables
extern Inscription inscriptions[];
extern int nb_inscriptions;

// File operations
int charger_inscriptions(const char *filename);
int sauvegarder_inscriptions(const char *filename);

// CRUD operations
int ajouter_inscription(Inscription inscriptions[], int *nb, Inscription new_inscri);
int supprimer_inscription(Inscription inscriptions[], int *nb, const char *trainer_id, const char *centre_id);
int modifier_role_inscription(Inscription inscriptions[], int nb, const char *trainer_id, const char *centre_id, const char *new_role);
int supprimer_toutes_inscriptions_trainer(Inscription inscriptions[], int *nb, const char *trainer_id);
int annuler_inscription(const char *centre_id, const char *trainer_id);

// Query operations
int trouver_inscription(Inscription inscriptions[], int nb, const char *trainer_id, const char *centre_id);
int get_inscriptions_par_trainer(Inscription inscriptions[], int nb, const char *trainer_id, Inscription result[], int *result_count);

// Specialites management
int charger_specialites(const char *filename, char specialites[][SPECIALITE_LEN], int *nb);

#endif // INSCRIPTIONS_H
