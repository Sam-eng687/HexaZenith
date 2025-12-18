#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "treeview.h"
#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "centres.h"
// Global path for private trainers file
extern char ENTRAINEUR_PRIVE_FILE_PATH[256];

// Helper function to get niveau name
static const char* get_niveau_nom(int niveau) {
    switch(niveau) {
        case 0: return "Debutant";
        case 1: return "Intermediaire";
        case 2: return "Avance";
        default: return "Inconnu";
    }
}

// Function to read a specific cours from file
static Cours lire_cours(const char *filename, const char *id_cours) {
    Cours c;
    FILE *f = fopen(filename, "r");
    strcpy(c.id_cours, "-1");
    
    if (f != NULL) {
        while (fscanf(f, "%s %s %s %d %d %d %d %d %d %d %s %u", 
                     c.id_cours, c.nom_cours, c.type_activite, 
                     &c.jour, &c.mois, &c.annee, 
                     &c.heure_debut, &c.heure_fin, 
                     &c.niveau, &c.places_max, c.centre, &c.jours_mask) == 12) {
            if (strcmp(c.id_cours, id_cours) == 0) {
                fclose(f);
                return c;
            }
        }
        fclose(f);
        strcpy(c.id_cours, "-1");
    }
    return c;
}

// Function to read all courses from file
static int lire_tous_les_cours(const char *filename, Cours *tab, int max_size) {
    FILE *f = fopen(filename, "r");
    int count = 0;
    
    if (f == NULL) return 0;
    
    while (count < max_size && 
           fscanf(f, "%s %s %s %d %d %d %d %d %d %d %s %u",
                  tab[count].id_cours, tab[count].nom_cours, tab[count].type_activite,
                  &tab[count].jour, &tab[count].mois, &tab[count].annee,
                  &tab[count].heure_debut, &tab[count].heure_fin,
                  &tab[count].niveau, &tab[count].places_max, 
                  tab[count].centre, &tab[count].jours_mask) == 12) {
        count++;
    }
    
    fclose(f);
    return count;
}

// Function to display courses for a trainer
void refresh_trainer_courses_treeview(GtkWidget *liste, const char *id_entraineur_param) {
    GtkListStore *store;
    GtkTreeModel *model;
    GtkTreeIter iter;
    FILE *f_inscriptions;
    char id_entr_lu[MAX_STR], id_cours[MAX_STR];
    Cours c;
    char date_str[50], horaire_str[50];
    
    if (liste == NULL) return;
    
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(liste));
    
    if (model == NULL) {
        GtkCellRenderer *renderer;
        GtkTreeViewColumn *column;
        
        store = gtk_list_store_new(8, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, 
                                   G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, 
                                   G_TYPE_INT, G_TYPE_STRING);
        
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("ID", renderer, "text", 0, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(liste), column);
        
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Nom", renderer, "text", 1, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(liste), column);
        
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Type", renderer, "text", 2, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(liste), column);
        
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Date", renderer, "text", 3, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(liste), column);
        
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Horaire", renderer, "text", 4, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(liste), column);
        
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Niveau", renderer, "text", 5, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(liste), column);
        
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Places", renderer, "text", 6, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(liste), column);
        
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Centre", renderer, "text", 7, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(liste), column);
        
        gtk_tree_view_set_model(GTK_TREE_VIEW(liste), GTK_TREE_MODEL(store));
        g_object_unref(store);
    } else {
        store = GTK_LIST_STORE(model);
        gtk_list_store_clear(store);
    }
    
    f_inscriptions = fopen(INSCRIPTION_ENTRAINEUR_FILE_PATH, "r");
    if (f_inscriptions != NULL) {
        while (fscanf(f_inscriptions, "%s %s", id_entr_lu, id_cours) == 2) {
            if (strcmp(id_entr_lu, id_entraineur_param) == 0) {
                c = lire_cours(COURS_FILE_PATH, id_cours);
                
                if (strcmp(c.id_cours, "-1") != 0) {
                    sprintf(date_str, "%02d/%02d/%04d", c.jour, c.mois, c.annee);
                    sprintf(horaire_str, "%02d:00-%02d:00", c.heure_debut, c.heure_fin);
                    
                    gtk_list_store_append(store, &iter);
                    gtk_list_store_set(store, &iter, 
                                      0, c.id_cours, 
                                      1, c.nom_cours, 
                                      2, c.type_activite, 
                                      3, date_str, 
                                      4, horaire_str, 
                                      5, get_niveau_nom(c.niveau), 
                                      6, c.places_max, 
                                      7, c.centre, 
                                      -1);
                }
            }
        }
        fclose(f_inscriptions);
    }
}

