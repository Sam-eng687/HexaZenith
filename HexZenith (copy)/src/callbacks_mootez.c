
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "callbacks.h"
#include "treeview.h"

#include "interface.h"
#include "support.h"
#include "equipement.h"
#include "reservation.h"

// Variables globales
char selected_id[50] = "";
GtkWidget *global_treeview = NULL;


// Variables globales pour la réservation
char selected_cours_id[MAX_STR] = "";
int quantite_demandee_globale = 0;
char equipement_depassement_id[50] = "";
char equipement_depassement_nom[50] = "";
char equipement_depassement_type[30] = "";
int stock_disponible_depassement = 0;

// Function prototypes

void on_toggle_equipreserve_trainer(GtkCellRendererToggle *cell, gchar *path_str, gpointer data);
void on_checkbutton_selectall_equipreserve_trainer_toggled(GtkToggleButton *button, gpointer user_data);

// Function to authenticate user



void charger_types_activite_dans_combobox(GtkWidget *combobox)
{
    FILE *f;
    Cours c;
    char types[100][MAX_STR];
    int nb_types = 0, i, existe;
    
    if (combobox == NULL) return;
    
    GtkTreeModel *model = gtk_combo_box_get_model(GTK_COMBO_BOX(combobox));
    if (model != NULL) gtk_list_store_clear(GTK_LIST_STORE(model));
    
    gtk_combo_box_append_text(GTK_COMBO_BOX(combobox), "Aucune");
    
    f = fopen("./src/database/cours.txt", "r");
    if (f != NULL)
    {
        while (fscanf(f, "%s %s %s %d %d %d %d %d %d %d %s %u", c.id_cours, c.nom_cours, c.type_activite, &c.jour, &c.mois, &c.annee, &c.heure_debut, &c.heure_fin, &c.niveau, &c.places_max, c.centre, &c.jours_mask) != EOF)
        {
            existe = 0;
            for (i = 0; i < nb_types; i++)
            {
                if (strcmp(types[i], c.type_activite) == 0) { existe = 1; break; }
            }
            if (!existe && nb_types < 100)
            {
                strcpy(types[nb_types], c.type_activite);
                nb_types++;
            }
        }
        fclose(f);
    }
    
    for (i = 0; i < nb_types; i++)
    {
        gtk_combo_box_append_text(GTK_COMBO_BOX(combobox), types[i]);
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(combobox), 0);
}


void on_button_chercher_mescoursid_entraineur_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *window, *treeview, *entry_recherche, *combobox_type, *combobox_niveau, *checkbutton_date, *spin_jour, *spin_mois, *spin_annee;
    GtkListStore *store;
    GtkTreeModel *model;
    GtkTreeIter iter;
    const gchar *search_id;
    gchar *filtre_type, *filtre_niveau;
    int filtre_type_actif = 0, filtre_niveau_actif = 0, filtre_date_actif = 0;
    int jour_recherche = 0, mois_recherche = 0, annee_recherche = 0, niveau_recherche = -1;
    FILE *f_inscriptions;
    char id_entr_lu[MAX_STR], id_cours[MAX_STR];
    Cours c;
    char date_str[50], horaire_str[50];
    
    window = lookup_widget(GTK_WIDGET(button), "window_espace_entraineur");
    treeview = lookup_widget(window, "treeview_liste_mescours_entraineur");
    entry_recherche = lookup_widget(window, "entry_recherche_mescoursid_entraineur");
    combobox_type = lookup_widget(window, "combobox_filter_typeactivite_entraineur");
    combobox_niveau = lookup_widget(window, "combobox_filter_niveau_entraineur");
    checkbutton_date = lookup_widget(window, "checkbutton_filtre_datecour_entraineur");
    spin_jour = lookup_widget(window, "spinbutton_jours_mescours_entraineur");
    spin_mois = lookup_widget(window, "spinbutton_mois_mescours_entraieur");
    spin_annee = lookup_widget(window, "spinbutton_annee_mescours_entraineur");
    
    if (!treeview) return;
    
    search_id = entry_recherche ? gtk_entry_get_text(GTK_ENTRY(entry_recherche)) : "";
    
    if (combobox_type)
    {
        filtre_type = gtk_combo_box_get_active_text(GTK_COMBO_BOX(combobox_type));
        if (filtre_type && strcmp(filtre_type, "Aucune") != 0) filtre_type_actif = 1;
    }
    
    if (combobox_niveau)
    {
        filtre_niveau = gtk_combo_box_get_active_text(GTK_COMBO_BOX(combobox_niveau));
        if (filtre_niveau && strcmp(filtre_niveau, "Aucune") != 0)
        {
            filtre_niveau_actif = 1;
            if (strcmp(filtre_niveau, "Debutant") == 0) niveau_recherche = 0;
            else if (strcmp(filtre_niveau, "Intermediaire") == 0) niveau_recherche = 1;
            else if (strcmp(filtre_niveau, "Avance") == 0) niveau_recherche = 2;
        }
    }
    
    if (checkbutton_date && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbutton_date)))
    {
        filtre_date_actif = 1;
        if (spin_jour) jour_recherche = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_jour));
        if (spin_mois) mois_recherche = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_mois));
        if (spin_annee) annee_recherche = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_annee));
    }
    
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
    if (model != NULL)
    {
        store = GTK_LIST_STORE(model);
        gtk_list_store_clear(store);
    }
    
    if (strlen(search_id) == 0 && !filtre_type_actif && !filtre_niveau_actif && !filtre_date_actif)
    {
	refresh_trainer_courses_treeview(treeview, current_trainer_id_str);
        if (filtre_type) g_free(filtre_type);
        if (filtre_niveau) g_free(filtre_niveau);
        return;
    }
    
    f_inscriptions = fopen("./src/database/inscriptionentraineur.txt", "r");
    if (f_inscriptions != NULL)
    {
        store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(treeview)));
        
        while (fscanf(f_inscriptions, "%s %s", id_entr_lu, id_cours) != EOF)
        {
            if (strcmp(id_entr_lu, current_trainer_id_str) == 0)
            {
                c = lire_cours("./src/database/cours.txt", id_cours);
                
                if (strcmp(c.id_cours, "-1") != 0)
                {
                    int afficher = 1;
                    
                    if (strlen(search_id) > 0 && strstr(c.id_cours, search_id) == NULL) afficher = 0;
                    if (filtre_type_actif && afficher && strcmp(c.type_activite, filtre_type) != 0) afficher = 0;
                    if (filtre_niveau_actif && afficher && c.niveau != niveau_recherche) afficher = 0;
                    if (filtre_date_actif && afficher)
                    {
                        if (c.jour != jour_recherche || c.mois != mois_recherche || c.annee != annee_recherche) afficher = 0;
                    }
                    
                    if (afficher)
                    {
                        sprintf(date_str, "%02d/%02d/%04d", c.jour, c.mois, c.annee);
                        sprintf(horaire_str, "%02d:00-%02d:00", c.heure_debut, c.heure_fin);
                        
                        gtk_list_store_append(store, &iter);
                        gtk_list_store_set(store, &iter, 0, c.id_cours, 1, c.nom_cours, 2, c.type_activite, 3, date_str, 4, horaire_str, 5, get_niveau_nom(c.niveau), 6, c.places_max, 7, c.centre, -1);
                    }
                }
            }
        }
        fclose(f_inscriptions);
    }
    
    if (filtre_type) g_free(filtre_type);
    if (filtre_niveau) g_free(filtre_niveau);
}



