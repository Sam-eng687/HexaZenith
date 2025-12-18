#include "equipement.h"
#include "treeview.h"
#include <string.h>

int ajouter_equipement(char *filename, Equipement e)
{
    FILE *f = fopen(filename, "a");
    if (f != NULL)
    {
        fprintf(f, "%s %s %s %d\n", e.id_equipement, e.nom_equipement, e.type, e.nombre);
        fclose(f);
        return 1;
    }
    return 0;
}

int modifier_equipement(char *filename, char *id, Equipement nouv)
{
    int tr = 0;
    Equipement e;
    FILE *f = fopen(filename, "r");
    FILE *f2 = fopen("./src/database/temp.txt", "w");
    
    if (f != NULL && f2 != NULL)
    {
        while (fscanf(f, "%s %s %s %d", e.id_equipement, e.nom_equipement, e.type, &e.nombre) != EOF)
        {
            if (strcmp(e.id_equipement, id) == 0)
            {
                fprintf(f2, "%s %s %s %d\n", nouv.id_equipement, nouv.nom_equipement, nouv.type, nouv.nombre);
                tr = 1;
            }
            else
            {
                fprintf(f2, "%s %s %s %d\n", e.id_equipement, e.nom_equipement, e.type, e.nombre);
            }
        }
        fclose(f);
        fclose(f2);
        remove(filename);
        rename("./src/database/temp.txt", filename);
    }
    else
    {
        if (f != NULL) fclose(f);
        if (f2 != NULL) fclose(f2);
    }
    
    return tr;
}

int supprimer_equipement(char *filename, char *id)
{
    int tr = 0;
    Equipement e;
    FILE *f = fopen(filename, "r");
    FILE *f2 = fopen("./src/database/temp.txt", "w");
    
    if (f != NULL && f2 != NULL)
    {
        while (fscanf(f, "%s %s %s %d", e.id_equipement, e.nom_equipement, e.type, &e.nombre) != EOF)
        {
            if (strcmp(e.id_equipement, id) == 0)
            {
                tr = 1;
            }
            else
            {
                fprintf(f2, "%s %s %s %d\n", e.id_equipement, e.nom_equipement, e.type, e.nombre);
            }
        }
        fclose(f);
        fclose(f2);
        remove(filename);
        rename("./src/database/temp.txt", filename);
    }
    else
    {
        if (f != NULL) fclose(f);
        if (f2 != NULL) fclose(f2);
    }
    
    return tr;
}

Equipement chercher_equipement(char *filename, char *id)
{
    Equipement e;
    int tr = 0;
    FILE *f = fopen(filename, "r");
    
    // Initialiser la structure
    strcpy(e.id_equipement, "-1");
    strcpy(e.nom_equipement, "");
    strcpy(e.type, "");
    e.nombre = 0;
    
    if (f != NULL)
    {
        while (tr == 0 && fscanf(f, "%s %s %s %d", e.id_equipement, e.nom_equipement, e.type, &e.nombre) != EOF)
        {
            if (strcmp(e.id_equipement, id) == 0)
            {
                tr = 1;
            }
        }
        fclose(f);
        
        if (tr == 0)
        {
            strcpy(e.id_equipement, "-1");
        }
    }
    
    return e;
}

int equipement_id_existe(char *filename, char *id)
{
    Equipement e;
    FILE *f = fopen(filename, "r");
    
    if (f != NULL)
    {
        while (fscanf(f, "%s %s %s %d", e.id_equipement, e.nom_equipement, e.type, &e.nombre) != EOF)
        {
            if (strcmp(e.id_equipement, id) == 0)
            {
                fclose(f);
                return 1; // ID existe
            }
        }
        fclose(f);
    }
    
    return 0; // ID n'existe pas
}