// Function to display all equipment
void refresh_equipment_treeview(GtkWidget *liste) {
    GtkListStore *store;
    GtkTreeModel *model;
    GtkTreeIter iter;
    Equipement e;
    FILE *f;
    
    if (liste == NULL) return;
    
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(liste));
    
    if (model == NULL) {
        GtkCellRenderer *renderer;
        GtkTreeViewColumn *column;
        
        store = gtk_list_store_new(4, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT);
        
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("ID", renderer, "text", 0, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(liste), column);
        
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Nom", renderer, "text", 1, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(liste), column);
        
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Type", renderer, "text", 2, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(liste), column);
        
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Quantité", renderer, "text", 3, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(liste), column);
        
        gtk_tree_view_set_model(GTK_TREE_VIEW(liste), GTK_TREE_MODEL(store));
        g_object_unref(store);
    } else {
        store = GTK_LIST_STORE(model);
        gtk_list_store_clear(store);
    }
    
    f = fopen(EQUIPEMENT_FILE_PATH, "r");
    if (f != NULL) {
        store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(liste)));
        while (fscanf(f, "%s %s %s %d", e.id_equipement, e.nom_equipement, e.type, &e.nombre) != EOF) {
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter, 
                              0, e.id_equipement, 
                              1, e.nom_equipement, 
                              2, e.type, 
                              3, e.nombre, 
                              -1);
        }
        fclose(f);
    }
}

// Function to load equipment types into combobox
void refresh_equipment_types_combobox(GtkWidget *combobox, int ajouter_aucune) {
    FILE *f;
    Equipement e;
    GtkTreeModel *model;
    char types[100][MAX_STR];
    int nb_types = 0;
    int i, j, existe;
    
    if (combobox == NULL) return;
    
    model = gtk_combo_box_get_model(GTK_COMBO_BOX(combobox));
    if (model != NULL) {
        gtk_list_store_clear(GTK_LIST_STORE(model));
    }
    
    if (ajouter_aucune) {
        gtk_combo_box_append_text(GTK_COMBO_BOX(combobox), "Aucune");
    }
    
    f = fopen(EQUIPEMENT_FILE_PATH, "r");
    if (f != NULL) {
        while (fscanf(f, "%s %s %s %d", e.id_equipement, e.nom_equipement, e.type, &e.nombre) != EOF) {
            existe = 0;
            for (j = 0; j < nb_types; j++) {
                if (strcmp(types[j], e.type) == 0) {
                    existe = 1;
                    break;
                }
            }
            if (!existe && nb_types < 100) {
                strcpy(types[nb_types], e.type);
                nb_types++;
            }
        }
        fclose(f);
        
        for (i = 0; i < nb_types; i++) {
            gtk_combo_box_append_text(GTK_COMBO_BOX(combobox), types[i]);
        }
    }
    
    gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), 0);
}

// Function to load activity types into combobox
void refresh_activity_types_combobox(GtkWidget *combobox) {
    FILE *f;
    Cours c;
    GtkTreeModel *model;
    char types[100][MAX_STR];
    int nb_types = 0;
    int i, j, existe;
    
    if (combobox == NULL) return;
    
    model = gtk_combo_box_get_model(GTK_COMBO_BOX(combobox));
    if (model != NULL) {
        gtk_list_store_clear(GTK_LIST_STORE(model));
    }
    
    gtk_combo_box_append_text(GTK_COMBO_BOX(combobox), "Aucune");
    
    f = fopen(COURS_FILE_PATH, "r");
    if (f != NULL) {
        while (fscanf(f, "%s %s %s %d %d %d %d %d %d %d %s %u", 
                     c.id_cours, c.nom_cours, c.type_activite, 
                     &c.jour, &c.mois, &c.annee, 
                     &c.heure_debut, &c.heure_fin, 
                     &c.niveau, &c.places_max, c.centre, &c.jours_mask) == 12) {
            existe = 0;
            for (j = 0; j < nb_types; j++) {
                if (strcmp(types[j], c.type_activite) == 0) {
                    existe = 1;
                    break;
                }
            }
            if (!existe && nb_types < 100) {
                strcpy(types[nb_types], c.type_activite);
                nb_types++;
            }
        }
        fclose(f);
        
        for (i = 0; i < nb_types; i++) {
            gtk_combo_box_append_text(GTK_COMBO_BOX(combobox), types[i]);
        }
    }
    
    gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), 0);
}

// Function to refresh admin courses treeview (shows all courses)
void refresh_admin_courses_treeview(GtkWidget *treeview) {
    if (!treeview) return;
    
    Cours tab[MAX_COURS];
    int n = lire_tous_les_cours(COURS_FILE_PATH, tab, MAX_COURS);
    
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
    GtkListStore *store;
    
    if (model == NULL) {
        GtkCellRenderer *renderer;
        GtkTreeViewColumn *column;
        
        store = gtk_list_store_new(8, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                                   G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                                   G_TYPE_INT, G_TYPE_STRING);
        
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("ID", renderer, "text", 0, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
        
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Nom", renderer, "text", 1, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
        
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Type", renderer, "text", 2, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
        
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Date", renderer, "text", 3, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
        
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Horaire", renderer, "text", 4, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
        
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Niveau", renderer, "text", 5, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
        
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Places", renderer, "text", 6, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
        
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Centre", renderer, "text", 7, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
        
        gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(store));
        g_object_unref(store);
    } else {
        store = GTK_LIST_STORE(model);
        gtk_list_store_clear(store);
    }
    
    GtkTreeIter iter;
    for (int i = 0; i < n; i++) {
        char date_str[50], horaire_str[50];
        sprintf(date_str, "%02d/%02d/%04d", tab[i].jour, tab[i].mois, tab[i].annee);
        sprintf(horaire_str, "%02d:00-%02d:00", tab[i].heure_debut, tab[i].heure_fin);
        
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                          0, tab[i].id_cours,
                          1, tab[i].nom_cours,
                          2, tab[i].type_activite,
                          3, date_str,
                          4, horaire_str,
                          5, get_niveau_nom(tab[i].niveau),
                          6, tab[i].places_max,
                          7, tab[i].centre,
                          -1);
    }
}