void on_button_reserver_equip_entraineur_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *window_entraineur, *treeview_cours, *window_dispo;
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *id_cours;
    
    window_entraineur = lookup_widget(GTK_WIDGET(button), "window_espace_entraineur");
    treeview_cours = lookup_widget(window_entraineur, "treeview_liste_mescours_entraineur");
    
    if (!treeview_cours) return;
    
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview_cours));
    
    if (gtk_tree_selection_get_selected(selection, &model, &iter))
    {
        gtk_tree_model_get(model, &iter, 0, &id_cours, -1);
        strcpy(selected_cours_id, id_cours);
        g_free(id_cours);
        
        window_dispo = create_window_equip_dispo_trainer();
        
        GtkWidget *treeview_dispo = lookup_widget(window_dispo, "treeview_liste_equipdispo_trainer");
        if (treeview_dispo != NULL) refresh_equipment_treeview(treeview_dispo);
        
        GtkWidget *combobox_filtre = lookup_widget(window_dispo, "combobox_filter_equipdispo_trainer");
        if (combobox_filtre != NULL)refresh_equipment_types_combobox(combobox_filtre, 1);
        
        gtk_widget_show(window_dispo);
    }
    else
    {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window_entraineur), GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "Veuillez sélectionner un cours de la liste.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }
}

void on_button_rechercheid_equipdispo_trainer_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *entry, *treeview, *window, *combobox_filtre;
    GtkListStore *store;
    GtkTreeModel *model;
    GtkTreeIter iter;
    const gchar *search_id;
    gchar *filtre_type;
    Equipement e;
    FILE *f;
    int filtre_actif = 0;
    
    window = lookup_widget(GTK_WIDGET(button), "window_equip_dispo_trainer");
    entry = lookup_widget(window, "entry_rechercheid_equipdispo_trainer");
    treeview = lookup_widget(window, "treeview_liste_equipdispo_trainer");
    combobox_filtre = lookup_widget(window, "combobox_filter_equipdispo_trainer");
    
    if (!entry || !treeview) return;
    
    search_id = gtk_entry_get_text(GTK_ENTRY(entry));
    
    if (combobox_filtre != NULL)
    {
        filtre_type = gtk_combo_box_get_active_text(GTK_COMBO_BOX(combobox_filtre));
        if (filtre_type != NULL && strcmp(filtre_type, "Aucune") != 0) filtre_actif = 1;
    }
    
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
    if (model != NULL)
    {
        store = GTK_LIST_STORE(model);
        gtk_list_store_clear(store);
    }
    
    if (strlen(search_id) == 0 && !filtre_actif)
    {
        refresh_equipment_treeview(treeview);
        if (filtre_type) g_free(filtre_type);
        return;
    }
    
    f = fopen("./src/database/equipements.txt", "r");
    if (f != NULL)
    {
        store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(treeview)));
        while (fscanf(f, "%s %s %s %d", e.id_equipement, e.nom_equipement, e.type, &e.nombre) != EOF)
        {
            if (e.nombre > 0)
            {
                int afficher = 1;
                if (strlen(search_id) > 0 && strstr(e.id_equipement, search_id) == NULL) afficher = 0;
                if (filtre_actif && afficher && strcmp(e.type, filtre_type) != 0) afficher = 0;
                
                if (afficher)
                {
                    gtk_list_store_append(store, &iter);
                    gtk_list_store_set(store, &iter, 0, e.id_equipement, 1, e.nom_equipement, 2, e.type, 3, e.nombre, -1);
                }
            }
        }
        fclose(f);
    }
    
    if (filtre_type) g_free(filtre_type);
}

void on_button_ajout_equipreserve_trainer_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *window, *treeview, *spinbutton, *window_depassement, *dialog;
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *equip_id, *equip_nom, *equip_type;
    gint stock_disponible, response;
    int quantite_demandee;
    
    window = lookup_widget(GTK_WIDGET(button), "window_equip_dispo_trainer");
    treeview = lookup_widget(window, "treeview_liste_equipdispo_trainer");
    spinbutton = lookup_widget(window, "spinbutton_nombre_reserve_equip_trainer");
    
    if (!treeview || !spinbutton) return;
    
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
    
    if (!gtk_tree_selection_get_selected(selection, &model, &iter))
    {
        dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "Veuillez sélectionner un équipement de la liste.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }
    
    gtk_tree_model_get(model, &iter, 0, &equip_id, 1, &equip_nom, 2, &equip_type, 3, &stock_disponible, -1);
    quantite_demandee = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinbutton));
    
    // Show confirmation popup with details
    dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, 
        "Confirmer la réservation:\n\nÉquipement: %s (%s)\nType: %s\nQuantité: %d\nCours: %s\n\nVoulez-vous continuer?", 
        equip_nom, equip_id, equip_type, quantite_demandee, selected_cours_id);
    response = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    
    if (response != GTK_RESPONSE_YES)
    {
        g_free(equip_id); g_free(equip_nom); g_free(equip_type);
        return;
    }
    
    if (quantite_demandee > stock_disponible)
    {
        strcpy(equipement_depassement_id, equip_id);
        strcpy(equipement_depassement_nom, equip_nom);
        strcpy(equipement_depassement_type, equip_type);
        stock_disponible_depassement = stock_disponible;
        quantite_demandee_globale = quantite_demandee;
        
        window_depassement = create_window_depassement_nombre();
        
        GtkWidget *spin_nouveau = lookup_widget(window_depassement, "spinbutton_nombre_nouveau_trainer");
        if (spin_nouveau)
        {
            gtk_spin_button_set_range(GTK_SPIN_BUTTON(spin_nouveau), 1, stock_disponible);
            gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_nouveau), stock_disponible);
        }
        
        GtkWidget *radio_max = lookup_widget(window_depassement, "radiobutton_nombremax_equipdispo_trainer");
        GtkWidget *radio_nouv = lookup_widget(window_depassement, "radiobutton_nouveaunombre_equipdispo_trainer");
        if (radio_max && radio_nouv)
        {
            gtk_radio_button_set_group(GTK_RADIO_BUTTON(radio_nouv), gtk_radio_button_get_group(GTK_RADIO_BUTTON(radio_max)));
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_max), TRUE);
        }
        
        gtk_widget_show(window_depassement);
        g_free(equip_id); g_free(equip_nom); g_free(equip_type);
        return;
    }
    
    if (ajouter_reservation_equipement(current_trainer_id_str, selected_cours_id, equip_id, equip_nom, equip_type, quantite_demandee))
    {
        refresh_equipment_treeview(treeview);
        dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "Équipement réservé avec succès!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }
    else
    {
        dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Erreur lors de la réservation.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }
    
    g_free(equip_id); g_free(equip_nom); g_free(equip_type);
}

