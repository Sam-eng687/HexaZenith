#include "reservation.h"
#include "equipement.h"
#include "treeview.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

Cours lire_cours_reservation(char *filename, char *id_cours)
{
    Cours c;
    FILE *f = fopen(filename, "r");
    int trouve = 0;
    
    // Initialiser la structure
    strcpy(c.id_cours, "-1");
    strcpy(c.nom_cours, "");
    strcpy(c.type_activite, "");
    c.jour = 0;
    c.mois = 0;
    c.annee = 0;
    c.heure_debut = 0;
    c.heure_fin = 0;
    c.niveau = 0;
    c.places_max = 0;
    strcpy(c.centre, "");
    c.jours_mask = 0;
    
    if (f != NULL)
    {
        while (!trouve && fscanf(f, "%s %s %s %d %d %d %d %d %d %d %s %u",
                                 c.id_cours, c.nom_cours, c.type_activite,
                                 &c.jour, &c.mois, &c.annee,
                                 &c.heure_debut, &c.heure_fin,
                                 &c.niveau, &c.places_max,
                                 c.centre, &c.jours_mask) != EOF)
        {
            if (strcmp(c.id_cours, id_cours) == 0)
            {
                trouve = 1;
            }
        }
        fclose(f);
        
        if (!trouve)
        {
            strcpy(c.id_cours, "-1");
        }
    }
    
    return c;
}

const char* get_niveau_nom_reservation(int niveau)
{
    switch(niveau)
    {
        case 0: return "Debutant";
        case 1: return "Intermediaire";
        case 2: return "Avance";
        default: return "Inconnu";
    }
}

// Ajouter ou mettre à jour une réservation d'équipement
int ajouter_reservation_equipement(char *id_entraineur, char *id_cours, char *equip_id, char *equip_nom, char *equip_type, int quantite)
{
    FILE *f_res, *f_temp;
    char ligne[500];
    int reservation_trouvee = 0;
    int equipement_existe_deja = 0;
    
    // Lire le fichier de réservations
    f_res = fopen("./src/database/reservations.txt", "r");
    f_temp = fopen("./src/database/temp_reservations.txt", "w");
    
    if (f_temp == NULL) return 0;
    
    if (f_res != NULL)
    {
        while (fgets(ligne, sizeof(ligne), f_res) != NULL)
        {
            char id_entr_lu[MAX_STR], id_cours_lu[MAX_STR], equipements_data[400];
            
            if (sscanf(ligne, "%s %s %[^\n]", id_entr_lu, id_cours_lu, equipements_data) == 3)
            {
                if (strcmp(id_entr_lu, id_entraineur) == 0 && strcmp(id_cours_lu, id_cours) == 0)
                {
                    reservation_trouvee = 1;
                    
                    // Vérifier si l'équipement existe déjà
                    char nouvelle_ligne[500] = "";
                    char equipements_copy[400];
                    strcpy(equipements_copy, equipements_data);
                    char *token = strtok(equipements_copy, ";");
                    int premier = 1;
                    
                    while (token != NULL)
                    {
                        char eq_id[50], eq_nom[50], eq_type[30];
                        int qte;
                        
                        if (sscanf(token, "%[^,],%[^,],%[^,],%d", eq_id, eq_nom, eq_type, &qte) == 4)
                        {
                            if (strcmp(eq_id, equip_id) == 0)
                            {
                                equipement_existe_deja = 1;
                                qte += quantite;
                            }
                            
                            if (!premier) strcat(nouvelle_ligne, ";");
                            char temp[150];
                            sprintf(temp, "%s,%s,%s,%d", eq_id, eq_nom, eq_type, qte);
                            strcat(nouvelle_ligne, temp);
                            premier = 0;
                        }
                        
                        token = strtok(NULL, ";");
                    }
                    
                    // Si l'équipement n'existe pas, l'ajouter
                    if (!equipement_existe_deja)
                    {
                        if (!premier) strcat(nouvelle_ligne, ";");
                        char temp[150];
                        sprintf(temp, "%s,%s,%s,%d", equip_id, equip_nom, equip_type, quantite);
                        strcat(nouvelle_ligne, temp);
                    }
                    
                    fprintf(f_temp, "%s %s %s\n", id_entraineur, id_cours, nouvelle_ligne);
                }
                else
                {
                    fputs(ligne, f_temp);
                }
            }
            else
            {
                fputs(ligne, f_temp);
            }
        }
        fclose(f_res);
    }
    
    // Si la réservation n'existe pas, la créer
    if (!reservation_trouvee)
    {
        fprintf(f_temp, "%s %s %s,%s,%s,%d\n", id_entraineur, id_cours, equip_id, equip_nom, equip_type, quantite);
    }
    
    fclose(f_temp);
    
    // Remplacer le fichier original
    remove("./src/database/reservations.txt");
    rename("./src/database/temp_reservations.txt", "./src/database/reservations.txt");
    
    // Mettre à jour le stock en utilisant les fonctions de equipement.c
    Equipement e = chercher_equipement("./src/database/equipements.txt", equip_id);
    if (strcmp(e.id_equipement, "-1") != 0)
    {
        e.nombre -= quantite;
        if (e.nombre <= 0)
        {
            supprimer_equipement("./src/database/equipements.txt", equip_id);
        }
        else
        {
            modifier_equipement("./src/database/equipements.txt", equip_id, e);
        }
    }
    
    return 1;
}