// Function to refresh member courses treeview (shows only courses with available places)
void refresh_member_courses_treeview(GtkWidget *treeview) {
    if (!treeview) return;
    
    Cours tab[MAX_COURS];
    int n = lire_tous_les_cours(COURS_FILE_PATH, tab, MAX_COURS);
    
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
    GtkListStore *store;
    
    if (model == NULL) {
        GtkCellRenderer *renderer;
        GtkTreeViewColumn *column;
        
        store = gtk_list_store_new(8, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                                   G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING,
                                   G_TYPE_INT, G_TYPE_STRING);
        
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("ID", renderer, "text", 0, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
        
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Nom", renderer, "text", 1, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
        
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Type", renderer, "text", 2, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
        
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Date", renderer, "text", 3, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
        
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Horaire", renderer, "text", 4, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
        
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Niveau", renderer, "text", 5, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
        
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Places", renderer, "text", 6, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
        
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Centre", renderer, "text", 7, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
        
        gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(store));
        g_object_unref(store);
    } else {
        store = GTK_LIST_STORE(model);
        gtk_list_store_clear(store);
    }
    
    GtkTreeIter iter;
    for (int i = 0; i < n; i++) {
        // Only show courses with available places
        if (tab[i].places_max <= 0) continue;
        
        char date_str[50], horaire_str[50];
        sprintf(date_str, "%02d/%02d/%04d", tab[i].jour, tab[i].mois, tab[i].annee);
        sprintf(horaire_str, "%02d:00-%02d:00", tab[i].heure_debut, tab[i].heure_fin);
        
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
                          0, tab[i].id_cours,
                          1, tab[i].nom_cours,
                          2, tab[i].type_activite,
                          3, date_str,
                          4, horaire_str,
                          5, get_niveau_nom(tab[i].niveau),
                          6, tab[i].places_max,
                          7, tab[i].centre,
                          -1);
    }
}
static int lire_centres(const char *filename, Centre *tab, int max_size) {
    FILE *f = fopen(filename, "r");
    int count = 0;
    
    if (f == NULL) {
        g_warning("Cannot open centres file: %s", filename);
        return 0;
    }
    
    char line[512];
    while (count < max_size && fgets(line, sizeof(line), f)) {
        // Skip empty lines
        if (line[0] == '\n' || line[0] == '\0') continue;
        
        int result = sscanf(line, "%[^,],%[^,],%[^,],%d,%d,%d,%d,%d,%[^\n]",
                  tab[count].id,
                  tab[count].nom,
                  tab[count].type,
                  &tab[count].capacite,
                  &tab[count].ouvert,
                  &tab[count].parking,
                  &tab[count].wifi,
                  &tab[count].cafeteria,
                  tab[count].ville);
        
        if (result == 9) {
            count++;
        }
    }
    
    fclose(f);
    return count;
}
// Function to setup centres treeview columns
static void setup_centres_treeview(GtkTreeView *treeview) {
    if (!treeview) return;
    
    GtkTreeModel *model = gtk_tree_view_get_model(treeview);
    if (model == NULL) {
        GtkListStore *store = gtk_list_store_new(6, 
            G_TYPE_STRING,  // ID
            G_TYPE_STRING,  // Nom
            G_TYPE_STRING,  // Type
            G_TYPE_INT,     // Capacité
            G_TYPE_STRING,  // Statut (Ouvert/Fermé)
            G_TYPE_STRING   // Ville
        );
        gtk_tree_view_set_model(treeview, GTK_TREE_MODEL(store));
        g_object_unref(store);
        
        GtkCellRenderer *renderer;
        GtkTreeViewColumn *column;
        
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("ID", renderer, "text", 0, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(treeview, column);
        
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Nom", renderer, "text", 1, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(treeview, column);
        
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Type", renderer, "text", 2, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(treeview, column);
        
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Capacité", renderer, "text", 3, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(treeview, column);
        
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Statut", renderer, "text", 4, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(treeview, column);
        
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Ville", renderer, "text", 5, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(treeview, column);
    }
}

// Function to refresh admin centres treeview
void refresh_admin_centres_treeview(GtkWidget *treeview) {
    if (!treeview) return;
    
    Centre tab[MAX_CENTRES];
    int n = lire_centres(CENTRE_FILE_PATH, tab, MAX_CENTRES);
    
    setup_centres_treeview(GTK_TREE_VIEW(treeview));
    
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
    GtkListStore *store = GTK_LIST_STORE(model);
    gtk_list_store_clear(store);
    
    GtkTreeIter iter;
    for (int i = 0; i < n; i++) {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
            0, tab[i].id,
            1, tab[i].nom,
            2, tab[i].type,
            3, tab[i].capacite,
            4, tab[i].ouvert ? "Ouvert" : "Fermé",
            5, tab[i].ville,
            -1);
    }
}