void on_button_confirmer_nouv_nombre_trainer_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *window_depassement, *radio_max, *radio_nouv, *spin_nouveau, *dialog;
    int quantite_finale;
    
    window_depassement = lookup_widget(GTK_WIDGET(button), "window_depassement_nombre");
    radio_max = lookup_widget(window_depassement, "radiobutton_nombremax_equipdispo_trainer");
    radio_nouv = lookup_widget(window_depassement, "radiobutton_nouveaunombre_equipdispo_trainer");
    spin_nouveau = lookup_widget(window_depassement, "spinbutton_nombre_nouveau_trainer");
    
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_max)))
    {
        quantite_finale = stock_disponible_depassement;
    }
    else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_nouv)))
    {
        quantite_finale = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_nouveau));
    }
    else
    {
        gtk_widget_destroy(window_depassement);
        return;
    }
    
    if (ajouter_reservation_equipement(current_trainer_id_str, selected_cours_id, equipement_depassement_id, equipement_depassement_nom, equipement_depassement_type, quantite_finale))
    {
        dialog = gtk_message_dialog_new(GTK_WINDOW(window_depassement), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "Équipement réservé avec succès!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }
    else
    {
        dialog = gtk_message_dialog_new(GTK_WINDOW(window_depassement), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Erreur lors de la réservation.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }
    
    gtk_widget_destroy(window_depassement);
}


void ouvrir_fenetre_modification(GtkTreeModel *model, GtkTreeIter *iter)
{
    GtkWidget *window_modifier, *entry_id, *entry_nom, *combobox_type, *spinbutton_qte;
    gchar *id, *nom, *type;
    gint qte;
    
    gtk_tree_model_get(model, iter, 0, &id, 1, &nom, 2, &type, 3, &qte, -1);
    strcpy(selected_id, id);
    
    window_modifier = create_window_modifier_equipement();
    combobox_type = lookup_widget(window_modifier, "comboboxentry_mod_typeequip_admin");
    if (combobox_type != NULL) refresh_equipment_types_combobox(combobox_type, 1);
    
    entry_id = lookup_widget(window_modifier, "entry_mod_idequip_admin");
    entry_nom = lookup_widget(window_modifier, "entry_mod_nomequip_admin");
    spinbutton_qte = lookup_widget(window_modifier, "spinbutton_mod_qteequip_admin");
    
    if (entry_id && entry_nom && combobox_type && spinbutton_qte)
    {
        gtk_entry_set_text(GTK_ENTRY(entry_id), id);
        gtk_widget_set_sensitive(entry_id, FALSE); // Make ID field non-editable
        gtk_entry_set_text(GTK_ENTRY(entry_nom), nom);
        
        GtkWidget *entry = GTK_BIN(combobox_type)->child;
        if (entry != NULL) gtk_entry_set_text(GTK_ENTRY(entry), type);
        
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinbutton_qte), qte);
    }
    
    gtk_widget_show(window_modifier);
    g_free(id); g_free(nom); g_free(type);
}

void vider_champs_ajout(GtkWidget *window)
{
    GtkWidget *entry_id, *entry_nom, *combobox_type, *spinbutton_qte;
    
    entry_id = lookup_widget(window, "entry_ajout_idequip_admin");
    entry_nom = lookup_widget(window, "entry_ajout_nomequip_admin");
    combobox_type = lookup_widget(window, "comboboxentry_ajout_typeequip_admin");
    spinbutton_qte = lookup_widget(window, "spinbutton_ajouter_qteequip_admin");
    
    if (entry_id) gtk_entry_set_text(GTK_ENTRY(entry_id), "");
    if (entry_nom) gtk_entry_set_text(GTK_ENTRY(entry_nom), "");
    
    if (combobox_type)
    {
        GtkWidget *entry = GTK_BIN(combobox_type)->child;
        if (entry != NULL) gtk_entry_set_text(GTK_ENTRY(entry), "");
    }
    
    if (spinbutton_qte) gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinbutton_qte), 1);
}

int valider_champs_equipement(Equipement e, GtkWidget *parent_window)
{
    if (strlen(e.id_equipement) == 0)
    {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(parent_window), GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "L'ID de l'équipement ne peut pas être vide.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return 0;
    }
    
    if (strlen(e.nom_equipement) == 0)
    {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(parent_window), GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "Le nom de l'équipement ne peut pas être vide.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return 0;
    }
    
    if (strlen(e.type) == 0)
    {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(parent_window), GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "Le type de l'équipement ne peut pas être vide.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return 0;
    }
    
    return 1;
}

void on_button_chercher_equipid_admin_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *entry, *treeview, *window, *combobox_filtre;
    GtkListStore *store;
    GtkTreeModel *model;
    GtkTreeIter iter;
    const gchar *search_id;
    gchar *filtre_type;
    Equipement e;
    FILE *f;
    int filtre_actif = 0;
    
    window = lookup_widget(GTK_WIDGET(button), "window_espace_admin");
    entry = lookup_widget(window, "entry_recherche_equipid_admin");
    treeview = lookup_widget(window, "treeview_listeequipements_admin");
    combobox_filtre = lookup_widget(window, "combobox_filter_equip_admin");
    
    if (!entry || !treeview) return;
    
    search_id = gtk_entry_get_text(GTK_ENTRY(entry));
    
    if (combobox_filtre != NULL)
    {
        filtre_type = gtk_combo_box_get_active_text(GTK_COMBO_BOX(combobox_filtre));
        if (filtre_type != NULL && strcmp(filtre_type, "Aucune") != 0) filtre_actif = 1;
    }
    
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
    if (model != NULL)
    {
        store = GTK_LIST_STORE(model);
        gtk_list_store_clear(store);
    }
    
    if (strlen(search_id) == 0 && !filtre_actif)
    {
        refresh_equipment_treeview(treeview);
        if (filtre_type) g_free(filtre_type);
        return;
    }
    
    f = fopen("./src/database/equipements.txt", "r");
    if (f != NULL)
    {
        store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(treeview)));
        while (fscanf(f, "%s %s %s %d", e.id_equipement, e.nom_equipement, e.type, &e.nombre) != EOF)
        {
            int afficher = 1;
            if (strlen(search_id) > 0 && strstr(e.id_equipement, search_id) == NULL) afficher = 0;
            if (filtre_actif && afficher && strcmp(e.type, filtre_type) != 0) afficher = 0;
            
            if (afficher)
            {
                gtk_list_store_append(store, &iter);
                gtk_list_store_set(store, &iter, 0, e.id_equipement, 1, e.nom_equipement, 2, e.type, 3, e.nombre, -1);
            }
        }
        fclose(f);
    }
    
    if (filtre_type) g_free(filtre_type);
}