// Supprimer un équipement spécifique d'une réservation
int supprimer_reservation_equipement(char *id_entraineur, char *id_cours, char *equip_id)
{
    FILE *f_res, *f_temp;
    char ligne[500];
    int trouve = 0;
    
    f_res = fopen("./src/database/reservations.txt", "r");
    f_temp = fopen("./src/database/temp_reservations.txt", "w");
    
    if (f_res == NULL || f_temp == NULL)
    {
        if (f_res) fclose(f_res);
        if (f_temp) fclose(f_temp);
        return 0;
    }
    
    while (fgets(ligne, sizeof(ligne), f_res) != NULL)
    {
        char id_entr[MAX_STR], id_cours_lu[MAX_STR], equipements_data[400];
        
        if (sscanf(ligne, "%s %s %[^\n]", id_entr, id_cours_lu, equipements_data) == 3)
        {
            if (strcmp(id_entr, id_entraineur) == 0 && strcmp(id_cours_lu, id_cours) == 0)
            {
                char nouvelle_ligne[500] = "";
                char equipements_copy[400];
                strcpy(equipements_copy, equipements_data);
                char *token = strtok(equipements_copy, ";");
                int premier = 1;
                
                while (token != NULL)
                {
                    char eq_id[50], eq_nom[50], eq_type[30];
                    int quantite;
                    
                    if (sscanf(token, "%[^,],%[^,],%[^,],%d", eq_id, eq_nom, eq_type, &quantite) == 4)
                    {
                        if (strcmp(eq_id, equip_id) == 0)
                        {
                            trouve = 1;
                            // Restaurer dans le stock
                            Equipement e = chercher_equipement("./src/database/equipements.txt", eq_id);
                            if (strcmp(e.id_equipement, "-1") != 0)
                            {
                                e.nombre += quantite;
                                modifier_equipement("./src/database/equipements.txt", eq_id, e);
                            }
                            else
                            {
                                // Recréer l'équipement
                                Equipement nouv;
                                strcpy(nouv.id_equipement, eq_id);
                                strcpy(nouv.nom_equipement, eq_nom);
                                strcpy(nouv.type, eq_type);
                                nouv.nombre = quantite;
                                ajouter_equipement("./src/database/equipements.txt", nouv);
                            }
                        }
                        else
                        {
                            if (!premier) strcat(nouvelle_ligne, ";");
                            char temp[150];
                            sprintf(temp, "%s,%s,%s,%d", eq_id, eq_nom, eq_type, quantite);
                            strcat(nouvelle_ligne, temp);
                            premier = 0;
                        }
                    }
                    
                    token = strtok(NULL, ";");
                }
                
                // Si il reste des équipements, écrire la ligne
                if (strlen(nouvelle_ligne) > 0)
                {
                    fprintf(f_temp, "%s %s %s\n", id_entr, id_cours_lu, nouvelle_ligne);
                }
                // Sinon, la réservation est vide, on ne l'écrit pas
            }
            else
            {
                fputs(ligne, f_temp);
            }
        }
        else
        {
            fputs(ligne, f_temp);
        }
    }
    
    fclose(f_res);
    fclose(f_temp);
    
    remove("./src/database/reservations.txt");
    rename("./src/database/temp_reservations.txt", "./src/database/reservations.txt");
    
    return trouve;
}

// Supprimer toutes les réservations d'un cours
int supprimer_toutes_reservations_cours(char *id_entraineur, char *id_cours)
{
    FILE *f_res, *f_temp;
    char ligne[500];
    int trouve = 0;
    
    f_res = fopen("./src/database/reservations.txt", "r");
    f_temp = fopen("./src/database/temp_reservations.txt", "w");
    
    if (f_res == NULL || f_temp == NULL)
    {
        if (f_res) fclose(f_res);
        if (f_temp) fclose(f_temp);
        return 0;
    }
    
    while (fgets(ligne, sizeof(ligne), f_res) != NULL)
    {
        char id_entr[MAX_STR], id_cours_lu[MAX_STR], equipements_data[400];
        
        if (sscanf(ligne, "%s %s %[^\n]", id_entr, id_cours_lu, equipements_data) == 3)
        {
            if (strcmp(id_entr, id_entraineur) == 0 && strcmp(id_cours_lu, id_cours) == 0)
            {
                trouve = 1;
                // Restaurer tous les équipements dans le stock
                char equipements_copy[400];
                strcpy(equipements_copy, equipements_data);
                char *token = strtok(equipements_copy, ";");
                
                while (token != NULL)
                {
                    char equip_id[50], equip_nom[50], equip_type[30];
                    int quantite;
                    
                    if (sscanf(token, "%[^,],%[^,],%[^,],%d", equip_id, equip_nom, equip_type, &quantite) == 4)
                    {
                        Equipement e = chercher_equipement("./src/database/equipements.txt", equip_id);
                        if (strcmp(e.id_equipement, "-1") != 0)
                        {
                            e.nombre += quantite;
                            modifier_equipement("./src/database/equipements.txt", equip_id, e);
                        }
                        else
                        {
                            // Recréer l'équipement
                            Equipement nouv;
                            strcpy(nouv.id_equipement, equip_id);
                            strcpy(nouv.nom_equipement, equip_nom);
                            strcpy(nouv.type, equip_type);
                            nouv.nombre = quantite;
                            ajouter_equipement("./src/database/equipements.txt", nouv);
                        }
                    }
                    
                    token = strtok(NULL, ";");
                }
                // Ne pas écrire cette ligne
                continue;
            }
        }
        
        fputs(ligne, f_temp);
    }
    
    fclose(f_res);
    fclose(f_temp);
    
    remove("./src/database/reservations.txt");
    rename("./src/database/temp_reservations.txt", "./src/database/reservations.txt");
    
    return trouve;
}