// Function to refresh trainer centres treeview
void refresh_trainer_centres_treeview(GtkWidget *treeview) {
    if (!treeview) return;
    
    Centre tab[MAX_CENTRES];
    int n = lire_centres(CENTRE_FILE_PATH, tab, MAX_CENTRES);
    
    setup_centres_treeview(GTK_TREE_VIEW(treeview));
    
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
    GtkListStore *store = GTK_LIST_STORE(model);
    gtk_list_store_clear(store);
    
    GtkTreeIter iter;
    for (int i = 0; i < n; i++) {
        // Only show open centres for trainers
        if (tab[i].ouvert) {
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter,
                0, tab[i].id,
                1, tab[i].nom,
                2, tab[i].type,
                3, tab[i].capacite,
                4, "Ouvert",  // Always "Ouvert" since we filtered
                5, tab[i].ville,
                -1);
        }
    }
}

// Function to populate centre types combobox for admin
void refresh_centre_types_combobox(GtkWidget *combobox, const char *filename) {
    if (!combobox) return;
    
    Centre tab[MAX_CENTRES];
    int n = lire_centres(filename, tab, MAX_CENTRES);
    
    GtkTreeModel *model = gtk_combo_box_get_model(GTK_COMBO_BOX(combobox));
    if (model != NULL) {
        gtk_list_store_clear(GTK_LIST_STORE(model));
    }
    
    // Set up cell renderer
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_clear(GTK_CELL_LAYOUT(combobox));
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combobox), renderer, TRUE);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combobox), renderer, "text", 0, NULL);
    
    // Create new model if needed
    if (model == NULL) {
        GtkListStore *store = gtk_list_store_new(1, G_TYPE_STRING);
        gtk_combo_box_set_model(GTK_COMBO_BOX(combobox), GTK_TREE_MODEL(store));
        g_object_unref(store);
        model = gtk_combo_box_get_model(GTK_COMBO_BOX(combobox));
    }
    
    GtkListStore *store = GTK_LIST_STORE(model);
    GtkTreeIter iter;
    
    // Add "Tous les types" option
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Tous les types", -1);
    
    // Track unique types
    char types[MAX_CENTRES][MAX_STR];
    int nb_types = 0;
    
    for (int i = 0; i < n; i++) {
        if (tab[i].type[0] == '\0') continue;
        
        int exists = 0;
        for (int j = 0; j < nb_types; j++) {
            if (strcmp(types[j], tab[i].type) == 0) {
                exists = 1;
                break;
            }
        }
        
        if (!exists && nb_types < MAX_CENTRES) {
            strcpy(types[nb_types], tab[i].type);
            nb_types++;
            
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter, 0, tab[i].type, -1);
        }
    }
    
    gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), 0);
}

// Function to populate centre types combobox for trainer
void refresh_trainer_centre_types_combobox(GtkWidget *combobox, const char *filename) {
    if (!combobox) return;
    
    Centre tab[MAX_CENTRES];
    int n = lire_centres(filename, tab, MAX_CENTRES);
    
    GtkTreeModel *model = gtk_combo_box_get_model(GTK_COMBO_BOX(combobox));
    if (model != NULL) {
        gtk_list_store_clear(GTK_LIST_STORE(model));
    }
    
    // Set up cell renderer
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_clear(GTK_CELL_LAYOUT(combobox));
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combobox), renderer, TRUE);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combobox), renderer, "text", 0, NULL);
    
    // Create new model if needed
    if (model == NULL) {
        GtkListStore *store = gtk_list_store_new(1, G_TYPE_STRING);
        gtk_combo_box_set_model(GTK_COMBO_BOX(combobox), GTK_TREE_MODEL(store));
        g_object_unref(store);
        model = gtk_combo_box_get_model(GTK_COMBO_BOX(combobox));
    }
    
    GtkListStore *store = GTK_LIST_STORE(model);
    GtkTreeIter iter;
    
    // Add "Tous les types" option
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Tous les types", -1);
    
    // Track unique types from OPEN centres only
    char types[MAX_CENTRES][MAX_STR];
    int nb_types = 0;
    
    for (int i = 0; i < n; i++) {
        // Only consider open centres for trainers
        if (!tab[i].ouvert || tab[i].type[0] == '\0') continue;
        
        int exists = 0;
        for (int j = 0; j < nb_types; j++) {
            if (strcmp(types[j], tab[i].type) == 0) {
                exists = 1;
                break;
            }
        }
        
        if (!exists && nb_types < MAX_CENTRES) {
            strcpy(types[nb_types], tab[i].type);
            nb_types++;
            
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter, 0, tab[i].type, -1);
        }
    }
    
    gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), 0);
}
static int lire_tous_les_events(const char *filename, evenement *tab, int max_size) {
    FILE *f = fopen(filename, "r");
    int count = 0;
    
    if (f == NULL) {
        g_warning("Cannot open events file: %s", filename);
        return 0;
    }
    
    char line[512];
    while (count < max_size && fgets(line, sizeof(line), f)) {
        // Skip empty lines
        if (line[0] == '\n' || line[0] == '\0') continue;
        
        int result = sscanf(line, "%6[^,],%299[^,],%99[^,],%199[^,],%d/%d/%d,%d",
                  tab[count].id_event,
                  tab[count].nom_event,
                  tab[count].type_event,
                  tab[count].centre,
                  &tab[count].jour,
                  &tab[count].mois,
                  &tab[count].annee,
                  &tab[count].type_acces);
        
        if (result == 8) {
            count++;
        }
    }
    
    fclose(f);
    return count;
}