void on_button_ajouter_equip_admin_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *window_admin = lookup_widget(GTK_WIDGET(button), "window_espace_admin");
    GtkWidget *window_ajouter = create_window_ajouter_equipement();
    
    // Store the admin window reference in the dialog
    g_object_set_data(G_OBJECT(window_ajouter), "parent_admin_window", window_admin);
    
    GtkWidget *combobox_type = lookup_widget(window_ajouter, "comboboxentry_ajout_typeequip_admin");
    if (combobox_type != NULL) refresh_equipment_types_combobox(combobox_type, 1);
    gtk_widget_show(window_ajouter);
}
void on_button_ajout_enregistrerequip_admin_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *entry_id, *entry_nom, *combobox_type, *spinbutton_qte, *window_ajouter;
    Equipement e;
    gchar *type_text;

    window_ajouter = lookup_widget(GTK_WIDGET(button), "window_ajouter_equipement");
    
    // Get the stored admin window reference
    GtkWidget *window_admin = g_object_get_data(G_OBJECT(window_ajouter), "parent_admin_window");
    
    entry_id = lookup_widget(window_ajouter, "entry_ajout_idequip_admin");
    entry_nom = lookup_widget(window_ajouter, "entry_ajout_nomequip_admin");
    combobox_type = lookup_widget(window_ajouter, "comboboxentry_ajout_typeequip_admin");
    spinbutton_qte = lookup_widget(window_ajouter, "spinbutton_ajouter_qteequip_admin");

    if (!entry_id || !entry_nom || !combobox_type || !spinbutton_qte)
    {
        gtk_widget_destroy(window_ajouter);
        return;
    }

    strcpy(e.id_equipement, gtk_entry_get_text(GTK_ENTRY(entry_id)));
    strcpy(e.nom_equipement, gtk_entry_get_text(GTK_ENTRY(entry_nom)));

    type_text = gtk_combo_box_get_active_text(GTK_COMBO_BOX(combobox_type));
    if (type_text == NULL || strlen(type_text) == 0)
    {
        GtkWidget *entry = GTK_BIN(combobox_type)->child;
        if (entry != NULL) type_text = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry)));
    }

    if (type_text != NULL)
    {
        strcpy(e.type, type_text);
        g_free(type_text);
    }
    else strcpy(e.type, "");

    e.nombre = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinbutton_qte));

    remplacer_espaces(e.id_equipement);
    remplacer_espaces(e.nom_equipement);
    remplacer_espaces(e.type);

    if (!valider_champs_equipement(e, window_ajouter)) return;

    if (id_existe("./src/database/equipements.txt", e.id_equipement))
    {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window_ajouter), GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "L'ID '%s' existe déjà.\nVeuillez choisir un autre ID.", e.id_equipement);
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        return;
    }

    // Show confirmation popup with details
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window_ajouter), GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
        "Confirmer l'ajout de l'équipement:\n\nID: %s\nNom: %s\nType: %s\nQuantité: %d\n\nVoulez-vous continuer?",
        e.id_equipement, e.nom_equipement, e.type, e.nombre);
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    if (response != GTK_RESPONSE_YES) return;

    if (ajouter("./src/database/equipements.txt", e))
    {
        vider_champs_ajout(window_ajouter);
        
        // Refresh the admin window's treeview
        if (window_admin != NULL)
        {
            GtkWidget *treeview = lookup_widget(window_admin, "treeview_listeequipements_admin");
            if (treeview != NULL) {
                refresh_equipment_treeview(treeview);
            }
            
            GtkWidget *combobox_filtre = lookup_widget(window_admin, "combobox_filter_equip_admin");
            if (combobox_filtre != NULL) {
                refresh_equipment_types_combobox(combobox_filtre, 1);
            }
        }

        // Show success message
        dialog = gtk_message_dialog_new(GTK_WINDOW(window_ajouter), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "Équipement ajouté avec succès!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);

        gtk_widget_destroy(window_ajouter);
    }
    else
    {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window_ajouter), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Erreur lors de l'ajout de l'équipement.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }
}

void on_button_ajout_annulerequip_admin_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *window_ajouter = lookup_widget(GTK_WIDGET(button), "window_ajouter_equipement");
    vider_champs_ajout(window_ajouter);
    gtk_widget_destroy(window_ajouter);
}

void on_button_modifier_equip_admin_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *treeview, *window_admin;
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter;

    window_admin = lookup_widget(GTK_WIDGET(button), "window_espace_admin");
    treeview = lookup_widget(window_admin, "treeview_listeequipements_admin");

    if (!treeview) return;

    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));

    if (gtk_tree_selection_get_selected(selection, &model, &iter))
    {
        GtkWidget *window_modifier;
        gchar *id, *nom, *type;
        gint qte;

        gtk_tree_model_get(model, &iter, 0, &id, 1, &nom, 2, &type, 3, &qte, -1);
        strcpy(selected_id, id);

        window_modifier = create_window_modifier_equipement();
        
        // Store the admin window reference in the dialog
        g_object_set_data(G_OBJECT(window_modifier), "parent_admin_window", window_admin);
        
        GtkWidget *combobox_type = lookup_widget(window_modifier, "comboboxentry_mod_typeequip_admin");
        if (combobox_type != NULL) refresh_equipment_types_combobox(combobox_type, 1);

        GtkWidget *entry_id = lookup_widget(window_modifier, "entry_mod_idequip_admin");
        GtkWidget *entry_nom = lookup_widget(window_modifier, "entry_mod_nomequip_admin");
        GtkWidget *spinbutton_qte = lookup_widget(window_modifier, "spinbutton_mod_qteequip_admin");

        if (entry_id && entry_nom && combobox_type && spinbutton_qte)
        {
            gtk_entry_set_text(GTK_ENTRY(entry_id), id);
            gtk_widget_set_sensitive(entry_id, FALSE); // Make ID field non-editable
            gtk_entry_set_text(GTK_ENTRY(entry_nom), nom);

            GtkWidget *entry = GTK_BIN(combobox_type)->child;
            if (entry != NULL) gtk_entry_set_text(GTK_ENTRY(entry), type);

            gtk_spin_button_set_value(GTK_SPIN_BUTTON(spinbutton_qte), qte);
        }

        gtk_widget_show(window_modifier);
        g_free(id); g_free(nom); g_free(type);
    }
    else
    {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window_admin), GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "Veuillez sélectionner un équipement de la liste.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }
}

