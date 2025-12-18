#include "centres.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int charger_centres(const char *filename, Centre centres[], int *nb_centres) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("DEBUG: Could not open centres file: %s\n", filename);
        /* Create empty file if it doesn't exist */
        file = fopen(filename, "w");
        if (file) fclose(file);
        *nb_centres = 0;
        return 0;
    }
    
    *nb_centres = 0;
    char line[256];
    
    while (fgets(line, sizeof(line), file) && *nb_centres < MAX_CENTRES) {
        if (line[0] == '#' || line[0] == '\n') continue;
        
        Centre *c = &centres[*nb_centres];
        int ouvert_int, parking_int, wifi_int, cafeteria_int;
        
        int result = sscanf(line, "%99[^,],%99[^,],%99[^,],%d,%d,%d,%d,%d,%99[^\n]",
                           c->id, c->nom, c->type, &c->capacite,
                           &ouvert_int, &parking_int, &wifi_int, &cafeteria_int, c->ville);
        
        if (result == 9) {
            c->ouvert = ouvert_int;
            c->parking = parking_int;
            c->wifi = wifi_int;
            c->cafeteria = cafeteria_int;
            (*nb_centres)++;
        }
    }
    
    fclose(file);
    printf("DEBUG: Loaded %d centres from %s\n", *nb_centres, filename);
    return 0;
}

int sauvegarder_centres(const char *filename, Centre centres[], int nb_centres) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Error opening centres file for writing");
        return -1;
    }
    
    fprintf(file, "# ID,Nom,Type,Capacité,Ouvert,Parking,Wifi,Cafeteria,Ville\n");
    
    for (int i = 0; i < nb_centres; i++) {
        fprintf(file, "%s,%s,%s,%d,%d,%d,%d,%d,%s\n",
                centres[i].id,
                centres[i].nom,
                centres[i].type,
                centres[i].capacite,
                centres[i].ouvert,
                centres[i].parking,
                centres[i].wifi,
                centres[i].cafeteria,
                centres[i].ville);
    }
    
    fclose(file);
    printf("DEBUG: Saved %d centres to %s\n", nb_centres, filename);
    return 0;
}

int ajouter_centre(Centre centres[], int *nb_centres, Centre nouveau) {
    if (*nb_centres >= MAX_CENTRES) {
        return -1;
    }
    
    /* Check if ID already exists */
    for (int i = 0; i < *nb_centres; i++) {
        if (strcmp(centres[i].id, nouveau.id) == 0) {
            return -2;
        }
    }
    
    centres[*nb_centres] = nouveau;
    (*nb_centres)++;
    return 0;
}

void supprimer_centre(Centre centres[], int *nb_centres, int index) {
    if (index < 0 || index >= *nb_centres) {
        return;
    }
    
    /* Shift all elements after index one position left */
    for (int i = index; i < *nb_centres - 1; i++) {
        centres[i] = centres[i + 1];
    }
    
    (*nb_centres)--;
}

int modifier_centre(Centre centres[], int nb_centres, const char *id, Centre modifications) {
    for (int i = 0; i < nb_centres; i++) {
        if (strcmp(centres[i].id, id) == 0) {
            /* Don't modify ID, nom, ville */
            if (modifications.type[0] != '\0') {
                strncpy(centres[i].type, modifications.type, TYPE_LEN-1);
                centres[i].type[TYPE_LEN-1] = '\0';
            }
            
            if (modifications.capacite > 0) {
                centres[i].capacite = modifications.capacite;
            }
            
            /* These are flags, -1 means don't change */
            if (modifications.ouvert != -1) {
                centres[i].ouvert = modifications.ouvert;
            }
            if (modifications.parking != -1) {
                centres[i].parking = modifications.parking;
            }
            if (modifications.wifi != -1) {
                centres[i].wifi = modifications.wifi;
            }
            if (modifications.cafeteria != -1) {
                centres[i].cafeteria = modifications.cafeteria;
            }
            
            return 0;
        }
    }
    return -1;
}

void rechercher_par_id(Centre centres[], int nb_centres, const char *id, Centre result[], int *result_count) {
    *result_count = 0;
    
    for (int i = 0; i < nb_centres; i++) {
        if (strstr(centres[i].id, id) != NULL) {
            result[*result_count] = centres[i];
            (*result_count)++;
        }
    }
}

void filtrer_par_type(Centre centres[], int nb_centres, const char *type, Centre result[], int *result_count) {
    *result_count = 0;
    
    for (int i = 0; i < nb_centres; i++) {
        if (strcmp(centres[i].type, type) == 0) {
            result[*result_count] = centres[i];
            (*result_count)++;
        }
    }
}

void filtrer_par_ville(Centre centres[], int nb_centres, const char *ville, Centre result[], int *result_count) {
    *result_count = 0;
    
    for (int i = 0; i < nb_centres; i++) {
        if (strcmp(centres[i].ville, ville) == 0) {
            result[*result_count] = centres[i];
            (*result_count)++;
        }
    }
}

int nombre_centres_ouverts(Centre centres[], int nb_centres) {
    int count = 0;
    for (int i = 0; i < nb_centres; i++) {
        if (centres[i].ouvert) {
            count++;
        }
    }
    return count;
}

int capacite_totale(Centre centres[], int nb_centres) {
    int total = 0;
    for (int i = 0; i < nb_centres; i++) {
        total += centres[i].capacite;
    }
    return total;
}

void centres_par_ville(Centre centres[], int nb_centres, char villes[][VILLE_LEN], int counts[], int *nb_villes) {
    *nb_villes = 0;
    
    for (int i = 0; i < nb_centres; i++) {
        int found = 0;
        for (int j = 0; j < *nb_villes; j++) {
            if (strcmp(villes[j], centres[i].ville) == 0) {
                counts[j]++;
                found = 1;
                break;
            }
        }
        
        if (!found && *nb_villes < MAX_CENTRES) {
            strncpy(villes[*nb_villes], centres[i].ville, VILLE_LEN-1);
            villes[*nb_villes][VILLE_LEN-1] = '\0';
            counts[*nb_villes] = 1;
            (*nb_villes)++;
        }
    }
}