// Setup event treeview columns
static void setup_events_treeview(GtkTreeView *treeview) {
    if (!treeview) return;
    
    // Check if already setup by getting the columns list
    GList *columns = gtk_tree_view_get_columns(treeview);
    if (columns != NULL) {
        g_list_free(columns);
        return; // Already setup
    }
    
    // Create model
    GtkListStore *store = gtk_list_store_new(6,
        G_TYPE_STRING,      // ID
        G_TYPE_STRING,   // Nom
        G_TYPE_STRING,   // Type
        G_TYPE_STRING,   // Centre
        G_TYPE_STRING,   // Date (formatted string)
        G_TYPE_STRING    // Type Acces (Gratuit/Payant)
    );
    gtk_tree_view_set_model(treeview, GTK_TREE_MODEL(store));
    g_object_unref(store);
    
    // Add columns
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("ID", renderer, "text", 0, NULL);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_append_column(treeview, column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Nom", renderer, "text", 1, NULL);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_append_column(treeview, column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Type", renderer, "text", 2, NULL);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_append_column(treeview, column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Centre", renderer, "text", 3, NULL);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_append_column(treeview, column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Date", renderer, "text", 4, NULL);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_append_column(treeview, column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Accès", renderer, "text", 5, NULL);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_append_column(treeview, column);
}

// Refresh admin events treeview (shows all events)
void refresh_admin_events_treeview(GtkWidget *treeview) {
    if (!treeview) return;
    
    evenement tab[1000];
    int n = lire_tous_les_events(EVENT_FILE_PATH, tab, 1000);
    
    setup_events_treeview(GTK_TREE_VIEW(treeview));
    
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
    GtkListStore *store = GTK_LIST_STORE(model);
    gtk_list_store_clear(store);
    
    GtkTreeIter iter;
    for (int i = 0; i < n; i++) {
        char date_str[20];
        snprintf(date_str, sizeof(date_str), "%02d/%02d/%04d", 
                 tab[i].jour, tab[i].mois, tab[i].annee);
        
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
            0, tab[i].id_event,
            1, tab[i].nom_event,
            2, tab[i].type_event,
            3, tab[i].centre,
            4, date_str,
            5, tab[i].type_acces ? "Payant" : "Gratuit",
            -1);
    }
}

static int lire_tous_les_entraineurs(const char *filename, Entraineur *tab, int max_size) {
    FILE *f = fopen(filename, "r");
    int count = 0;
    
    if (f == NULL) {
        g_warning("Cannot open trainers file: %s", filename);
        return 0;
    }
    
    char line[512];
    while (count < max_size && fgets(line, sizeof(line), f)) {
        // Skip empty lines
        if (line[0] == '\n' || line[0] == '\0') continue;
        
        int result = sscanf(line, "%d,%99[^,],%99[^,],%d,%99[^,],%d/%d/%d,%d",
                  &tab[count].CIN_trainer,
                  tab[count].Nom_trainer,
                  tab[count].Prenom_trainer,
                  &tab[count].sexe_trainer,
                  tab[count].specialite,
                  &tab[count].jour,
                  &tab[count].mois,
                  &tab[count].annee,
                  &tab[count].train_priv);
        
        if (result == 9) {
            count++;
        }
    }
    
    fclose(f);
    return count;
}
static void setup_trainers_treeview(GtkTreeView *treeview) {
    if (!treeview) return;
    
    // Check if already setup
    GList *columns = gtk_tree_view_get_columns(treeview);
    if (columns != NULL) {
        g_list_free(columns);
        return;
    }
    
    // Create model with 7 columns
    GtkListStore *store = gtk_list_store_new(7,
        G_TYPE_INT,      // CIN
        G_TYPE_STRING,   // Nom
        G_TYPE_STRING,   // Prenom
        G_TYPE_STRING,   // Sexe
        G_TYPE_STRING,   // Specialite
        G_TYPE_STRING,   // Date naissance
        G_TYPE_STRING    // Type (Privé/Public)
    );
    gtk_tree_view_set_model(treeview, GTK_TREE_MODEL(store));
    g_object_unref(store);
    
    // Add columns
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("CIN", renderer, "text", 0, NULL);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_append_column(treeview, column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Nom", renderer, "text", 1, NULL);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_append_column(treeview, column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Prénom", renderer, "text", 2, NULL);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_append_column(treeview, column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Sexe", renderer, "text", 3, NULL);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_append_column(treeview, column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Spécialité", renderer, "text", 4, NULL);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_append_column(treeview, column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Date Naissance", renderer, "text", 5, NULL);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_append_column(treeview, column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Type", renderer, "text", 6, NULL);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_append_column(treeview, column);
}