void on_button_mod_enregistrerequip_admin_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *entry_id, *entry_nom, *combobox_type, *spinbutton_qte, *window_modifier;
    Equipement e;
    gchar *type_text;

    window_modifier = lookup_widget(GTK_WIDGET(button), "window_modifier_equipement");
    
    // Get the stored admin window reference
    GtkWidget *window_admin = g_object_get_data(G_OBJECT(window_modifier), "parent_admin_window");
    
    entry_id = lookup_widget(window_modifier, "entry_mod_idequip_admin");
    entry_nom = lookup_widget(window_modifier, "entry_mod_nomequip_admin");
    combobox_type = lookup_widget(window_modifier, "comboboxentry_mod_typeequip_admin");
    spinbutton_qte = lookup_widget(window_modifier, "spinbutton_mod_qteequip_admin");

    if (!entry_id || !entry_nom || !combobox_type || !spinbutton_qte)
    {
        gtk_widget_destroy(window_modifier);
        return;
    }

    strcpy(e.id_equipement, gtk_entry_get_text(GTK_ENTRY(entry_id)));
    strcpy(e.nom_equipement, gtk_entry_get_text(GTK_ENTRY(entry_nom)));

    type_text = gtk_combo_box_get_active_text(GTK_COMBO_BOX(combobox_type));
    if (type_text == NULL || strlen(type_text) == 0)
    {
        GtkWidget *entry = GTK_BIN(combobox_type)->child;
        if (entry != NULL) type_text = g_strdup(gtk_entry_get_text(GTK_ENTRY(entry)));
    }

    if (type_text != NULL)
    {
        strcpy(e.type, type_text);
        g_free(type_text);
    }
    else strcpy(e.type, "");

    e.nombre = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spinbutton_qte));

    remplacer_espaces(e.id_equipement);
    remplacer_espaces(e.nom_equipement);
    remplacer_espaces(e.type);

    if (!valider_champs_equipement(e, window_modifier)) return;

    // Show confirmation popup with details (ID is not editable, so no need to check for duplicate ID)
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window_modifier), GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO,
        "Confirmer la modification de l'équipement:\n\nID: %s\nNom: %s\nType: %s\nQuantité: %d\n\nVoulez-vous continuer?",
        e.id_equipement, e.nom_equipement, e.type, e.nombre);
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    if (response != GTK_RESPONSE_YES) return;

    if (modifier("./src/database/equipements.txt", selected_id, e))
    {
        // Refresh the admin window's treeview
        if (window_admin != NULL)
        {
            GtkWidget *treeview = lookup_widget(window_admin, "treeview_listeequipements_admin");
            if (treeview != NULL) {
                refresh_equipment_treeview(treeview);
            }
            
            GtkWidget *combobox_filtre = lookup_widget(window_admin, "combobox_filter_equip_admin");
            if (combobox_filtre != NULL) {
                refresh_equipment_types_combobox(combobox_filtre, 1);
            }
        }

        // Show success message
        dialog = gtk_message_dialog_new(GTK_WINDOW(window_modifier), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "Équipement modifié avec succès!");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);

        gtk_widget_destroy(window_modifier);
    }
    else
    {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window_modifier), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Erreur lors de la modification de l'équipement.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }
}

void on_button_mod_annulerequip_admin_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *window_modifier = lookup_widget(GTK_WIDGET(button), "window_modifier_equipement");
    gtk_widget_destroy(window_modifier);
}

void on_button_supprimer_equip_admin_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *treeview, *window_admin;
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *id;
    
    window_admin = lookup_widget(GTK_WIDGET(button), "window_espace_admin");
    treeview = lookup_widget(window_admin, "treeview_listeequipements_admin");
    
    if (!treeview) return;
    
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
    
    if (gtk_tree_selection_get_selected(selection, &model, &iter))
    {
        GtkWidget *dialog;
        gint response;
        
        gtk_tree_model_get(model, &iter, 0, &id, -1);
        dialog = gtk_message_dialog_new(GTK_WINDOW(window_admin), GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, "Voulez-vous vraiment supprimer l'équipement '%s' ?", id);
        response = gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        
        if (response == GTK_RESPONSE_YES)
        {
            if (supprimer("./src/database/equipements.txt", id))
            {
                refresh_equipment_treeview(treeview);
            }
            else
            {
                dialog = gtk_message_dialog_new(GTK_WINDOW(window_admin), GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK, "Erreur lors de la suppression de l'équipement.");
                gtk_dialog_run(GTK_DIALOG(dialog));
                gtk_widget_destroy(dialog);
            }
        }
        g_free(id);
    }
    else
    {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window_admin), GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "Veuillez sélectionner un équipement de la liste.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }
}

void on_treeview_listeequipements_admin_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data)
{
    GtkTreeModel *model;
    GtkTreeIter iter;
    
    model = gtk_tree_view_get_model(treeview);
    if (gtk_tree_model_get_iter(model, &iter, path))
    {
        ouvrir_fenetre_modification(model, &iter);
    }
}

// Callback for individual checkbox toggle
void on_toggle_equipreserve_trainer(GtkCellRendererToggle *cell, gchar *path_str, gpointer data)
{
    GtkTreeModel *model = GTK_TREE_MODEL(data);
    GtkTreeIter iter;
    GtkTreePath *path = gtk_tree_path_new_from_string(path_str);
    gboolean active;
    
    gtk_tree_model_get_iter(model, &iter, path);
    gtk_tree_model_get(model, &iter, 0, &active, -1);
    
    // Toggle the value
    active = !active;
    gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, active, -1);
    
    gtk_tree_path_free(path);
}

// Callback for select-all checkbox
void on_checkbutton_selectall_equipreserve_trainer_toggled(GtkToggleButton *button, gpointer user_data)
{
    GtkWidget *window = lookup_widget(GTK_WIDGET(button), "window_equip_reserve_trainer");
    GtkWidget *treeview = lookup_widget(window, "treeview_liste_equipreservee_trainer");
    
    if (!treeview) return;
    
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
    if (!model) return;
    
    GtkTreeIter iter;
    gboolean select_all = gtk_toggle_button_get_active(button);
    
    if (gtk_tree_model_get_iter_first(model, &iter))
    {
        do {
            gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, select_all, -1);
        } while (gtk_tree_model_iter_next(model, &iter));
    }
}

