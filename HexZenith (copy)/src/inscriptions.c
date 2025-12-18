#include <stdio.h>
#include <string.h>
#include "inscriptions.h"

Inscription inscriptions[MAX_INSCRIPTIONS];
int nb_inscriptions = 0;

/* Load inscriptions from file */
int charger_inscriptions(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        nb_inscriptions = 0;
        return -1;
    }
    
    nb_inscriptions = 0;
    while (nb_inscriptions < MAX_INSCRIPTIONS &&
           fscanf(f, "%s %s %s %s %d\n", 
                  inscriptions[nb_inscriptions].trainer_id,
                  inscriptions[nb_inscriptions].centre_id,
                  inscriptions[nb_inscriptions].specialite,
                  inscriptions[nb_inscriptions].role,
                  &inscriptions[nb_inscriptions].disponible) == 5) {
        nb_inscriptions++;
    }
    
    fclose(f);
    return 0;
}

/* Save inscriptions to file */
int sauvegarder_inscriptions(const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        return -1;
    }
    
    for (int i = 0; i < nb_inscriptions; i++) {
        fprintf(f, "%s %s %s %s %d\n",
                inscriptions[i].trainer_id,
                inscriptions[i].centre_id,
                inscriptions[i].specialite,
                inscriptions[i].role,
                inscriptions[i].disponible);
    }
    
    fclose(f);
    return 0;
}

/* Add a new inscription */
int ajouter_inscription(Inscription inscriptions[], int *nb, Inscription new_inscri) {
    if (*nb >= MAX_INSCRIPTIONS) {
        return -1;  // Array full
    }
    
    // Check if inscription already exists
    if (trouver_inscription(inscriptions, *nb, new_inscri.trainer_id, new_inscri.centre_id) != -1) {
        return -2;  // Already exists
    }
    
    inscriptions[*nb] = new_inscri;
    (*nb)++;
    return 0;
}

/* Delete an inscription */
int supprimer_inscription(Inscription inscriptions[], int *nb, const char *trainer_id, const char *centre_id) {
    int index = trouver_inscription(inscriptions, *nb, trainer_id, centre_id);
    
    if (index == -1) {
        return -1;  // Not found
    }
    
    // Shift elements
    for (int i = index; i < *nb - 1; i++) {
        inscriptions[i] = inscriptions[i + 1];
    }
    
    (*nb)--;
    return 0;
}

/* Modify role of an inscription */
int modifier_role_inscription(Inscription inscriptions[], int nb, const char *trainer_id, const char *centre_id, const char *new_role) {
    int index = trouver_inscription(inscriptions, nb, trainer_id, centre_id);
    
    if (index == -1) {
        return -1;  // Not found
    }
    
    strncpy(inscriptions[index].role, new_role, ROLE_LEN - 1);
    inscriptions[index].role[ROLE_LEN - 1] = '\0';
    return 0;
}

/* Delete all inscriptions for a trainer */
int supprimer_toutes_inscriptions_trainer(Inscription inscriptions[], int *nb, const char *trainer_id) {
    int count = 0;
    int i = 0;
    
    while (i < *nb) {
        if (strcmp(inscriptions[i].trainer_id, trainer_id) == 0) {
            // Shift elements
            for (int j = i; j < *nb - 1; j++) {
                inscriptions[j] = inscriptions[j + 1];
            }
            (*nb)--;
            count++;
        } else {
            i++;
        }
    }
    
    return count;
}

/* Cancel an inscription (wrapper function) */
int annuler_inscription(const char *centre_id, const char *trainer_id) {
    int index = trouver_inscription(inscriptions, nb_inscriptions, trainer_id, centre_id);
    
    if (index == -1) {
        return 0;  // Not found
    }
    
    // Shift elements to remove the inscription
    for (int i = index; i < nb_inscriptions - 1; i++) {
        inscriptions[i] = inscriptions[i + 1];
    }
    
    nb_inscriptions--;
    
    // Save to file
    sauvegarder_inscriptions("inscriptions.txt");
    
    return 1;  // Success
}

/* Find inscription index */
int trouver_inscription(Inscription inscriptions[], int nb, const char *trainer_id, const char *centre_id) {
    for (int i = 0; i < nb; i++) {
        if (strcmp(inscriptions[i].trainer_id, trainer_id) == 0 &&
            strcmp(inscriptions[i].centre_id, centre_id) == 0) {
            return i;
        }
    }
    return -1;
}

/* Get all inscriptions for a trainer */
int get_inscriptions_par_trainer(Inscription inscriptions[], int nb, const char *trainer_id, Inscription result[], int *result_count) {
    *result_count = 0;
    
    for (int i = 0; i < nb; i++) {
        if (strcmp(inscriptions[i].trainer_id, trainer_id) == 0) {
            result[*result_count] = inscriptions[i];
            (*result_count)++;
        }
    }
    
    return *result_count > 0 ? 0 : -1;
}

/* Load specialites from file */
int charger_specialites(const char *filename, char specialites[][SPECIALITE_LEN], int *nb) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        *nb = 0;
        return -1;
    }
    
    *nb = 0;
    char line[SPECIALITE_LEN];
    
    while (*nb < 100 && fgets(line, sizeof(line), f)) {
        // Remove newline
        line[strcspn(line, "\n")] = '\0';
        
        if (strlen(line) > 0) {
            strncpy(specialites[*nb], line, SPECIALITE_LEN - 1);
            specialites[*nb][SPECIALITE_LEN - 1] = '\0';
            (*nb)++;
        }
    }
    
    fclose(f);
    return 0;
}