void refresh_admin_trainers_treeview(GtkWidget *treeview) {
    if (!treeview) return;
    
    Entraineur tab[1000];
    int n = lire_tous_les_entraineurs(ENTRAINEUR_FILE_PATH, tab, 1000);
    
    setup_trainers_treeview(GTK_TREE_VIEW(treeview));
    
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
    GtkListStore *store = GTK_LIST_STORE(model);
    gtk_list_store_clear(store);
    
    GtkTreeIter iter;
    for (int i = 0; i < n; i++) {
        char date_str[20];
        snprintf(date_str, sizeof(date_str), "%02d/%02d/%04d", 
                 tab[i].jour, tab[i].mois, tab[i].annee);
        
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
            0, tab[i].CIN_trainer,
            1, tab[i].Nom_trainer,
            2, tab[i].Prenom_trainer,
            3, tab[i].sexe_trainer ? "Femme" : "Homme",
            4, tab[i].specialite,
            5, date_str,
            6, tab[i].train_priv ? "Privé" : "Public",
            -1);
    }
}


// Helper function to read all membres from file
static int lire_tous_les_membres(const char *filename, Membre *tab, int max_size) {
    FILE *f = fopen(filename, "r");
    int count = 0;
    
    if (f == NULL) {
        g_warning("Cannot open membres file: %s", filename);
        return 0;
    }
    
    char line[512];
    while (count < max_size && fgets(line, sizeof(line), f)) {
        // Skip empty lines
        if (line[0] == '\n' || line[0] == '\0') continue;
        
        int result = sscanf(line, "%[^|]|%[^|]|%[^|]|%[^|]|%[^|]|%[^\n]",
                  tab[count].cin,
                  tab[count].nom,
                  tab[count].prenom,
                  tab[count].sexe,
                  tab[count].telephone,
                  tab[count].date_inscription);
        
        if (result == 6) {
            count++;
        }
    }
    
    fclose(f);
    return count;
}

// Setup membres treeview columns
static void setup_membres_treeview(GtkTreeView *treeview) {
    if (!treeview) return;
    
    // Check if already setup
    GList *columns = gtk_tree_view_get_columns(treeview);
    if (columns != NULL) {
        g_list_free(columns);
        return;
    }
    
    // Create model with 6 columns
    GtkListStore *store = gtk_list_store_new(6,
        G_TYPE_STRING,   // CIN
        G_TYPE_STRING,   // Nom
        G_TYPE_STRING,   // Prenom
        G_TYPE_STRING,   // Sexe
        G_TYPE_STRING,   // Telephone
        G_TYPE_STRING    // Date inscription
    );
    gtk_tree_view_set_model(treeview, GTK_TREE_MODEL(store));
    g_object_unref(store);
    
    // Add columns
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("CIN", renderer, "text", 0, NULL);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_append_column(treeview, column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Nom", renderer, "text", 1, NULL);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_append_column(treeview, column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Prénom", renderer, "text", 2, NULL);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_append_column(treeview, column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Sexe", renderer, "text", 3, NULL);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_append_column(treeview, column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Téléphone", renderer, "text", 4, NULL);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_append_column(treeview, column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Date Inscription", renderer, "text", 5, NULL);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_append_column(treeview, column);
}

// Refresh admin membres treeview
void refresh_admin_membres_treeview(GtkWidget *treeview) {
    if (!treeview) return;
    
    Membre tab[1000];
    int n = lire_tous_les_membres(MEMBRE_FILE_PATH, tab, 1000);
    
    setup_membres_treeview(GTK_TREE_VIEW(treeview));
    
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
    GtkListStore *store = GTK_LIST_STORE(model);
    gtk_list_store_clear(store);
    
    GtkTreeIter iter;
    for (int i = 0; i < n; i++) {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
            0, tab[i].cin,
            1, tab[i].nom,
            2, tab[i].prenom,
            3, tab[i].sexe,
            4, tab[i].telephone,
            5, tab[i].date_inscription,
            -1);
    }
}