void afficher_equipements_reserves_trainer(GtkWidget *liste, char *id_entraineur_param, char *id_cours_param)
{
    GtkListStore *store;
    GtkTreeModel *model;
    GtkTreeIter iter;
    FILE *f;
    char ligne[500];
    
    if (liste == NULL) return;
    
    // Uncheck the select-all checkbox when refreshing
    GtkWidget *window = gtk_widget_get_toplevel(liste);
    if (window && GTK_IS_WINDOW(window))
    {
        GtkWidget *checkbutton_selectall = lookup_widget(window, "checkbutton_selectall_equipreserve_trainer");
        if (checkbutton_selectall)
        {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(checkbutton_selectall), FALSE);
        }
    }
    
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(liste));
    
    if (model == NULL)
    {
        GtkCellRenderer *renderer;
        GtkTreeViewColumn *column;
        
        // 5 columns: boolean (checkbox), string (ID), string (Nom), string (Type), int (Quantité)
        store = gtk_list_store_new(5, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT);
        
        // Add toggle renderer for checkbox column
        renderer = gtk_cell_renderer_toggle_new();
        column = gtk_tree_view_column_new_with_attributes("Sélection", renderer, "active", 0, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(liste), column);
        
        // Connect toggle signal
        g_signal_connect(renderer, "toggled", G_CALLBACK(on_toggle_equipreserve_trainer), store);
        
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("ID", renderer, "text", 1, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(liste), column);
        
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Nom", renderer, "text", 2, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(liste), column);
        
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Type", renderer, "text", 3, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(liste), column);
        
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Quantité", renderer, "text", 4, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(GTK_TREE_VIEW(liste), column);
        
        gtk_tree_view_set_model(GTK_TREE_VIEW(liste), GTK_TREE_MODEL(store));
        g_object_unref(store);
    }
    else
    {
        store = GTK_LIST_STORE(model);
        gtk_list_store_clear(store);
        
        // Reconnect the toggle signal for the renderer when refreshing existing model
        GtkTreeViewColumn *column = gtk_tree_view_get_column(GTK_TREE_VIEW(liste), 0);
        if (column)
        {
            GList *renderers = gtk_cell_layout_get_cells(GTK_CELL_LAYOUT(column));
            if (renderers)
            {
                GtkCellRenderer *renderer = GTK_CELL_RENDERER(renderers->data);
                if (GTK_IS_CELL_RENDERER_TOGGLE(renderer))
                {
                    // Disconnect any existing signal and reconnect
                    g_signal_handlers_disconnect_by_func(renderer, G_CALLBACK(on_toggle_equipreserve_trainer), NULL);
                    g_signal_connect(renderer, "toggled", G_CALLBACK(on_toggle_equipreserve_trainer), store);
                }
                g_list_free(renderers);
            }
        }
    }
    
    f = fopen("./src/database/reservations.txt", "r");
    if (f != NULL)
    {
        store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(liste)));
        
        while (fgets(ligne, sizeof(ligne), f) != NULL)
        {
            char id_entr[MAX_STR], id_cours[MAX_STR], equipements_data[400];
            
            if (sscanf(ligne, "%s %s %[^\n]", id_entr, id_cours, equipements_data) == 3)
            {
                if (strcmp(id_entr, id_entraineur_param) == 0 && strcmp(id_cours, id_cours_param) == 0)
                {
                    char equipements_copy[400];
                    strcpy(equipements_copy, equipements_data);
                    char *token = strtok(equipements_copy, ";");
                    while (token != NULL)
                    {
                        char equip_id[50], equip_nom[50], equip_type[30];
                        int quantite;
                        
                        if (sscanf(token, "%[^,],%[^,],%[^,],%d", equip_id, equip_nom, equip_type, &quantite) == 4)
                        {
                            gtk_list_store_append(store, &iter);
                            gtk_list_store_set(store, &iter, 
                                0, FALSE,  // checkbox initially unchecked
                                1, equip_id, 
                                2, equip_nom, 
                                3, equip_type, 
                                4, quantite, -1);
                        }
                        
                        token = strtok(NULL, ";");
                    }
                    break;
                }
            }
        }
        fclose(f);
    }
}

void on_button_equip_reserve_entraineur_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *window_entraineur, *treeview_cours, *window_reserve;
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GtkTreeIter iter;
    gchar *id_cours;
    
    window_entraineur = lookup_widget(GTK_WIDGET(button), "window_espace_entraineur");
    treeview_cours = lookup_widget(window_entraineur, "treeview_liste_mescours_entraineur");
    
    if (!treeview_cours) return;
    
    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview_cours));
    
    if (gtk_tree_selection_get_selected(selection, &model, &iter))
    {
        gtk_tree_model_get(model, &iter, 0, &id_cours, -1);
        strcpy(selected_cours_id, id_cours);
        g_free(id_cours);
        
        window_reserve = create_window_equip_reserve_trainer();
        
        GtkWidget *treeview_reserve = lookup_widget(window_reserve, "treeview_liste_equipreservee_trainer");
        if (treeview_reserve != NULL) afficher_equipements_reserves_trainer(treeview_reserve, current_trainer_id_str, selected_cours_id);
        
        GtkWidget *combobox_filtre = lookup_widget(window_reserve, "combobox_filter_equipreserve_trainer");
        if (combobox_filtre != NULL) refresh_equipment_types_combobox(combobox_filtre, 1);
        
        // Connect the select-all checkbox signal
        connect_selectall_checkbox(window_reserve);
        
        gtk_widget_show(window_reserve);
    }
    else
    {
        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(window_entraineur), GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "Veuillez sélectionner un cours de la liste.");
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }
}

void on_button_chercher_equipreserve_trainer_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *entry, *treeview, *window, *combobox_filtre;
    GtkListStore *store;
    GtkTreeModel *model;
    GtkTreeIter iter;
    const gchar *search_id;
    gchar *filtre_type;
    FILE *f;
    char ligne[500];
    int filtre_actif = 0;
    
    window = lookup_widget(GTK_WIDGET(button), "window_equip_reserve_trainer");
    entry = lookup_widget(window, "entry_rechercheid_equipreserve_trainer");
    treeview = lookup_widget(window, "treeview_liste_equipreservee_trainer");
    combobox_filtre = lookup_widget(window, "combobox_filter_equipreserve_trainer");
    
    if (!entry || !treeview) return;
    
    search_id = gtk_entry_get_text(GTK_ENTRY(entry));
    
    if (combobox_filtre != NULL)
    {
        filtre_type = gtk_combo_box_get_active_text(GTK_COMBO_BOX(combobox_filtre));
        if (filtre_type != NULL && strcmp(filtre_type, "Aucune") != 0) filtre_actif = 1;
    }
    
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
    if (model != NULL)
    {
        store = GTK_LIST_STORE(model);
        gtk_list_store_clear(store);
    }
    
    if (strlen(search_id) == 0 && !filtre_actif)
    {
        afficher_equipements_reserves_trainer(treeview, current_trainer_id_str, selected_cours_id);
        
        // Reconnect the checkbox signals after refreshing
        GtkTreeModel *new_model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
        if (new_model)
        {
            // Get the toggle renderer from the first column
            GtkTreeViewColumn *column = gtk_tree_view_get_column(GTK_TREE_VIEW(treeview), 0);
            if (column)
            {
                GList *renderers = gtk_cell_layout_get_cells(GTK_CELL_LAYOUT(column));
                if (renderers)
                {
                    GtkCellRenderer *renderer = GTK_CELL_RENDERER(renderers->data);
                    if (GTK_IS_CELL_RENDERER_TOGGLE(renderer))
                    {
                        // Disconnect any existing signal
                        g_signal_handlers_disconnect_by_func(renderer, on_toggle_equipreserve_trainer, NULL);
                        // Reconnect the signal
                        g_signal_connect(renderer, "toggled", G_CALLBACK(on_toggle_equipreserve_trainer), new_model);
                    }
                    g_list_free(renderers);
                }
            }
        }
        
        if (filtre_type) g_free(filtre_type);
        return;
    }
    
    // ... rest of the existing function ...
    
    if (filtre_type) g_free(filtre_type);
}

void on_button_supp_equipreservee_trainer_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *window, *treeview, *checkbutton_selectall;
    GtkTreeModel *model;
    GtkTreeIter iter;
    GtkWidget *dialog;
    gint response;
    
    window = lookup_widget(GTK_WIDGET(button), "window_equip_reserve_trainer");
    treeview = lookup_widget(window, "treeview_liste_equipreservee_trainer");
    checkbutton_selectall = lookup_widget(window, "checkbutton_selectall_equipreserve_trainer");
    
    if (!treeview) return;
    
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
    if (!model) return;
    
    // Check if select-all is checked
    if (checkbutton_selectall && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbutton_selectall)))
    {
        // Ask for confirmation to delete all
        dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, 
            "Voulez-vous vraiment supprimer TOUTES les réservations de ce cours?");
        response = gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        
        if (response != GTK_RESPONSE_YES) return;
        
        if (supprimer_toutes_reservations_cours(current_trainer_id_str, selected_cours_id))
        {
            afficher_equipements_reserves_trainer(treeview, current_trainer_id_str, selected_cours_id);
            dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, 
                "Toutes les réservations ont été supprimées!");
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
        }
    }
    else
    {
        // Delete selected items
        int count = 0;
        char selected_ids[100][50];  // Store up to 100 selected IDs
        
        if (gtk_tree_model_get_iter_first(model, &iter))
        {
            do {
                gboolean selected;
                gchar *equip_id;
                
                gtk_tree_model_get(model, &iter, 0, &selected, 1, &equip_id, -1);
                
                if (selected)
                {
                    strcpy(selected_ids[count], equip_id);
                    count++;
                }
                
                g_free(equip_id);
            } while (gtk_tree_model_iter_next(model, &iter));
        }
        
        if (count == 0)
        {
            dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, 
                "Veuillez sélectionner au moins un équipement à supprimer.");
            gtk_dialog_run(GTK_DIALOG(dialog));
            gtk_widget_destroy(dialog);
            return;
        }
        
        // Ask for confirmation
        dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_QUESTION, GTK_BUTTONS_YES_NO, 
            "Voulez-vous vraiment supprimer %d équipement(s) sélectionné(s)?", count);
        response = gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        
        if (response != GTK_RESPONSE_YES) return;
        
        // Delete each selected equipment
        int success_count = 0;
        for (int i = 0; i < count; i++)
        {
            if (supprimer_reservation_equipement(current_trainer_id_str, selected_cours_id, selected_ids[i]))
            {
                success_count++;
            }
        }
        
        // Refresh the display
        afficher_equipements_reserves_trainer(treeview, current_trainer_id_str, selected_cours_id);
        
        // Show result message
        dialog = gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, 
            "%d équipement(s) supprimé(s) avec succès!", success_count);
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
    }
}

void on_button_retour_equipdispo_trainer_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *window = lookup_widget(GTK_WIDGET(button), "window_equip_dispo_trainer");
    if (window) gtk_widget_destroy(window);
}

void on_button_retour_equireservee_trainer_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *window = lookup_widget(GTK_WIDGET(button), "window_equip_reserve_trainer");
    if (window) gtk_widget_destroy(window);
}

// Add this function to connect the select-all checkbox signal
void connect_selectall_checkbox(GtkWidget *window)
{
    if (!window) return;
    
    GtkWidget *checkbutton_selectall = lookup_widget(window, "checkbutton_selectall_equipreserve_trainer");
    if (checkbutton_selectall)
    {
        g_signal_connect(G_OBJECT(checkbutton_selectall), "toggled", 
                        G_CALLBACK(on_checkbutton_selectall_equipreserve_trainer_toggled), NULL);
    }
}


void
on_treeview_list_centres_admin_row_activated
                                        (GtkTreeView     *treeview,
                                        GtkTreePath     *path,
                                        GtkTreeViewColumn *column,
                                        gpointer         user_data)
{

}