// Helper function to check if private trainer exists in file
static int entraineur_prive_existe(int cin, EntraineurPrive *ep) {
    FILE *f = fopen(ENTRAINEUR_PRIVE_FILE_PATH, "r");
    if (f == NULL) return 0;
    
    char line[512];
    int found = 0;
    
    while (fgets(line, sizeof(line), f)) {
        if (line[0] == '\n' || line[0] == '\0') continue;
        
        int file_cin;
        char file_nom[100], file_prenom[100], file_spec[100], file_jours[20];
        file_jours[0] = '\0'; // Initialize to empty
        
        int result = sscanf(line, "%d|%[^|]|%[^|]|%[^|]|%[^\n]", 
                   &file_cin, file_nom, file_prenom, file_spec, file_jours);
        
        // Check both 5 fields and 4 fields format
        if (result >= 4 && file_cin == cin) {
            found = 1;
            if (ep != NULL) {
                ep->cin = file_cin;
                strcpy(ep->nom, file_nom);
                strcpy(ep->prenom, file_prenom);
                strcpy(ep->specialite, file_spec);
                strcpy(ep->jours_disponibles, file_jours);
            }
            break;
        }
    }
    
    fclose(f);
    return found;
}

// Helper function to add private trainer to file
static void ajouter_entraineur_prive(int cin, const char *nom, const char *prenom, const char *specialite) {
    FILE *f = fopen(ENTRAINEUR_PRIVE_FILE_PATH, "a");
    if (f == NULL) {
        g_warning("Cannot open private trainers file for writing: %s", ENTRAINEUR_PRIVE_FILE_PATH);
        return;
    }
    
    // Add with CIN and all 7 days available: L M Me J V S D
    fprintf(f, "%d|%s|%s|%s|L M Me J V S\n", cin, nom, prenom, specialite);
    fclose(f);
}

// Helper function to synchronize private trainers
static void synchroniser_entraineurs_prives() {
    // Read all trainers from entraineurs.txt
    Entraineur tab[1000];
    int n = lire_tous_les_entraineurs(ENTRAINEUR_FILE_PATH, tab, 1000);
    
    // Create file if it doesn't exist
    FILE *f = fopen(ENTRAINEUR_PRIVE_FILE_PATH, "a+");
    if (f != NULL) {
        fclose(f);
    }
    
    // Check each private trainer
    for (int i = 0; i < n; i++) {
        if (tab[i].train_priv == 1) { // Private trainer
            if (!entraineur_prive_existe(tab[i].CIN_trainer, NULL)) {
                ajouter_entraineur_prive(tab[i].CIN_trainer, tab[i].Nom_trainer, 
                                        tab[i].Prenom_trainer, tab[i].specialite);
            }
        }
    }
}

// Helper function to read private trainers with available days
static int lire_entraineurs_prives_disponibles(EntraineurPrive *tab, int max_size) {
    synchroniser_entraineurs_prives();
    
    FILE *f = fopen(ENTRAINEUR_PRIVE_FILE_PATH, "r");
    int count = 0;
    
    if (f == NULL) {
        g_warning("Cannot open private trainers file: %s", ENTRAINEUR_PRIVE_FILE_PATH);
        return 0;
    }
    
    char line[512];
    while (count < max_size && fgets(line, sizeof(line), f)) {
        if (line[0] == '\n' || line[0] == '\0') continue;
        
        EntraineurPrive ep;
        // Initialize jours_disponibles to empty
        ep.jours_disponibles[0] = '\0';
        
        int result = sscanf(line, "%d|%[^|]|%[^|]|%[^|]|%[^\n]", 
                   &ep.cin, ep.nom, ep.prenom, ep.specialite, ep.jours_disponibles);
        
        // Accept both 5 fields (with jours) or 4 fields (without jours)
        if (result >= 4) {
            // Trim whitespace from jours_disponibles
            char *start = ep.jours_disponibles;
            char *end;
            
            // Trim leading spaces
            while (*start == ' ') start++;
            
            // Trim trailing spaces
            end = start + strlen(start) - 1;
            while (end > start && (*end == ' ' || *end == '\n' || *end == '\r')) end--;
            *(end + 1) = '\0';
            
            // Move trimmed string to beginning
            if (start != ep.jours_disponibles) {
                memmove(ep.jours_disponibles, start, strlen(start) + 1);
            }
            
            // Only add if they have at least one available day
            if (strlen(ep.jours_disponibles) > 0) {
                tab[count] = ep;
                count++;
            }
        }
    }
    
    fclose(f);
    return count;
}
// Setup private trainers treeview columns
static void setup_entraineurs_prives_treeview(GtkTreeView *treeview) {
    if (!treeview) return;
    
    // Check if already setup
    GList *columns = gtk_tree_view_get_columns(treeview);
    if (columns != NULL) {
        g_list_free(columns);
        return;
    }
    
    // Create model with 4 columns
    GtkListStore *store = gtk_list_store_new(4,
        G_TYPE_STRING,   // Nom
        G_TYPE_STRING,   // Prenom
        G_TYPE_STRING,   // Specialite
        G_TYPE_STRING    // Jours Disponibles
    );
    gtk_tree_view_set_model(treeview, GTK_TREE_MODEL(store));
    g_object_unref(store);
    
    // Add columns
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Nom", renderer, "text", 0, NULL);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_append_column(treeview, column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Prénom", renderer, "text", 1, NULL);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_append_column(treeview, column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Spécialité", renderer, "text", 2, NULL);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_append_column(treeview, column);
    
    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("Jours Disponibles", renderer, "text", 3, NULL);
    gtk_tree_view_column_set_resizable(column, TRUE);
    gtk_tree_view_append_column(treeview, column);
}