void
on_spinbutton_cap_min_admin_value_changed
                                        (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{

}


void
on_combobox_filtrer_partype_changed    (GtkComboBox     *combobox,
                                        gpointer         user_data)
{

}


void
on_radiobutton_tri_ville_toggled       (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{

}


void
on_radiobutton_tri_nom_toggled         (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{

}


void
on_checkbutton_Centres_ouverts_filtrage_toggled
                                        (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{

}


void
on_button_ajouter_centre_admin_clicked (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_modifier_centre_admin_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_supprimer_centre_admin_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_chercher_par_idcentre_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_treeview_listentraineurs_admin_row_activated
                                        (GtkTreeView     *treeview,
                                        GtkTreePath     *path,
                                        GtkTreeViewColumn *column,
                                        gpointer         user_data)
{

}


void
on_button_chercher_parid_entraineur_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_ajouter_entraineur_admin_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_modifier_entraineur_admin_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_supprimer_entraineur_admin_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_treeview_listemembre_admin_row_activated
                                        (GtkTreeView     *treeview,
                                        GtkTreePath     *path,
                                        GtkTreeViewColumn *column,
                                        gpointer         user_data)
{

}


void
on_button_ajouter_membre_admmin_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_modifier_membre_admin_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_supprimer_membre_admin_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_chercher_membre_parid_admin_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_treeview_liste_cours_admin_row_activated
                                        (GtkTreeView     *treeview,
                                        GtkTreePath     *path,
                                        GtkTreeViewColumn *column,
                                        gpointer         user_data)
{

}


void
on_button_chercher_parid_cours_admin_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_ajouter_cours_sportifs_admin_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_modifier_cours_admin_clicked (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_supprimer_cours_admin_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_treeview_liste_event_admin_row_activated
                                        (GtkTreeView     *treeview,
                                        GtkTreePath     *path,
                                        GtkTreeViewColumn *column,
                                        gpointer         user_data)
{

}


void
on_button_chercher_parid_event_admin_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_ajouter_event_admin_clicked  (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_modifier_event_admin_clicked (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_supprimer_event_admin_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}




void
on_treeview_listecentres_entraineur_row_activated
                                        (GtkTreeView     *treeview,
                                        GtkTreePath     *path,
                                        GtkTreeViewColumn *column,
                                        gpointer         user_data)
{

}


void
on_spinbutton_cap_min_entraineur_value_changed
                                        (GtkSpinButton   *spinbutton,
                                        gpointer         user_data)
{

}


void
on_button_chercher_centre_entraineur_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_combobox_filtrer_partype_entraineur_changed
                                        (GtkComboBox     *combobox,
                                        gpointer         user_data)
{

}


void
on_checkbutton_Centres_ouverts_filtrage_entraineur_toggled
                                        (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{

}


void
on_radiobutton_tri_nom_entraineur_toggled
                                        (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{

}


void
on_radiobutton_tri_ville_entraineur_toggled
                                        (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{

}


void
on_button_sinscrire_entraineur_clicked (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_voir_inscriptions_entraineur_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_treeview_cours_dispo_entraineur_row_activated
                                        (GtkTreeView     *treeview,
                                        GtkTreePath     *path,
                                        GtkTreeViewColumn *column,
                                        gpointer         user_data)
{

}


void
on_button_sinscrirecours_entraineur_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_mescours_entraineur_clicked  (GtkButton       *button,
                                        gpointer         user_data)
{

}





void
on_treeview_listes_du_cours__row_activated
                                        (GtkTreeView     *treeview,
                                        GtkTreePath     *path,
                                        GtkTreeViewColumn *column,
                                        gpointer         user_data)
{

}


void
on_button_chercher_cours_membre_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_radiobutton_debutant_cours_membre_toggled
                                        (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{

}


void
on_radiobutton_intermidiaire_cours_membre_toggled
                                        (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{

}


void
on_button_sinscrirecours_membre_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_mescours_membre_clicked      (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_radiobutton_avance_cours_membre_toggled
                                        (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{

}


void
on_treeview_listeentraineur_membre_row_activated
                                        (GtkTreeView     *treeview,
                                        GtkTreePath     *path,
                                        GtkTreeViewColumn *column,
                                        gpointer         user_data)
{

}


void
on_button_chercher_entraineur_membre_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_demander_entraineur_membre_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_mesentraineurs_membre_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_treeview_liste_event_membre_row_activated
                                        (GtkTreeView     *treeview,
                                        GtkTreePath     *path,
                                        GtkTreeViewColumn *column,
                                        gpointer         user_data)
{

}


void
on_button_chercher_event_membre_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_sinscrire_event_membre_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_mesinscriptions_membre_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}




void
on_button_ajout_enregistrer_admin_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_ajout_annuler_admin_clicked  (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_mod_enrregistrer_admin_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_mod_annuler_admin_clicked    (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_ajout_enregistrertrainer_admin_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_ajout_annulertrainer_admin_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_mod_enregistrer_admin_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_ajout_annulermembre_admin_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_ajout_enregistremembrer_admin_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_mod_enregistrermembre_admin_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_mod_annulermembre_admin_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_ajout_enregistrerevent_admin_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_ajout_annulerevent_admin_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_mod_enregistrerevent_admin_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_mod_annulerevent_admin_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_ajout_enregistrercours_admin_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_ajout_annulercours_admin_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_mod_enregistrercours_admin_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_mod_annulercours_admin_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_inscri_confirmer_trainer_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_treeview__mesinscriptions_centre_trainer_row_activated
                                        (GtkTreeView     *treeview,
                                        GtkTreePath     *path,
                                        GtkTreeViewColumn *column,
                                        gpointer         user_data)
{

}


void
on_checkbutton_mesinscri_selectall_trainer_toggled
                                        (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{

}


void
on_button_mesincri_centre_modrole_trainer_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_mesinscri_centre_annuler_trainer_enter
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_mesinscri_retour_trainer_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_mesinscri_enregistrermod_trainer_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_mesinscri_modannuler_trainer_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_supp_cours_trainer_clicked   (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_mescours_chercherid_trainer_enter
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_mescours_retour_trainer_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_treeview_listcours_membre_row_activated
                                        (GtkTreeView     *treeview,
                                        GtkTreePath     *path,
                                        GtkTreeViewColumn *column,
                                        gpointer         user_data)
{

}


void
on_checkbutton_selectout_mescours_membre_toggled
                                        (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{

}


void
on_button_supp_cours_membre_clicked    (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_mescours_retour_membre_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_treeview_liste_entraineur_membre_row_activated
                                        (GtkTreeView     *treeview,
                                        GtkTreePath     *path,
                                        GtkTreeViewColumn *column,
                                        gpointer         user_data)
{

}


void
on_checkbutton_selectionner_toutentraineur_membre_toggled
                                        (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{

}


void
on_button_supp_entraineur_membre_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_retour_mesentraineur_membre_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_treeview_listevent_membre_row_activated
                                        (GtkTreeView     *treeview,
                                        GtkTreePath     *path,
                                        GtkTreeViewColumn *column,
                                        gpointer         user_data)
{

}


void
on_checkbutton_selectall_event_membre_toggled
                                        (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{

}


void
on_button_supp_myevent_membre_clicked  (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_button_mesevent_retour_membre_clicked
                                        (GtkButton       *button,
                                        gpointer         user_data)
{

}


void
on_radiobutton_filtre_payant_event_admin_toggled
                                        (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{

}


void
on_radiobutton_filtre_gratuit_event_admin_toggled
                                        (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{

}


void
on_radiobutton_filtre_gratuit_membre_toggled
                                        (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{

}


void
on_radiobutton_filtre_payant_event_membre_toggled
                                        (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{

}

// Add these two functions at the END of callbacks.c (before the last closing brace)

void on_button_trier_date_clicked(GtkButton *button, gpointer user_data)
{
}

void on_treeview_listes_du_cours_row_activated(GtkTreeView *treeview, 
                                                 GtkTreePath *path, 
                                                 GtkTreeViewColumn *column, 
                                                 gpointer user_data)
{

}