// Refresh membre entraineurs prives treeview (only shows trainers with available days)
void refresh_membre_entraineurs_prives_treeview(GtkWidget *treeview) {
    if (!treeview) return;
    
    EntraineurPrive tab[1000];
    int n = lire_entraineurs_prives_disponibles(tab, 1000);
    
    setup_entraineurs_prives_treeview(GTK_TREE_VIEW(treeview));
    
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
    GtkListStore *store = GTK_LIST_STORE(model);
    gtk_list_store_clear(store);
    
    GtkTreeIter iter;
    for (int i = 0; i < n; i++) {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
            0, tab[i].nom,
            1, tab[i].prenom,
            2, tab[i].specialite,
            3, tab[i].jours_disponibles,
            -1);
    }
}

// Add these functions to the end of your treeview.c file

// Generic function to refresh centre treeview (used in callbacks.c)
void refresh_treeview(GtkTreeView *treeview, Centre centres[], int nb_centres) {
    if (!treeview) return;
    
    GtkTreeModel *model = gtk_tree_view_get_model(treeview);
    GtkListStore *store;
    
    // If model doesn't exist, create it with columns
    if (model == NULL) {
        store = gtk_list_store_new(6, 
            G_TYPE_STRING,  // ID
            G_TYPE_STRING,  // Nom
            G_TYPE_STRING,  // Type
            G_TYPE_INT,     // Capacité
            G_TYPE_STRING,  // Statut
            G_TYPE_STRING   // Ville
        );
        gtk_tree_view_set_model(treeview, GTK_TREE_MODEL(store));
        g_object_unref(store);
        
        // Add columns if they don't exist
        GList *columns = gtk_tree_view_get_columns(treeview);
        if (g_list_length(columns) == 0) {
            GtkCellRenderer *renderer;
            GtkTreeViewColumn *column;
            
            const char *titles[] = {"ID", "Nom", "Type", "Capacité", "Statut", "Ville"};
            for (int i = 0; i < 6; i++) {
                renderer = gtk_cell_renderer_text_new();
                column = gtk_tree_view_column_new_with_attributes(titles[i], renderer, "text", i, NULL);
                gtk_tree_view_column_set_resizable(column, TRUE);
                gtk_tree_view_append_column(treeview, column);
            }
        }
        g_list_free(columns);
        
        model = gtk_tree_view_get_model(treeview);
    }
    
    store = GTK_LIST_STORE(model);
    gtk_list_store_clear(store);
    
    // Populate with data
    GtkTreeIter iter;
    for (int i = 0; i < nb_centres; i++) {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
            0, centres[i].id,
            1, centres[i].nom,
            2, centres[i].type,
            3, centres[i].capacite,
            4, centres[i].ouvert ? "Ouvert" : "Fermé",
            5, centres[i].ville,
            -1);
    }
}

// Function to populate combobox with unique centre types
void populate_combobox_types(GtkWidget *combobox, Centre centres[], int nb_centres) {
    if (!combobox) return;
    
    GtkTreeModel *model = gtk_combo_box_get_model(GTK_COMBO_BOX(combobox));
    if (model != NULL) {
        gtk_list_store_clear(GTK_LIST_STORE(model));
    } else {
        GtkListStore *store = gtk_list_store_new(1, G_TYPE_STRING);
        gtk_combo_box_set_model(GTK_COMBO_BOX(combobox), GTK_TREE_MODEL(store));
        g_object_unref(store);
        
        GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
        gtk_cell_layout_clear(GTK_CELL_LAYOUT(combobox));
        gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combobox), renderer, TRUE);
        gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combobox), renderer, "text", 0, NULL);
    }
    
    model = gtk_combo_box_get_model(GTK_COMBO_BOX(combobox));
    GtkListStore *store = GTK_LIST_STORE(model);
    GtkTreeIter iter;
    
    // Add "All types" option
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Tous les types", -1);
    
    // Track unique types
    char types[MAX_CENTRES][TYPE_LEN];
    int nb_types = 0;
    
    for (int i = 0; i < nb_centres; i++) {
        if (centres[i].type[0] == '\0') continue;
        
        int exists = 0;
        for (int j = 0; j < nb_types; j++) {
            if (strcmp(types[j], centres[i].type) == 0) {
                exists = 1;
                break;
            }
        }
        
        if (!exists && nb_types < MAX_CENTRES) {
            strcpy(types[nb_types], centres[i].type);
            nb_types++;
            
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter, 0, centres[i].type, -1);
        }
    }
    
    gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), 0);
}

