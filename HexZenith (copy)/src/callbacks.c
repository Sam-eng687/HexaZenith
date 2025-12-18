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
#include "centres.h"
#include "inscriptions.h"
#include "equipement.h"
#include "reservation.h"

#define id_entraineur current_trainer_id_str
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
void charger_types_activite_dans_combobox(GtkWidget *combobox);
void on_toggle_equipreserve_trainer(GtkCellRendererToggle *cell, gchar *path_str, gpointer data);
void on_checkbutton_selectall_equipreserve_trainer_toggled(GtkToggleButton *button, gpointer user_data);
/* External variable declarations - these are defined in main.c */
extern Centre centres[];
extern int nb_centres;
extern Inscription inscriptions[];
extern int nb_inscriptions;
extern const char *filename_centres;
extern const char *filename_inscriptions;
extern const char *filename_specialites;
extern char current_trainer_id_str[50];
extern char current_membre_id_str[50];

/* External window pointers - defined in main.c */
extern GtkWidget *window_login;
extern GtkWidget *window_espace_admin;
extern GtkWidget *window_espace_entraineur;
extern GtkWidget *window_espace_membres;



static GtkWidget* find_widget(GtkWidget *parent, const char *possible_names[]) {
    for (int i = 0; possible_names[i] != NULL; i++) {
        GtkWidget *widget = lookup_widget(parent, possible_names[i]);
        if (widget) {
            return widget;
        }
    }
    return NULL;
}
/* Helper function to find admin window */
GtkWidget* find_admin_window(void) {
    // First, try using the global pointer
    extern GtkWidget *window_espace_admin;
    if (window_espace_admin != NULL && GTK_IS_WINDOW(window_espace_admin)) {
        return window_espace_admin;
    }
    
    // If not available, search through all top-level windows
    GList *windows = gtk_window_list_toplevels();
    GList *iter = windows;
    GtkWidget *admin_window = NULL;
    
    while (iter) {
        GtkWidget *win = GTK_WIDGET(iter->data);
        const gchar *title = gtk_window_get_title(GTK_WINDOW(win));
        
        // Look for window with "admin" in the title (adjust based on your actual window title)
        if (title && (strstr(title, "admin") || strstr(title, "Admin") || 
                      strstr(title, "ADMIN") || strstr(title, "Espace Admin"))) {
            admin_window = win;
            break;
        }
        
        iter = iter->next;
    }
    
    g_list_free(windows);
    return admin_window;
}
// Function to authenticate user
int authenticate_user(const char *username, const char *password, int *user_id) {
    FILE *file = fopen(USER_FILE_PATH, "r");
    if (file == NULL) {
        g_warning("Cannot open user file: %s", USER_FILE_PATH);
        return 0;
    }

    char line[256];
    int role, id;
    char file_username[50], file_password[50];
    int user_found = 0;

    while (fgets(line, sizeof(line), file)) {
        if (sscanf(line, "%d %s %s %d", &role, file_username, file_password, &id) == 4) {
            if (strcmp(file_username, username) == 0) {
                user_found = 1;
                if (strcmp(file_password, password) == 0) {
                    *user_id = id;
                    fclose(file);
                    return role;
                } else {
                    fclose(file);
                    return -1;
                }
            }
        }
    }

    fclose(file);
    return 0;
}

// Show message dialog
void show_message(GtkWidget *parent, const char *message) {
    GtkWidget *dialog = gtk_message_dialog_new(
        GTK_WINDOW(parent),
        GTK_DIALOG_MODAL,
        GTK_MESSAGE_ERROR,
        GTK_BUTTONS_OK,
        "%s", message
    );
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// Login button callback


void on_button_login_clicked(GtkWidget *button, gpointer user_data) {
    GtkWidget *username_entry, *password_entry;
    const char *username, *password;
    int user_id = -1;
    int auth_result;

    GtkWidget *login_window = lookup_widget(button, "window_login");
    if (!login_window) {
        login_window = gtk_widget_get_toplevel(button);
    }

    username_entry = lookup_widget(login_window, "login_username_entry");
    password_entry = lookup_widget(login_window, "login_password_entry");

    if (!username_entry || !password_entry) {
        show_message(login_window, "Cannot find login fields!");
        return;
    }

    username = gtk_entry_get_text(GTK_ENTRY(username_entry));
    password = gtk_entry_get_text(GTK_ENTRY(password_entry));

    if (strlen(username) == 0 || strlen(password) == 0) {
        show_message(login_window, "Please enter both username and password!");
        return;
    }

    auth_result = authenticate_user(username, password, &user_id);

    if (auth_result == 0) {
        show_message(login_window, "Wrong username!");
        gtk_entry_set_text(GTK_ENTRY(password_entry), "");
    } else if (auth_result == -1) {
        show_message(login_window, "Wrong password for this username!");
        gtk_entry_set_text(GTK_ENTRY(password_entry), "");
    } else {
        // Authentication successful
        extern GtkWidget *window_espace_admin;
        extern GtkWidget *window_espace_entraineur;
        extern GtkWidget *window_espace_membres;
        extern char current_trainer_id_str[50];
        extern char current_membre_id_str[50];
        
        gtk_widget_hide(login_window);
        
        // Clear login fields
        gtk_entry_set_text(GTK_ENTRY(username_entry), "");
        gtk_entry_set_text(GTK_ENTRY(password_entry), "");

        switch (auth_result) {
            case 1:  // Admin
                if (window_espace_admin == NULL) {
                    window_espace_admin = create_window_espace_admin();
                }
                gtk_widget_show(window_espace_admin);
                break;
            
            case 2:  // Trainer
                sprintf(current_trainer_id_str, "%d", user_id);
                
                if (window_espace_entraineur == NULL) {
                    window_espace_entraineur = create_window_espace_entraineur();
                }
                gtk_widget_show(window_espace_entraineur);
                break;
            
            case 3:  // Member
                sprintf(current_membre_id_str, "%d", user_id);
                
                if (window_espace_membres == NULL) {
                    window_espace_membres = create_window_espace_membres();
                }
                gtk_widget_show(window_espace_membres);
                break;
            
            default:
                show_message(login_window, "Invalid role!");
                gtk_widget_show(login_window);
                break;
        }
    }
}

// Logout admin callback
// Replace the logout functions in callbacks.c with these fixed versions:

// Logout admin callback
void on_button_logout_admin_clicked(GtkWidget *button, gpointer user_data) {
    extern GtkWidget *window_espace_admin;
    extern GtkWidget *window_login;
    
    if (window_espace_admin) {
        gtk_widget_hide(window_espace_admin);
    }
    
    if (window_login == NULL) {
        window_login = create_window_login();
    }
    gtk_widget_show(window_login);
}

// Logout trainer callback
void on_button_logout_entraineur_clicked(GtkWidget *button, gpointer user_data) {
    extern GtkWidget *window_espace_entraineur;
    extern GtkWidget *window_login;
    extern char current_trainer_id_str[50];
    
    strcpy(current_trainer_id_str, "");
    
    if (window_espace_entraineur) {
        gtk_widget_hide(window_espace_entraineur);
    }
    
    if (window_login == NULL) {
        window_login = create_window_login();
    }
    gtk_widget_show(window_login);
}

// Logout membre callback
void on_button_logout_membre_clicked(GtkWidget *button, gpointer user_data) {
    extern GtkWidget *window_espace_membres;
    extern GtkWidget *window_login;
    extern char current_membre_id_str[50];
    
    strcpy(current_membre_id_str, "");
    
    if (window_espace_membres) {
        gtk_widget_hide(window_espace_membres);
    }
    
    if (window_login == NULL) {
        window_login = create_window_login();
    }
    gtk_widget_show(window_login);
}
   void on_window_espace_admin_show(GtkWidget *widget, gpointer user_data) {
    GtkWidget *treeviewequip, *treeviewcour, *combobox_filtre;
    GtkWidget *treeview_centres, *combobox_centre_types;
    GtkWidget *treeview_events;
    GtkWidget *treeview_entraineurs;
    GtkWidget *treeview_membres;
    
    // Refresh equipment treeview
    treeviewequip = lookup_widget(widget, "treeview_listeequipements_admin");
    if (treeviewequip != NULL) {
        refresh_equipment_treeview(treeviewequip);
    }
    
    // Refresh courses treeview
    treeviewcour = lookup_widget(widget, "treeview_liste_cours_admin");
    if (treeviewcour != NULL) {
        refresh_admin_courses_treeview(treeviewcour);
    }
    
    // Refresh equipment types combobox
    combobox_filtre = lookup_widget(widget, "combobox_filter_equip_admin");
    if (combobox_filtre != NULL) {
        refresh_equipment_types_combobox(combobox_filtre, 1);
    }
    
    // NEW: Refresh centres treeview for admin
    treeview_centres = lookup_widget(widget, "treeview_list_centres_admin");
    if (treeview_centres != NULL) {
        refresh_admin_centres_treeview(treeview_centres);
    }
    
    // NEW: Refresh centre types combobox for admin
    combobox_centre_types = lookup_widget(widget, "combobox_filtrer_partype");
    if (combobox_centre_types != NULL) {
        refresh_centre_types_combobox(combobox_centre_types, CENTRE_FILE_PATH);
    }
    //NEW: Refresh events treeview for admin
    treeview_events = lookup_widget(widget, "treeview_liste_event_admin");
    if (treeview_events != NULL) {
        refresh_admin_events_treeview(treeview_events);
    }
    //refresh trainers
    treeview_entraineurs = lookup_widget(widget, "treeview_listentraineurs_admin");
    if (treeview_entraineurs != NULL) {
        refresh_admin_trainers_treeview(treeview_entraineurs);
    }
    treeview_membres = lookup_widget(widget, "treeview_listemembre_admin");
    if (treeview_membres != NULL) {
        refresh_admin_membres_treeview(treeview_membres);
    }
}

// Update on_window_espace_entraineur_show function:
void on_window_espace_entraineur_show(GtkWidget *widget, gpointer user_data) {
    GtkWidget *treeview, *combobox_type, *combobox_niveau;
    GtkWidget *treeview_centres, *combobox_centre_types;
    GtkWidget *treeviewcour;


    // Refresh courses treeview for trainer
    treeviewcour = lookup_widget(widget, "treeview_cours_dispo_entraineur");
    if (treeviewcour != NULL) {
        refresh_admin_courses_treeview(treeviewcour);
    }
    // Refresh trainer courses treeview
    treeview = lookup_widget(widget, "treeview_liste_mescours_entraineur");
    if (treeview != NULL) {
        refresh_trainer_courses_treeview(treeview, current_trainer_id_str);
    }

    
    // Refresh activity types combobox
    combobox_type = lookup_widget(widget, "combobox_filter_typeactivite_entraineur");
    if (combobox_type != NULL) {
        refresh_activity_types_combobox(combobox_type);
    }
    
    // Refresh niveau combobox
    combobox_niveau = lookup_widget(widget, "combobox_filter_niveau_entraineur");
    if (combobox_niveau != NULL) {
        GtkTreeModel *model = gtk_combo_box_get_model(GTK_COMBO_BOX(combobox_niveau));
        if (model != NULL) {
            gtk_list_store_clear(GTK_LIST_STORE(model));
        }
        
        gtk_combo_box_append_text(GTK_COMBO_BOX(combobox_niveau), "Aucune");
        gtk_combo_box_append_text(GTK_COMBO_BOX(combobox_niveau), "Debutant");
        gtk_combo_box_append_text(GTK_COMBO_BOX(combobox_niveau), "Intermediaire");
        gtk_combo_box_append_text(GTK_COMBO_BOX(combobox_niveau), "Avance");
        gtk_combo_box_set_active(GTK_COMBO_BOX(combobox_niveau), 0);
    }
    
    // NEW: Refresh centres treeview for trainer
    treeview_centres = lookup_widget(widget, "treeview_listecentres_entraineur");
    if (treeview_centres != NULL) {
        refresh_trainer_centres_treeview(treeview_centres);
    }
    
    // NEW: Refresh centre types combobox for trainer
    combobox_centre_types = lookup_widget(widget, "combobox_filtrer_partype_entraineur");
    if (combobox_centre_types != NULL) {
        refresh_trainer_centre_types_combobox(combobox_centre_types, CENTRE_FILE_PATH);
    }
   
}
void on_window_espace_membres_show(GtkWidget *widget, gpointer user_data) {
    GtkWidget *treeview_cours;
    GtkWidget *treeview_events;
    GtkWidget *treeview_entraineurs_prives;

    // Load courses treeview (only courses with available places)
    treeview_cours = lookup_widget(widget, "treeview_listes_du_cours");
    if (treeview_cours != NULL) {
        refresh_member_courses_treeview(treeview_cours);
    }
  //NEW: Refresh events treeview for admin
    treeview_events = lookup_widget(widget, "treeview_liste_event_membre");
    if (treeview_events != NULL) {
        refresh_admin_events_treeview(treeview_events);
    }
    treeview_entraineurs_prives = lookup_widget(widget, "treeview_listeentraineur_membre");
    if (treeview_entraineurs_prives != NULL) {
        refresh_membre_entraineurs_prives_treeview(treeview_entraineurs_prives);
    }
}

//-----ajouter centre----------
void on_button_ajouter_centre_admin_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *window_ajouter = create_window_ajouter_centre();
    gtk_widget_show(window_ajouter);
}

void on_button_ajout_enregistrer_admin_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));

    Centre new_c;
    memset(&new_c, 0, sizeof(Centre));
    
    // FIXED WIDGET IDs (note the corrections)
    const char *id_names[] = {"entry_ajout_idcentre_admin", NULL};
    const char *nom_names[] = {"entry_ajout_nomcentre_admin", NULL};
    const char *type_names[] = {"entry_ajout_type_centre_admin", NULL}; // FIXED: "type_centre" not "typecentre"
    const char *ville_names[] = {"entry_ajout_villecentre_admin", NULL};
    const char *spin_cap_names[] = {"spinbutton_ajout_capacitecentre_admin", NULL};
    const char *ouvert_names[] = {"radiobutton_ajout_centreouvert_admin", NULL}; // FIXED: "radiobutton" not "radiobutto"
    const char *ferme_names[] = {"radiobutton_ajout_centreferme_admin", NULL}; // ADDED: ferme radio button
    const char *parking_names[] = {"checkbutton_ajout_parkingcentre_admin", NULL};
    const char *wifi_names[] = {"checkbutton_ajout_wificentre_admin", NULL};
    const char *cafeteria_names[] = {"checkbutton_ajout_cafeteriacentre_admin", NULL};
    
    GtkWidget *entry_id = find_widget(window, id_names);
    GtkWidget *entry_nom = find_widget(window, nom_names);
    GtkWidget *entry_type = find_widget(window, type_names);
    GtkWidget *entry_ville = find_widget(window, ville_names); // MOVED: ville entry is required
    GtkWidget *spin_cap = find_widget(window, spin_cap_names);
    GtkWidget *rb_ouvert = find_widget(window, ouvert_names);
    GtkWidget *rb_ferme = find_widget(window, ferme_names); // ADDED
    GtkWidget *check_parking = find_widget(window, parking_names);
    GtkWidget *check_wifi = find_widget(window, wifi_names);
    GtkWidget *check_cafeteria = find_widget(window, cafeteria_names);

    // Debug: Check which widgets are found
    if (!entry_id) g_warning("Missing: entry_ajout_idcentre_admin");
    if (!entry_nom) g_warning("Missing: entry_ajout_nomcentre_admin");
    if (!entry_type) g_warning("Missing: entry_ajout_type_centre_admin");
    if (!entry_ville) g_warning("Missing: entry_ajout_villecentre_admin");
    if (!spin_cap) g_warning("Missing: spinbutton_ajout_capacitecentre_admin");
    if (!rb_ouvert) g_warning("Missing: radiobutton_ajout_centreouvert_admin");
    if (!rb_ferme) g_warning("Missing: radiobutton_ajout_centreferme_admin");
    if (!check_parking) g_warning("Missing: checkbutton_ajout_parkingcentre_admin");
    if (!check_wifi) g_warning("Missing: checkbutton_ajout_wificentre_admin");
    if (!check_cafeteria) g_warning("Missing: checkbutton_ajout_cafeteriacentre_admin");

    // Ville is required but wasn't in the original check
    if (!entry_id || !entry_nom || !entry_type || !entry_ville) {
        show_message(window, "Champs requis manquants!");
        return;
    }

    const char *id_text = gtk_entry_get_text(GTK_ENTRY(entry_id));
    const char *nom_text = gtk_entry_get_text(GTK_ENTRY(entry_nom));
    const char *ville_text = gtk_entry_get_text(GTK_ENTRY(entry_ville));
    const char *type_text = gtk_entry_get_text(GTK_ENTRY(entry_type));
    
    if (strlen(id_text) == 0 || strlen(nom_text) == 0 || strlen(ville_text) == 0 || strlen(type_text) == 0) {
        show_message(window, "ID, Nom, Type et Ville sont requis!");
        return;
    }

    strncpy(new_c.id, id_text, ID_LEN-1); 
    new_c.id[ID_LEN-1] = '\0';
    
    strncpy(new_c.nom, nom_text, NOM_LEN-1); 
    new_c.nom[NOM_LEN-1] = '\0';
    
    strncpy(new_c.type, type_text, TYPE_LEN-1); 
    new_c.type[TYPE_LEN-1] = '\0';
    
    strncpy(new_c.ville, ville_text, VILLE_LEN-1); 
    new_c.ville[VILLE_LEN-1] = '\0';
    
    if (spin_cap) {
        new_c.capacite = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_cap));
    } else {
        new_c.capacite = 0; // Default value
    }
    
    // Handle radio buttons: ouvert vs ferme
    if (rb_ouvert && rb_ferme) {
        // Check which radio button is active
        if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rb_ouvert))) {
            new_c.ouvert = 1; // Centre ouvert
        } else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rb_ferme))) {
            new_c.ouvert = 0; // Centre fermé
        } else {
            new_c.ouvert = 1; // Default to ouvert if neither is selected
        }
    } else {
        new_c.ouvert = 1; // Default to ouvert if radio buttons not found
    }
    
    if (check_parking) {
        new_c.parking = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_parking)) ? 1 : 0;
    } else {
        new_c.parking = 0;
    }
    
    if (check_wifi) {
        new_c.wifi = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_wifi)) ? 1 : 0;
    } else {
        new_c.wifi = 0;
    }
    
    if (check_cafeteria) {
        new_c.cafeteria = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_cafeteria)) ? 1 : 0;
    } else {
        new_c.cafeteria = 0;
    }

    if (ajouter_centre(centres, &nb_centres, new_c) == 0) {
        sauvegarder_centres(filename_centres, centres, nb_centres);
        show_message(window, "Centre ajouté avec succès!");
        
        GtkWidget *window_admin = find_admin_window();
        
        if (window_admin) {
            GtkWidget *treeview = lookup_widget(window_admin, "treeview_list_centres_admin");
            if (treeview) {
                refresh_treeview(GTK_TREE_VIEW(treeview), centres, nb_centres);
                
                const char *combobox_names[] = {
                    "combobox_filtrer_partype",
                    "combobox_type_filter",
                    "filter_type_combobox",
                    NULL
                };
                GtkWidget *combobox_widget = find_widget(window_admin, combobox_names);
                if (combobox_widget) {
                    populate_combobox_types(combobox_widget, centres, nb_centres);
                }
            }
        }
        
        gtk_widget_hide(window);
    } else {
        show_message(window, "Impossible d'ajouter le centre (ID existant ou erreur).");
    }
}

void on_button_ajout_annuler_admin_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    gtk_widget_hide(window);
}

/* ========== Modifier Centre ========== */
void on_button_modifier_centre_admin_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *window_admin = find_admin_window();
    if (!window_admin) return;
    
    const char *treeview_names[] = {
        "treeview_list_centres_admin",
        "treeview_centres",
        "centres_treeview",
        NULL
    };
    GtkWidget *treeview = find_widget(window_admin, treeview_names);
    if (!treeview) return;
    
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
    GtkTreeIter iter;
    GtkTreeModel *model;
    
    if (!gtk_tree_selection_get_selected(selection, &model, &iter)) {
        show_message(NULL, "Veuillez sélectionner un centre à modifier.");
        return;
    }
    
    gchar *id;
    gtk_tree_model_get(model, &iter, 0, &id, -1);
    
    int index = -1;
    for (int i = 0; i < nb_centres; i++) {
        if (strcmp(centres[i].id, id) == 0) {
            index = i;
            break;
        }
    }
    g_free(id);
    
    if (index == -1) return;
    
    GtkWidget *window_mod = create_window_modifier_centre();
    gtk_widget_show(window_mod);
    
    // FIXED: entry_mod_ville_centre_admin (with underscore)
    const char *mod_id_names[] = {"entry_mod_idcentre_admin", NULL};
    const char *mod_nom_names[] = {"entry_mod_nomcentre_admin", NULL};
    const char *mod_ville_names[] = {"entry_mod_ville_centre_admin", NULL}; // FIXED
    const char *mod_type_names[] = {"entry_mod_typecentre_admin", NULL};
    const char *mod_cap_names[] = {"spinbutton_mod_capacitecentre_admin", NULL};
    
    GtkWidget *entry_id = find_widget(window_mod, mod_id_names);
    GtkWidget *entry_nom = find_widget(window_mod, mod_nom_names);
    GtkWidget *entry_ville = find_widget(window_mod, mod_ville_names);
    GtkWidget *entry_type = find_widget(window_mod, mod_type_names);
    GtkWidget *spin_cap = find_widget(window_mod, mod_cap_names);
    
    if (!entry_id || !entry_nom || !entry_ville || !entry_type || !spin_cap) {
        g_warning("Could not find all widgets in modify window");
        if (!entry_id) g_warning("Missing: entry_mod_idcentre_admin");
        if (!entry_nom) g_warning("Missing: entry_mod_nomcentre_admin");
        if (!entry_ville) g_warning("Missing: entry_mod_ville_centre_admin");
        if (!entry_type) g_warning("Missing: entry_mod_typecentre_admin");
        if (!spin_cap) g_warning("Missing: spinbutton_mod_capacitecentre_admin");
        gtk_widget_destroy(window_mod);
        return;
    }
    
    gtk_entry_set_text(GTK_ENTRY(entry_id), centres[index].id);
    gtk_entry_set_text(GTK_ENTRY(entry_nom), centres[index].nom);
    gtk_entry_set_text(GTK_ENTRY(entry_ville), centres[index].ville);
    gtk_entry_set_text(GTK_ENTRY(entry_type), centres[index].type);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_cap), centres[index].capacite);
    
    const char *ouvert_radio_names[] = {"radiobutton_mod_centreouvert_admin", NULL};
    const char *ferme_radio_names[] = {"radiobutton_mod_centreferme_admin", NULL};
    
    GtkWidget *rb_ouvert = find_widget(window_mod, ouvert_radio_names);
    GtkWidget *rb_ferme = find_widget(window_mod, ferme_radio_names);
    
    if (rb_ouvert && rb_ferme) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rb_ouvert), centres[index].ouvert);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rb_ferme), !centres[index].ouvert);
    }
    
    const char *parking_check_names[] = {"checkbutton_mod_parkingcentre_admin", NULL};
    const char *wifi_check_names[] = {"checkbutton_mod_wificentre_admin", NULL};
    const char *cafe_check_names[] = {"checkbutton_mod_cafeteriacentre_admin", NULL};
    
    GtkWidget *check_parking = find_widget(window_mod, parking_check_names);
    GtkWidget *check_wifi = find_widget(window_mod, wifi_check_names);
    GtkWidget *check_cafe = find_widget(window_mod, cafe_check_names);
    
    if (check_parking) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_parking), centres[index].parking);
    if (check_wifi) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_wifi), centres[index].wifi);
    if (check_cafe) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_cafe), centres[index].cafeteria);
    
    gtk_editable_set_editable(GTK_EDITABLE(entry_id), FALSE);
    gtk_editable_set_editable(GTK_EDITABLE(entry_nom), FALSE);
    gtk_editable_set_editable(GTK_EDITABLE(entry_ville), FALSE);
}

void on_button_mod_enrregistrer_admin_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *window_mod = gtk_widget_get_toplevel(GTK_WIDGET(button));
    
    const char *mod_id_names[] = {"entry_mod_idcentre_admin", NULL};
    GtkWidget *entry_id = find_widget(window_mod, mod_id_names);
    if (!entry_id) return;
    
    const char *id = gtk_entry_get_text(GTK_ENTRY(entry_id));
    
    int index = -1;
    for (int i = 0; i < nb_centres; i++) {
        if (strcmp(centres[i].id, id) == 0) {
            index = i;
            break;
        }
    }
    
    if (index == -1) {
        gtk_widget_destroy(window_mod);
        return;
    }
    
    // FIXED: entry_mod_ville_centre_admin (with underscore)
    const char *mod_type_names[] = {"entry_mod_typecentre_admin", NULL};
    const char *mod_cap_names[] = {"spinbutton_mod_capacitecentre_admin", NULL};
    const char *ouvert_radio_names[] = {"radiobutton_mod_centreouvert_admin", NULL};
    const char *parking_check_names[] = {"checkbutton_mod_parkingcentre_admin", NULL};
    const char *wifi_check_names[] = {"checkbutton_mod_wificentre_admin", NULL};
    const char *cafe_check_names[] = {"checkbutton_mod_cafeteriacentre_admin", NULL};
    
    GtkWidget *entry_type = find_widget(window_mod, mod_type_names);
    GtkWidget *spin_cap = find_widget(window_mod, mod_cap_names);
    GtkWidget *rb_ouvert = find_widget(window_mod, ouvert_radio_names);
    GtkWidget *check_parking = find_widget(window_mod, parking_check_names);
    GtkWidget *check_wifi = find_widget(window_mod, wifi_check_names);
    GtkWidget *check_cafe = find_widget(window_mod, cafe_check_names);
    
    if (entry_type) {
        strncpy(centres[index].type, gtk_entry_get_text(GTK_ENTRY(entry_type)), TYPE_LEN-1);
        centres[index].type[TYPE_LEN-1] = '\0';
    }
    
    if (spin_cap) {
        centres[index].capacite = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_cap));
    }
    
    if (rb_ouvert) {
        centres[index].ouvert = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rb_ouvert)) ? 1 : 0;
    }
    
    if (check_parking) {
        centres[index].parking = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_parking)) ? 1 : 0;
    }
    
    if (check_wifi) {
        centres[index].wifi = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_wifi)) ? 1 : 0;
    }
    
    if (check_cafe) {
        centres[index].cafeteria = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_cafe)) ? 1 : 0;
    }
    
    sauvegarder_centres(filename_centres, centres, nb_centres);
    
    GtkWidget *window_admin = find_admin_window();
    
    if (window_admin) {
        const char *treeview_names[] = {
            "treeview_list_centres_admin",
            "treeview_centres",
            "centres_treeview",
            NULL
        };
        GtkWidget *treeview = find_widget(window_admin, treeview_names);
        if (treeview) {
            refresh_treeview(GTK_TREE_VIEW(treeview), centres, nb_centres);
            
            const char *combobox_names[] = {
                "combobox_filtrer_partype",
                "combobox_type_filter",
                "filter_type_combobox",
                NULL
            };
            GtkWidget *combobox_widget = find_widget(window_admin, combobox_names);
            if (combobox_widget) {
                populate_combobox_types(combobox_widget, centres, nb_centres);
            }
        }
    }
    
    gtk_widget_destroy(window_mod);
}

void on_button_mod_annuler_admin_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    gtk_widget_destroy(window);
}

/* ========== Supprimer Centre ========== */
void on_button_supprimer_centre_admin_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *window_admin = find_admin_window();
    int index_to_delete = -1;
    if (!window_admin) return;
    
    const char *treeview_names[] = {
        "treeview_list_centres_admin",
        "treeview_centres",
        "centres_treeview",
        NULL
    };
    GtkWidget *treeview = find_widget(window_admin, treeview_names);
    if (!treeview) return;
    
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
    GtkTreeIter iter;
    GtkTreeModel *model;
    
    if (!gtk_tree_selection_get_selected(selection, &model, &iter)) {
        show_message(NULL, "Veuillez sélectionner un centre à supprimer.");
        return;
    }
    
    gchar *id;
    gtk_tree_model_get(model, &iter, 0, &id, -1);
    
    index_to_delete = -1;
    for (int i = 0; i < nb_centres; i++) {
        if (strcmp(centres[i].id, id) == 0) {
            index_to_delete = i;
            break;
        }
    }
    g_free(id);
    
    if (index_to_delete == -1) return;
    
    // Create confirmation popup dialog
    GtkWidget *dialog = gtk_message_dialog_new(
        GTK_WINDOW(window_admin),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_QUESTION,
        GTK_BUTTONS_YES_NO,
        "Êtes-vous sûr de vouloir supprimer ce centre ?"
    );
    
    gtk_window_set_title(GTK_WINDOW(dialog), "Confirmation de suppression");
    
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    
    if (response == GTK_RESPONSE_YES) {
        // User clicked Yes - proceed with deletion
        supprimer_centre(centres, &nb_centres, index_to_delete);
        sauvegarder_centres(filename_centres, centres, nb_centres);
        
        // Refresh the treeview
        if (treeview) {
            refresh_treeview(GTK_TREE_VIEW(treeview), centres, nb_centres);
            
            const char *combobox_names[] = {
                "combobox_filtrer_partype",
                "combobox_type_filter",
                "filter_type_combobox",
                NULL
            };
            GtkWidget *combobox_widget = find_widget(window_admin, combobox_names);
            if (combobox_widget) {
                populate_combobox_types(combobox_widget, centres, nb_centres);
            }
        }
        
     show_message(window_admin, "Centre supprimé avec succès.");

    }
    
    // Reset index regardless of response
    index_to_delete = -1;
}




/* ========== Recherche / Filtre / Tri ========== */
void on_button_chercher_par_idcentre_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    
    GtkWidget *entry_id = lookup_widget(window, "entry_recherche_par_idcentre");
    GtkWidget *treeview = lookup_widget(window, "treeview_list_centres_admin");
    
    if (!entry_id) {
        g_warning("Search entry 'entry_recherche_par_idcentre' not found!");
        
        const char *alt_entry_names[] = {
            "entry_chercher_id",
            "entry_id_recherche", 
            NULL
        };
        
        for (int i = 0; alt_entry_names[i] != NULL; i++) {
            entry_id = lookup_widget(window, alt_entry_names[i]);
            if (entry_id) {
                printf("DEBUG: Found entry with alternative name: %s\n", alt_entry_names[i]);
                break;
            }
        }
    }
    
    if (!treeview) {
        g_warning("TreeView 'treeview_list_centres_admin' not found!");
    }
    
    if (!entry_id || !treeview) {
        show_message(window, "Widgets de recherche non trouvés!");
        return;
    }
    
    const char *id = gtk_entry_get_text(GTK_ENTRY(entry_id));
    
    if (strlen(id) == 0) {
        /* If search is empty, show all centres */
        refresh_treeview(GTK_TREE_VIEW(treeview), centres, nb_centres);
        return;
    }
    
    Centre result[MAX_CENTRES];
    int r_count = 0;
    
    rechercher_par_id(centres, nb_centres, id, result, &r_count);
    refresh_treeview(GTK_TREE_VIEW(treeview), result, r_count);
    
    if (r_count == 0) {
        show_message(window, "Aucun centre trouvé avec cet ID.");
    }
}

void on_combobox_filtrer_partype_changed(GtkComboBox *combobox, gpointer user_data) {
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(combobox));
    
    const char *treeview_names[] = {
        "treeview_list_centres_admin",
        "treeview_centres",
        "centres_treeview",
        NULL
    };
    GtkWidget *treeview = find_widget(window, treeview_names);
    
    if (!treeview) {
        g_warning("TreeView not found in on_combobox_filtrer_partype_changed");
        return;
    }
    
    /* Get the active text from GtkComboBox */
    GtkTreeModel *model = gtk_combo_box_get_model(combobox);
    GtkTreeIter iter;
    gchar *type = NULL;
    
    if (gtk_combo_box_get_active_iter(combobox, &iter)) {
        gtk_tree_model_get(model, &iter, 0, &type, -1);
    }
    
    if (type == NULL || strlen(type) == 0 || strcmp(type, "Tous les types") == 0) {
        /* Show all when "All" selected or nothing selected */
        refresh_treeview(GTK_TREE_VIEW(treeview), centres, nb_centres);
        if (type) g_free(type);
        return;
    }
    
    Centre result[MAX_CENTRES];
    int r_count = 0;
    
    filtrer_par_type(centres, nb_centres, type, result, &r_count);
    refresh_treeview(GTK_TREE_VIEW(treeview), result, r_count);
    
    if (type) g_free(type);
}

void on_spinbutton_cap_min_admin_value_changed(GtkSpinButton *spinbutton, gpointer user_data) {
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(spinbutton));
    
    const char *treeview_names[] = {
        "treeview_list_centres_admin",
        "treeview_centres",
        "centres_treeview",
        NULL
    };
    GtkWidget *treeview = find_widget(window, treeview_names);
    
    if (!treeview) {
        g_warning("TreeView not found in on_spinbutton_cap_min_admin_value_changed");
        return;
    }
    
    int min_cap = gtk_spin_button_get_value_as_int(spinbutton);
    Centre result[MAX_CENTRES];
    int r_count = 0;
    
    for (int i = 0; i < nb_centres; i++) {
        if (centres[i].capacite >= min_cap) {
            result[r_count++] = centres[i];
        }
    }
    
    refresh_treeview(GTK_TREE_VIEW(treeview), result, r_count);
}

void on_checkbutton_Centres_ouverts_filtrage_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(togglebutton));
    
    const char *treeview_names[] = {
        "treeview_list_centres_admin",
        "treeview_centres",
        "centres_treeview",
        NULL
    };
    GtkWidget *treeview = find_widget(window, treeview_names);
    
    if (!treeview) {
        g_warning("TreeView not found in on_checkbutton_Centres_ouverts_filtrage_toggled");
        return;
    }
    
    gboolean filter_ouvert = gtk_toggle_button_get_active(togglebutton);
    Centre result[MAX_CENTRES];
    int r_count = 0;
    
    for (int i = 0; i < nb_centres; i++) {
        if (!filter_ouvert || centres[i].ouvert) {
            result[r_count++] = centres[i];
        }
    }
    
    refresh_treeview(GTK_TREE_VIEW(treeview), result, r_count);
}

static int cmp_nom(const void *a, const void *b) {
    return strcmp(((Centre*)a)->nom, ((Centre*)b)->nom);
}

static int cmp_ville(const void *a, const void *b) {
    return strcmp(((Centre*)a)->ville, ((Centre*)b)->ville);
}

void on_radiobutton_tri_nom_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
    if (!gtk_toggle_button_get_active(togglebutton)) return;
    
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(togglebutton));
    
    const char *treeview_names[] = {
        "treeview_list_centres_admin",
        "treeview_centres",
        "centres_treeview",
        NULL
    };
    GtkWidget *treeview = find_widget(window, treeview_names);
    
    if (!treeview) {
        g_warning("TreeView not found in on_radiobutton_tri_nom_toggled");
        return;
    }
    
    /* Create a copy to sort */
    Centre sorted[MAX_CENTRES];
    memcpy(sorted, centres, nb_centres * sizeof(Centre));
    qsort(sorted, nb_centres, sizeof(Centre), cmp_nom);
    refresh_treeview(GTK_TREE_VIEW(treeview), sorted, nb_centres);
}

void on_radiobutton_tri_ville_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
    if (!gtk_toggle_button_get_active(togglebutton)) return;
    
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(togglebutton));
    
    const char *treeview_names[] = {
        "treeview_list_centres_admin",
        "treeview_centres",
        "centres_treeview",
        NULL
    };
    GtkWidget *treeview = find_widget(window, treeview_names);
    
    if (!treeview) {
        g_warning("TreeView not found in on_radiobutton_tri_ville_toggled");
        return;
    }
    
    /* Create a copy to sort */
    Centre sorted[MAX_CENTRES];
    memcpy(sorted, centres, nb_centres * sizeof(Centre));
    qsort(sorted, nb_centres, sizeof(Centre), cmp_ville);
    refresh_treeview(GTK_TREE_VIEW(treeview), sorted, nb_centres);
}



/* ========================================================================== */
/* ==================== TRAINER FUNCTIONS =================================== */
/* ========================================================================== */

/* ---------- Helper Functions for Trainer ---------- */

/* Populate specialites combobox from file */
static void populate_combobox_specialites(GtkWidget *combobox) {
    if (!combobox) {
        printf("DEBUG: combobox is NULL\n");
        return;
    }
    
    printf("DEBUG: populate_combobox_specialites called\n");
    printf("DEBUG: Attempting to open file: %s\n", filename_specialites);
    
    GtkComboBox *combo = GTK_COMBO_BOX(combobox);
    GtkTreeModel *model = gtk_combo_box_get_model(combo);
    
    if (model) {
        gtk_list_store_clear(GTK_LIST_STORE(model));
    } else {
        GtkListStore *store = gtk_list_store_new(1, G_TYPE_STRING);
        gtk_combo_box_set_model(combo, GTK_TREE_MODEL(store));
        g_object_unref(store);
    }
    
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_clear(GTK_CELL_LAYOUT(combo));
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(combo), renderer, TRUE);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(combo), renderer, "text", 0, NULL);
    
    model = gtk_combo_box_get_model(combo);
    GtkListStore *store = GTK_LIST_STORE(model);
    GtkTreeIter iter;
    
    char specialites[100][SPECIALITE_LEN];
    int nb_spec = 0;
    
    printf("DEBUG: Calling charger_specialites\n");
    int result = charger_specialites(filename_specialites, specialites, &nb_spec);
    printf("DEBUG: charger_specialites returned: %d, loaded %d specialites\n", result, nb_spec);
    
    if (result == 0 && nb_spec > 0) {
        for (int i = 0; i < nb_spec; i++) {
            printf("DEBUG: Adding specialite: %s\n", specialites[i]);
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter, 0, specialites[i], -1);
        }
        gtk_combo_box_set_active(combo, 0);
        printf("DEBUG: Successfully added %d specialites to combobox\n", nb_spec);
    } else {
        printf("DEBUG: Failed to load specialites or file is empty\n");
        printf("DEBUG: Check if file exists at: %s\n", filename_specialites);
    }
}
/* Callback for individual row selection in inscriptions treeview */
static void on_inscription_selection_toggled(GtkCellRendererToggle *cell, gchar *path_str, gpointer user_data) {
    GtkTreeView *treeview = GTK_TREE_VIEW(user_data);
    GtkTreeModel *model = gtk_tree_view_get_model(treeview);
    GtkTreePath *path = gtk_tree_path_new_from_string(path_str);
    GtkTreeIter iter;
    
    if (gtk_tree_model_get_iter(model, &iter, path)) {
        gboolean active;
        gtk_tree_model_get(model, &iter, 0, &active, -1);
        gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, !active, -1);
    }
    
    gtk_tree_path_free(path);
}

/* Setup TreeView for trainer inscriptions */
static void setup_treeview_inscriptions(GtkTreeView *treeview) {
    GtkListStore *store = gtk_list_store_new(6,
        G_TYPE_BOOLEAN, // Selection checkbox
        G_TYPE_STRING,  // Centre ID
        G_TYPE_STRING,  // Centre Nom
        G_TYPE_STRING,  // Specialite
        G_TYPE_STRING,  // Role
        G_TYPE_STRING   // Disponibilité
    );
    gtk_tree_view_set_model(treeview, GTK_TREE_MODEL(store));
    g_object_unref(store);

    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    
    // Checkbox column
    renderer = gtk_cell_renderer_toggle_new();
    column = gtk_tree_view_column_new_with_attributes("Sélection", renderer, "active", 0, NULL);
    gtk_tree_view_append_column(treeview, column);
    g_signal_connect(renderer, "toggled", G_CALLBACK(on_inscription_selection_toggled), treeview);
    
    const char *titles[] = {"ID Centre", "Nom Centre", "Spécialité", "Rôle", "Disponibilité"};
    for (int i = 0; i < 5; i++) {
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes(titles[i], renderer, "text", i+1, NULL);
        gtk_tree_view_column_set_resizable(column, TRUE);
        gtk_tree_view_append_column(treeview, column);
    }
}

/* Refresh inscriptions treeview for current trainer */
static void refresh_inscriptions_treeview(GtkTreeView *treeview) {
    GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(treeview));
    gtk_list_store_clear(store);
    
    Inscription trainer_inscriptions[MAX_INSCRIPTIONS];
    int count = 0;
    
    printf("DEBUG: Getting inscriptions for trainer '%s'\n", current_trainer_id_str);
    get_inscriptions_par_trainer(inscriptions, nb_inscriptions, current_trainer_id_str, trainer_inscriptions, &count);
    printf("DEBUG: Found %d inscriptions for this trainer\n", count);
    
    GtkTreeIter iter;
    for (int i = 0; i < count; i++) {
        // Find centre info
        Centre *c = NULL;
        for (int j = 0; j < nb_centres; j++) {
            if (strcmp(centres[j].id, trainer_inscriptions[i].centre_id) == 0) {
                c = &centres[j];
                break;
            }
        }
        
        printf("DEBUG: Adding row %d - centre_id='%s', nom='%s'\n", 
               i, trainer_inscriptions[i].centre_id, c ? c->nom : "N/A");
        
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter,
            0, FALSE,  // Not selected by default
            1, trainer_inscriptions[i].centre_id,
            2, c ? c->nom : "N/A",
            3, trainer_inscriptions[i].specialite,
            4, trainer_inscriptions[i].role,
            5, trainer_inscriptions[i].disponible ? "Disponible" : "Non disponible",
            -1);
    }
    
    printf("DEBUG: Finished adding %d rows to treeview\n", count);
}

/* ---------- Trainer Window Functions ---------- */


void on_button_chercher_centre_entraineur_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *entry_id = lookup_widget(window, "entry_recherche_parid_centres_entraineur");
    GtkWidget *treeview = lookup_widget(window, "treeview_listecentres_entraineur");
    
    if (!entry_id || !treeview) return;
    
    const char *id = gtk_entry_get_text(GTK_ENTRY(entry_id));
    
    if (strlen(id) == 0) {
        refresh_treeview(GTK_TREE_VIEW(treeview), centres, nb_centres);
        return;
    }
    
    Centre result[MAX_CENTRES];
    int r_count = 0;
    rechercher_par_id(centres, nb_centres, id, result, &r_count);
    refresh_treeview(GTK_TREE_VIEW(treeview), result, r_count);
    
    if (r_count == 0) {
        show_message(window, "Aucun centre trouvé avec cet ID.");
    }
}

void on_combobox_filtrer_partype_entraineur_changed(GtkComboBox *combobox, gpointer user_data) {
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(combobox));
    GtkWidget *treeview = lookup_widget(window, "treeview_listecentres_entraineur");
    if (!treeview) return;
    
    GtkTreeModel *model = gtk_combo_box_get_model(combobox);
    GtkTreeIter iter;
    gchar *type = NULL;
    
    if (gtk_combo_box_get_active_iter(combobox, &iter)) {
        gtk_tree_model_get(model, &iter, 0, &type, -1);
    }
    
    if (type == NULL || strlen(type) == 0 || strcmp(type, "Tous les types") == 0) {
        refresh_treeview(GTK_TREE_VIEW(treeview), centres, nb_centres);
        if (type) g_free(type);
        return;
    }
    
    Centre result[MAX_CENTRES];
    int r_count = 0;
    filtrer_par_type(centres, nb_centres, type, result, &r_count);
    refresh_treeview(GTK_TREE_VIEW(treeview), result, r_count);
    
    if (type) g_free(type);
}

void on_spinbutton_cap_min_entraineur_value_changed(GtkSpinButton *spinbutton, gpointer user_data) {
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(spinbutton));
    GtkWidget *treeview = lookup_widget(window, "treeview_listecentres_entraineur");
    if (!treeview) return;
    
    int min_cap = gtk_spin_button_get_value_as_int(spinbutton);
    Centre result[MAX_CENTRES];
    int r_count = 0;
    
    for (int i = 0; i < nb_centres; i++) {
        if (centres[i].capacite >= min_cap) {
            result[r_count++] = centres[i];
        }
    }
    
    refresh_treeview(GTK_TREE_VIEW(treeview), result, r_count);
}

void on_checkbutton_Centres_ouverts_filtrage_entraineur_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(togglebutton));
    GtkWidget *treeview = lookup_widget(window, "treeview_listecentres_entraineur");
    if (!treeview) return;
    
    gboolean filter_ouvert = gtk_toggle_button_get_active(togglebutton);
    Centre result[MAX_CENTRES];
    int r_count = 0;
    
    for (int i = 0; i < nb_centres; i++) {
        if (!filter_ouvert || centres[i].ouvert) {
            result[r_count++] = centres[i];
        }
    }
    
    refresh_treeview(GTK_TREE_VIEW(treeview), result, r_count);
}

void on_radiobutton_tri_nom_entraineur_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
    if (!gtk_toggle_button_get_active(togglebutton)) return;
    
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(togglebutton));
    GtkWidget *treeview = lookup_widget(window, "treeview_listecentres_entraineur");
    if (!treeview) return;
    
    Centre sorted[MAX_CENTRES];
    memcpy(sorted, centres, nb_centres * sizeof(Centre));
    qsort(sorted, nb_centres, sizeof(Centre), cmp_nom);
    refresh_treeview(GTK_TREE_VIEW(treeview), sorted, nb_centres);
}

void on_radiobutton_tri_ville_entraineur_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
    if (!gtk_toggle_button_get_active(togglebutton)) return;
    
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(togglebutton));
    GtkWidget *treeview = lookup_widget(window, "treeview_listecentres_entraineur");
    if (!treeview) return;
    
    Centre sorted[MAX_CENTRES];
    memcpy(sorted, centres, nb_centres * sizeof(Centre));
    qsort(sorted, nb_centres, sizeof(Centre), cmp_ville);
    refresh_treeview(GTK_TREE_VIEW(treeview), sorted, nb_centres);
}

/* ---------- Inscription Centre Functions ---------- */

void on_button_sinscrire_entraineur_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *window_trainer = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *treeview = lookup_widget(window_trainer, "treeview_listecentres_entraineur");
    
    if (!treeview) return;
    
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
    GtkTreeIter iter;
    GtkTreeModel *model;
    
    if (!gtk_tree_selection_get_selected(selection, &model, &iter)) {
        show_message(window_trainer, "Veuillez sélectionner un centre.");
        return;
    }
    
    gchar *centre_id, *centre_nom, *centre_type;
    gtk_tree_model_get(model, &iter, 0, &centre_id, 1, &centre_nom, 2, &centre_type, -1);
    
    GtkWidget *window_inscri = create_window_inscri_centre_trainer();
    
    GtkWidget *entry_id = lookup_widget(window_inscri, "entry_inscri_idcentre_trainer");
    GtkWidget *entry_nom = lookup_widget(window_inscri, "entry_inscri_nomcentre_trainer");
    GtkWidget *entry_type = lookup_widget(window_inscri, "entry_inscri_type_trainer");
    GtkWidget *combobox_spec = lookup_widget(window_inscri, "combobox_inscri_specialite_trainer");
    
    if (!combobox_spec) {
        printf("DEBUG: First lookup failed for combobox, trying GTK object data\n");
        combobox_spec = GTK_WIDGET(g_object_get_data(G_OBJECT(window_inscri), "combobox_inscri_specialite_trainer"));
    }
    
    if (entry_id) {
        gtk_entry_set_text(GTK_ENTRY(entry_id), centre_id);
        gtk_editable_set_editable(GTK_EDITABLE(entry_id), FALSE);
    } else {
        printf("DEBUG: entry_inscri_idcentre_trainer not found\n");
    }
    
    if (entry_nom) {
        gtk_entry_set_text(GTK_ENTRY(entry_nom), centre_nom);
        gtk_editable_set_editable(GTK_EDITABLE(entry_nom), FALSE);
    } else {
        printf("DEBUG: entry_inscri_nomcentre_trainer not found\n");
    }
    
    if (entry_type) {
        gtk_entry_set_text(GTK_ENTRY(entry_type), centre_type);
        gtk_editable_set_editable(GTK_EDITABLE(entry_type), FALSE);
    } else {
        printf("DEBUG: entry_inscri_type_trainer not found\n");
    }
    
    if (combobox_spec) {
        printf("DEBUG: Found combobox, populating specialites from: %s\n", filename_specialites);
        populate_combobox_specialites(combobox_spec);
        
        // Verify if specialites were loaded
        GtkTreeModel *combo_model = gtk_combo_box_get_model(GTK_COMBO_BOX(combobox_spec));
        if (combo_model) {
            int count = gtk_tree_model_iter_n_children(combo_model, NULL);
            printf("DEBUG: Combobox now has %d items\n", count);
        }
    } else {
        printf("DEBUG: combobox_inscri_specialite_trainer not found\n");
    }
    
    gtk_widget_show(window_inscri);
    
    g_free(centre_id);
    g_free(centre_nom);
    g_free(centre_type);
}

void on_button_inscri_confirmer_trainer_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    
    GtkWidget *entry_id = lookup_widget(window, "entry_inscri_idcentre_trainer");
    GtkWidget *combobox_spec = lookup_widget(window, "combobox_inscri_specialite_trainer");
    GtkWidget *rb_principal = lookup_widget(window, "radiobutton_inscri_principal_trainer");
    GtkWidget *check_dispo = lookup_widget(window, "checkbutton_inscri_dispo_trainer");
    
    if (!entry_id || !combobox_spec || !rb_principal) return;
    
    const char *centre_id = gtk_entry_get_text(GTK_ENTRY(entry_id));
    
    // Get specialite from combobox
    GtkTreeModel *model = gtk_combo_box_get_model(GTK_COMBO_BOX(combobox_spec));
    GtkTreeIter iter;
    gchar *specialite = NULL;
    
    if (gtk_combo_box_get_active_iter(GTK_COMBO_BOX(combobox_spec), &iter)) {
        gtk_tree_model_get(model, &iter, 0, &specialite, -1);
    }
    
    if (!specialite || strlen(specialite) == 0) {
        show_message(window, "Veuillez sélectionner une spécialité.");
        if (specialite) g_free(specialite);
        return;
    }
    
    Inscription new_inscri;
    strncpy(new_inscri.trainer_id, current_trainer_id_str, ID_LEN-1);
    new_inscri.trainer_id[ID_LEN-1] = '\0';
    strncpy(new_inscri.centre_id, centre_id, ID_LEN-1);
    new_inscri.centre_id[ID_LEN-1] = '\0';
    strncpy(new_inscri.specialite, specialite, SPECIALITE_LEN-1);
    new_inscri.specialite[SPECIALITE_LEN-1] = '\0';
    
    gboolean is_principal = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rb_principal));
    strcpy(new_inscri.role, is_principal ? "Principal" : "Assistant");
    
    new_inscri.disponible = check_dispo ? gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_dispo)) : 1;
    
    int result = ajouter_inscription(inscriptions, &nb_inscriptions, new_inscri);
    
    if (result == -2) {
        show_message(window, "Vous êtes déjà inscrit à ce centre.");
        g_free(specialite);
        return;
    } else if (result == -1) {
        show_message(window, "Erreur lors de l'ajout de l'inscription.");
        g_free(specialite);
        return;
    }
    
    sauvegarder_inscriptions(filename_inscriptions);
    show_message(window, "Inscription enregistrée avec succès!");
    
    g_free(specialite);
    gtk_widget_destroy(window);
}

void on_button_inscri_annuler_trainer_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    gtk_widget_destroy(window);
}

/* ---------- Mes Inscriptions Functions ---------- */

void on_button_voir_inscriptions_entraineur_clicked(GtkButton *button, gpointer user_data) {
    printf("DEBUG: Button clicked, current_trainer_id_str = '%s'\n", current_trainer_id_str);
    
    charger_inscriptions(filename_inscriptions);
    printf("DEBUG: Loaded %d total inscriptions\n", nb_inscriptions);
    
    // Print all inscriptions for debugging
    for (int i = 0; i < nb_inscriptions; i++) {
        printf("DEBUG: Inscription %d - trainer_id='%s', centre_id='%s', specialite='%s', role='%s'\n", 
               i, inscriptions[i].trainer_id, inscriptions[i].centre_id, 
               inscriptions[i].specialite, inscriptions[i].role);
    }
    
    GtkWidget *window_mesinscri = create_window_mesinscri_trainer();
    if (!window_mesinscri) {
        printf("DEBUG: Failed to create window_mesinscri_trainer\n");
        return;
    }
    
    // Find the treeview by searching recursively
    GtkWidget *treeview = lookup_widget(window_mesinscri, "treeview_mesinscriptions_centre_trainer");
    
    if (!treeview) {
        printf("DEBUG: First lookup failed, trying GTK object data\n");
        treeview = GTK_WIDGET(g_object_get_data(G_OBJECT(window_mesinscri), "treeview_mesinscriptions_centre_trainer"));
    }
    
    if (!treeview) {
        printf("DEBUG: Searching for ScrolledWindow or any container...\n");
        // The treeview might be inside a scrolled window
        GtkWidget *scrolled = lookup_widget(window_mesinscri, "scrolledwindow");
        if (scrolled) {
            GtkWidget *child = gtk_bin_get_child(GTK_BIN(scrolled));
            if (GTK_IS_TREE_VIEW(child)) {
                treeview = child;
                printf("DEBUG: Found treeview inside scrolled window\n");
            }
        }
    }
    
    gtk_widget_show(window_mesinscri);
    
    if (!treeview) {
        printf("DEBUG: Could not find treeview_mesinscriptions_centre_trainer\n");
        return;
    }
    
    printf("DEBUG: Setting up treeview\n");
    setup_treeview_inscriptions(GTK_TREE_VIEW(treeview));
    
    printf("DEBUG: Refreshing treeview\n");
    refresh_inscriptions_treeview(GTK_TREE_VIEW(treeview));
    
    printf("DEBUG: Done\n");
}


void on_checkbutton_mesinscri_selectall_trainer_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(togglebutton));
    GtkWidget *treeview = lookup_widget(window, "treeview_mesinscriptions_centre_trainer");
    
    if (!treeview) return;
    
    gboolean select_all = gtk_toggle_button_get_active(togglebutton);
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
    GtkTreeIter iter;
    
    if (gtk_tree_model_get_iter_first(model, &iter)) {
        do {
            gtk_list_store_set(GTK_LIST_STORE(model), &iter, 0, select_all, -1);
        } while (gtk_tree_model_iter_next(model, &iter));
    }
}

void on_button_mesincri_centre_modrole_trainer_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *window_mesinscri = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *treeview = lookup_widget(window_mesinscri, "treeview_mesinscriptions_centre_trainer");
    
    if (!treeview) return;
    
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
    GtkTreeIter iter;
    GtkTreeModel *model;
    
    if (!gtk_tree_selection_get_selected(selection, &model, &iter)) {
        show_message(window_mesinscri, "Veuillez sélectionner une inscription.");
        return;
    }
    
    gchar *centre_id, *current_role;
    gtk_tree_model_get(model, &iter, 1, &centre_id, 4, &current_role, -1);
    
    GtkWidget *window_modrole = create_window_mesinscri_mod_role_trainer();
    gtk_widget_show(window_modrole);
    
    // Store centre_id for later use
    g_object_set_data_full(G_OBJECT(window_modrole), "centre_id", g_strdup(centre_id), g_free);
    
    GtkWidget *rb_principal = lookup_widget(window_modrole, "radiobutton_mesinscri_modrole_princ_trainer");
    GtkWidget *rb_assistant = lookup_widget(window_modrole, "radiobutton_mesinscri_modrole_assis_trainer");
    
    if (rb_principal && rb_assistant) {
        if (strcmp(current_role, "Principal") == 0) {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rb_principal), TRUE);
        } else {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rb_assistant), TRUE);
        }
    }
    
    g_free(centre_id);
    g_free(current_role);
}

void on_button_mesinscri_centre_annuler_trainer_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *window_mesinscri = gtk_widget_get_toplevel(GTK_WIDGET(button));
    GtkWidget *treeview = lookup_widget(window_mesinscri, "treeview_mesinscriptions_centre_trainer");
    GtkWidget *check_selectall = lookup_widget(window_mesinscri, "checkbutton_mesinscri_selectall_trainer");
    
    if (!treeview) return;
    
    gboolean select_all = check_selectall && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_selectall));
    gboolean has_selection = FALSE;
    
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
    GtkTreeIter iter;
    
    if (select_all) {
        has_selection = TRUE;
    } else {
        // Check if any row is selected via checkboxes
        if (gtk_tree_model_get_iter_first(model, &iter)) {
            do {
                gboolean selected;
                gtk_tree_model_get(model, &iter, 0, &selected, -1);
                if (selected) {
                    has_selection = TRUE;
                    break;
                }
            } while (gtk_tree_model_iter_next(model, &iter));
        }
    }
    
    if (!has_selection) {
        show_message(window_mesinscri, "Aucune inscription sélectionnée.");
        return;
    }
    
    // Create confirmation popup dialog with Oui/Non buttons
    GtkWidget *dialog = gtk_message_dialog_new(
        GTK_WINDOW(window_mesinscri),
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_QUESTION,
        GTK_BUTTONS_NONE,  // We'll add custom buttons
        "Êtes-vous sûr de vouloir annuler les inscriptions sélectionnées ?"
    );
    
    gtk_window_set_title(GTK_WINDOW(dialog), "Confirmation d'annulation");
    
    // Add custom buttons "Non" and "Oui"
    gtk_dialog_add_button(GTK_DIALOG(dialog), "NO", GTK_RESPONSE_NO);
    gtk_dialog_add_button(GTK_DIALOG(dialog), "Yes", GTK_RESPONSE_YES);
    
    // Set "Non" as default response (so pressing Enter doesn't delete)
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_NO);
    
    // Show the dialog and get response
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    
    if (response == GTK_RESPONSE_YES) {
        int cancelled_count = 0;
        
        // First, collect all IDs to delete (to avoid iterator issues while deleting)
        typedef struct {
            char centre_id[ID_LEN];
            char trainer_id[ID_LEN];
        } InscriptionToDelete;
        
        InscriptionToDelete to_delete[MAX_INSCRIPTIONS];
        int to_delete_count = 0;
        
        if (select_all) {
            // Collect all inscriptions
            if (gtk_tree_model_get_iter_first(model, &iter)) {
                do {
                    gchar *centre_id = NULL;
                    gchar *trainer_id = NULL;
                    
                    // DEBUG: First, let's see what columns we have
                    // Assuming structure based on your original code:
                    // Column 0: checkbox (gboolean)
                    // Column 1: centre_id (gchar*)
                    // Column 2: trainer_id (gchar*)
                    // Try different column indices if needed
                    
                    gtk_tree_model_get(model, &iter, 
                                      1, &centre_id,  // Try column 1 for centre_id
                                      2, &trainer_id, // Try column 2 for trainer_id
                                      -1);
                    
                    if (centre_id && trainer_id) {
                        printf("DEBUG: Found inscription - Centre: %s, Trainer: %s\n", centre_id, trainer_id);
                        strncpy(to_delete[to_delete_count].centre_id, centre_id, ID_LEN - 1);
                        strncpy(to_delete[to_delete_count].trainer_id, trainer_id, ID_LEN - 1);
                        to_delete[to_delete_count].centre_id[ID_LEN - 1] = '\0';
                        to_delete[to_delete_count].trainer_id[ID_LEN - 1] = '\0';
                        to_delete_count++;
                    } else {
                        printf("DEBUG: centre_id or trainer_id is NULL\n");
                        if (!centre_id) printf("DEBUG: centre_id is NULL\n");
                        if (!trainer_id) printf("DEBUG: trainer_id is NULL\n");
                    }
                    
                    g_free(centre_id);
                    g_free(trainer_id);
                    
                } while (gtk_tree_model_iter_next(model, &iter) && to_delete_count < MAX_INSCRIPTIONS - 1);
            }
        } else {
            // Collect only selected inscriptions
            if (gtk_tree_model_get_iter_first(model, &iter)) {
                do {
                    gboolean selected = FALSE;
                    gchar *centre_id = NULL;
                    gchar *trainer_id = NULL;
                    
                    // Get checkbox state and IDs
                    gtk_tree_model_get(model, &iter, 
                                      0, &selected,    // Column 0: checkbox
                                      1, &centre_id,   // Column 1: centre_id
                                      2, &trainer_id,  // Column 2: trainer_id
                                      -1);
                    
                    if (selected && centre_id && trainer_id) {
                        printf("DEBUG: Selected inscription - Centre: %s, Trainer: %s\n", centre_id, trainer_id);
                        strncpy(to_delete[to_delete_count].centre_id, centre_id, ID_LEN - 1);
                        strncpy(to_delete[to_delete_count].trainer_id, trainer_id, ID_LEN - 1);
                        to_delete[to_delete_count].centre_id[ID_LEN - 1] = '\0';
                        to_delete[to_delete_count].trainer_id[ID_LEN - 1] = '\0';
                        to_delete_count++;
                    }
                    
                    g_free(centre_id);
                    g_free(trainer_id);
                    
                } while (gtk_tree_model_iter_next(model, &iter) && to_delete_count < MAX_INSCRIPTIONS - 1);
            }
        }
        
        printf("DEBUG: Total inscriptions to delete: %d\n", to_delete_count);
        
        // Now delete all collected inscriptions
        for (int i = 0; i < to_delete_count; i++) {
            printf("DEBUG: Deleting inscription %d - Centre: %s, Trainer: %s\n", 
                   i, to_delete[i].centre_id, to_delete[i].trainer_id);
            
            // Try both functions to see which one works
            // Option 1: Using annuler_inscription()
            if (annuler_inscription(to_delete[i].centre_id, to_delete[i].trainer_id)) {
                cancelled_count++;
                printf("DEBUG: Successfully deleted via annuler_inscription()\n");
            } 
            // Option 2: Using supprimer_inscription() if annuler_inscription() doesn't work
            else {
                // Try the other function from your second code snippet
                // You need to have current_trainer_id defined somewhere
                if (supprimer_inscription) { // Check if function exists
                    supprimer_inscription(inscriptions, &nb_inscriptions, current_trainer_id_str, to_delete[i].centre_id);
                    cancelled_count++;
                    printf("DEBUG: Successfully deleted via supprimer_inscription()\n");
                }
            }
        }
        
        // Save changes
        sauvegarder_inscriptions(filename_inscriptions);
        
        // Refresh the treeview
        refresh_inscriptions_treeview(GTK_TREE_VIEW(treeview));
        
        // Reset the select all checkbox if it exists
        if (check_selectall) {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_selectall), FALSE);
        }
        
        // Show success message
        char message[100];
        if (cancelled_count > 0) {
            sprintf(message, "%d inscription(s) annulée(s) avec succès.", cancelled_count);
            show_message(window_mesinscri, message);
        } else {
            show_message(window_mesinscri, "Aucune inscription annulée.");
        }
    }
}
void on_button_mesinscri_retour_trainer_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    gtk_widget_destroy(window);
}

/* ---------- Modifier Role Functions ---------- */

void on_button_mesinscri_enregistrermod_trainer_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *window_modrole = gtk_widget_get_toplevel(GTK_WIDGET(button));
    
    gchar *centre_id = g_object_get_data(G_OBJECT(window_modrole), "centre_id");
    if (!centre_id) return;
    
    GtkWidget *rb_principal = lookup_widget(window_modrole, "radiobutton_mesinscri_modrole_princ_trainer");
    if (!rb_principal) return;
    
    gboolean is_principal = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(rb_principal));
    const char *new_role = is_principal ? "Principal" : "Assistant";
    
    modifier_role_inscription(inscriptions, nb_inscriptions, current_trainer_id_str, centre_id, new_role);
    sauvegarder_inscriptions(filename_inscriptions);
    
    show_message(window_modrole, "Rôle modifié avec succès!");
    
    gtk_widget_destroy(window_modrole);
    
    // Refresh parent window
    GList *windows = gtk_window_list_toplevels();
    GList *iter = windows;
    while (iter) {
        GtkWidget *win = GTK_WIDGET(iter->data);
        GtkWidget *tv = lookup_widget(win, "treeview_mesinscriptions_centre_trainer");
        if (tv) {
            refresh_inscriptions_treeview(GTK_TREE_VIEW(tv));
            break;
        }
        iter = iter->next;
    }
    g_list_free(windows);
}

void on_button_mesinscri_modannuler_trainer_clicked(GtkButton *button, gpointer user_data) {
    GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(button));
    gtk_widget_destroy(window);
}

/* ---------- Confirmation Dialog Functions ---------- */


/* ----------  TreeView Row Activated ---------- */




void on_treeview_list_centres_admin_row_activated(GtkTreeView *treeview,
                                                  GtkTreePath *path,
                                                  GtkTreeViewColumn *column,
                                                  gpointer user_data) {
    
    // First, try to get the selected row from selection
    GtkTreeSelection *selection = gtk_tree_view_get_selection(treeview);
    GtkTreeIter iter;
    GtkTreeModel *model;
    
    // Use the provided path to get the iter directly (more reliable for row-activated)
    model = gtk_tree_view_get_model(treeview);
    if (!gtk_tree_model_get_iter(model, &iter, path)) {
        show_message(NULL, "Impossible de récupérer les données de la ligne.");
        return;
    }
    
    // Get the ID from the activated row
    gchar *id;
    gtk_tree_model_get(model, &iter, 0, &id, -1);
    
    // Find the centre in your array
    int index = -1;
    for (int i = 0; i < nb_centres; i++) {
        if (strcmp(centres[i].id, id) == 0) {
            index = i;
            break;
        }
    }
    g_free(id);
    
    if (index == -1) {
        show_message(NULL, "Centre introuvable dans la base de données.");
        return;
    }
    
    // Create and show the modification window
    GtkWidget *window_mod = create_window_modifier_centre();
    if (!window_mod) {
        g_warning("Failed to create modification window");
        return;
    }
    gtk_widget_show(window_mod);
    
    // Find all widgets in the modification window
    const char *mod_id_names[] = {"entry_mod_idcentre_admin", "entry_mod_id", "mod_id_entry", NULL};
    const char *mod_nom_names[] = {"entry_mod_nomcentre_admin", "entry_mod_nom", "mod_nom_entry", NULL};
    const char *mod_ville_names[] = {"entry_mod_villecentre_admin", "entry_mod_ville", "mod_ville_entry", NULL};
    const char *mod_type_names[] = {"entry_mod_typecentre_admin", "entry_mod_type", "mod_type_entry", NULL};
    const char *mod_cap_names[] = {"spinbutton_mod_capacitecentre_admin", "spin_mod_capacite", "mod_capacite_spin", NULL};
    
    GtkWidget *entry_id = find_widget(window_mod, mod_id_names);
    GtkWidget *entry_nom = find_widget(window_mod, mod_nom_names);
    GtkWidget *entry_ville = find_widget(window_mod, mod_ville_names);
    GtkWidget *entry_type = find_widget(window_mod, mod_type_names);
    GtkWidget *spin_cap = find_widget(window_mod, mod_cap_names);
    
    // Check if all essential widgets were found
    if (!entry_id || !entry_nom || !entry_ville || !entry_type || !spin_cap) {
        g_warning("Could not find all widgets in modify window");
        gtk_widget_destroy(window_mod);
        return;
    }
    
    // Populate the fields with centre data
    gtk_entry_set_text(GTK_ENTRY(entry_id), centres[index].id);
    gtk_entry_set_text(GTK_ENTRY(entry_nom), centres[index].nom);
    gtk_entry_set_text(GTK_ENTRY(entry_ville), centres[index].ville);
    gtk_entry_set_text(GTK_ENTRY(entry_type), centres[index].type);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_cap), centres[index].capacite);
    
    // Set radio buttons for ouvert/fermé status
    const char *ouvert_radio_names[] = {"radiobutton_mod_centreouvert_admin", "radio_mod_ouvert", "mod_ouvert_radio", NULL};
    const char *ferme_radio_names[] = {"radiobutton_mod_centreferme_admin", "radio_mod_ferme", "mod_ferme_radio", NULL};
    
    GtkWidget *rb_ouvert = find_widget(window_mod, ouvert_radio_names);
    GtkWidget *rb_ferme = find_widget(window_mod, ferme_radio_names);
    
    if (rb_ouvert && rb_ferme) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rb_ouvert), centres[index].ouvert);
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rb_ferme), !centres[index].ouvert);
    }
    
    // Set check buttons for amenities
    const char *parking_check_names[] = {"checkbutton_mod_parkingcentre_admin", "check_mod_parking", "mod_parking_check", NULL};
    const char *wifi_check_names[] = {"checkbutton_mod_wificentre_admin", "check_mod_wifi", "mod_wifi_check", NULL};
    const char *cafe_check_names[] = {"checkbutton_mod_cafeteriacentre_admin", "check_mod_cafeteria", "mod_cafeteria_check", NULL};
    
    GtkWidget *check_parking = find_widget(window_mod, parking_check_names);
    GtkWidget *check_wifi = find_widget(window_mod, wifi_check_names);
    GtkWidget *check_cafe = find_widget(window_mod, cafe_check_names);
    
    if (check_parking) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_parking), centres[index].parking);
    if (check_wifi) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_wifi), centres[index].wifi);
    if (check_cafe) gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_cafe), centres[index].cafeteria);
    
    // Make ID, name, and ville non-editable (as in your button handler)
    gtk_editable_set_editable(GTK_EDITABLE(entry_id), FALSE);
    gtk_editable_set_editable(GTK_EDITABLE(entry_nom), FALSE);
    gtk_editable_set_editable(GTK_EDITABLE(entry_ville), FALSE);
    
    // Optional: Store the index in window data for later use when saving
    g_object_set_data(G_OBJECT(window_mod), "centre_index", GINT_TO_POINTER(index));
    
    // Optional: Connect to the window's close event to clean up if needed
    // g_signal_connect(window_mod, "destroy", G_CALLBACK(on_modify_window_destroy), NULL);
}

void on_treeview_listecentres_entraineur_row_activated(GtkTreeView *treeview,
                                                       GtkTreePath *path,
                                                       GtkTreeViewColumn *column,
                                                       gpointer user_data) {
    
    // Get the window from the treeview
    GtkWidget *window_trainer = gtk_widget_get_toplevel(GTK_WIDGET(treeview));
    
    // Get the model from the treeview
    GtkTreeModel *model = gtk_tree_view_get_model(treeview);
    if (!model) {
        show_message(window_trainer, "Erreur: Modèle de données non disponible.");
        return;
    }
    
    // Get the iter for the activated row using the provided path
    GtkTreeIter iter;
    if (!gtk_tree_model_get_iter(model, &iter, path)) {
        show_message(window_trainer, "Erreur: Impossible de récupérer les données de la ligne.");
        return;
    }
    
    // Get the centre data from the row (adjust column indices as needed)
    gchar *centre_id = NULL, *centre_nom = NULL, *centre_type = NULL;
    gtk_tree_model_get(model, &iter, 
                       0, &centre_id,    // Column 0: ID
                       1, &centre_nom,   // Column 1: Nom
                       2, &centre_type,  // Column 2: Type
                       -1);
    
    // Validate data
    if (!centre_id || !centre_nom || !centre_type) {
        show_message(window_trainer, "Données du centre incomplètes.");
        g_free(centre_id);
        g_free(centre_nom);
        g_free(centre_type);
        return;
    }
    
    // Create the inscription window
    GtkWidget *window_inscri = create_window_inscri_centre_trainer();
    if (!window_inscri) {
        g_warning("Failed to create inscription window");
        g_free(centre_id);
        g_free(centre_nom);
        g_free(centre_type);
        return;
    }
    
    // Find widgets in the inscription window
    GtkWidget *entry_id = lookup_widget(window_inscri, "entry_inscri_idcentre_trainer");
    GtkWidget *entry_nom = lookup_widget(window_inscri, "entry_inscri_nomcentre_trainer");
    GtkWidget *entry_type = lookup_widget(window_inscri, "entry_inscri_type_trainer");
    GtkWidget *combobox_spec = lookup_widget(window_inscri, "combobox_inscri_specialite_trainer");
    
    // Alternative lookup for combobox if first fails
    if (!combobox_spec) {
        printf("DEBUG: First lookup failed for combobox, trying GTK object data\n");
        combobox_spec = GTK_WIDGET(g_object_get_data(G_OBJECT(window_inscri), "combobox_inscri_specialite_trainer"));
    }
    
    // Populate entry fields with centre data
    if (entry_id) {
        gtk_entry_set_text(GTK_ENTRY(entry_id), centre_id);
        gtk_editable_set_editable(GTK_EDITABLE(entry_id), FALSE);
    } else {
        printf("DEBUG: entry_inscri_idcentre_trainer not found\n");
    }
    
    if (entry_nom) {
        gtk_entry_set_text(GTK_ENTRY(entry_nom), centre_nom);
        gtk_editable_set_editable(GTK_EDITABLE(entry_nom), FALSE);
    } else {
        printf("DEBUG: entry_inscri_nomcentre_trainer not found\n");
    }
    
    if (entry_type) {
        gtk_entry_set_text(GTK_ENTRY(entry_type), centre_type);
        gtk_editable_set_editable(GTK_EDITABLE(entry_type), FALSE);
    } else {
        printf("DEBUG: entry_inscri_type_trainer not found\n");
    }
    
    // Populate combobox with specialites
    if (combobox_spec) {
        printf("DEBUG: Found combobox, populating specialites from: %s\n", filename_specialites);
        populate_combobox_specialites(combobox_spec);
        
        // Verify if specialites were loaded
        GtkTreeModel *combo_model = gtk_combo_box_get_model(GTK_COMBO_BOX(combobox_spec));
        if (combo_model) {
            int count = gtk_tree_model_iter_n_children(combo_model, NULL);
            printf("DEBUG: Combobox now has %d items\n", count);
        }
    } else {
        printf("DEBUG: combobox_inscri_specialite_trainer not found\n");
    }
    
    // Show the window
    gtk_widget_show(window_inscri);
    
    // Clean up
    g_free(centre_id);
    g_free(centre_nom);
    g_free(centre_type);
}
void on_treeview_mesinscriptions_centre_trainer_row_activated(GtkTreeView *treeview,
                                                              GtkTreePath *path,
                                                              GtkTreeViewColumn *column,
                                                              gpointer user_data) {
    
    // Get the parent window from the treeview
    GtkWidget *window_mesinscri = gtk_widget_get_toplevel(GTK_WIDGET(treeview));
    
    // Get the model from the treeview
    GtkTreeModel *model = gtk_tree_view_get_model(treeview);
    if (!model) {
        show_message(window_mesinscri, "Erreur: Modèle de données non disponible.");
        return;
    }
    
    // Get the iter for the activated row using the provided path
    GtkTreeIter iter;
    if (!gtk_tree_model_get_iter(model, &iter, path)) {
        show_message(window_mesinscri, "Erreur: Impossible de récupérer les données de la ligne.");
        return;
    }
    
    // Get the centre data from the row
    // Column indices based on your button handler:
    // Column 1: centre_id
    // Column 4: current_role
    gchar *centre_id = NULL, *current_role = NULL;
    gtk_tree_model_get(model, &iter, 
                       1, &centre_id,      // Column 1: centre_id
                       4, &current_role,   // Column 4: current_role
                       -1);
    
    // Validate data
    if (!centre_id || !current_role) {
        show_message(window_mesinscri, "Données d'inscription incomplètes.");
        g_free(centre_id);
        g_free(current_role);
        return;
    }
    
    // Create the role modification window
    GtkWidget *window_modrole = create_window_mesinscri_mod_role_trainer();
    if (!window_modrole) {
        g_warning("Failed to create role modification window");
        g_free(centre_id);
        g_free(current_role);
        return;
    }
    
    // Show the window
    gtk_widget_show(window_modrole);
    
    // Store centre_id in window data for later use (e.g., in save handler)
    g_object_set_data_full(G_OBJECT(window_modrole), "centre_id", 
                           g_strdup(centre_id), g_free);
    
    // Store the TreeIter path for potential future use
    gchar *path_str = gtk_tree_path_to_string(path);
    if (path_str) {
        g_object_set_data_full(G_OBJECT(window_modrole), "tree_path", 
                               path_str, g_free);
    }
    
    // Find and set the radio buttons based on current role
    GtkWidget *rb_principal = lookup_widget(window_modrole, "radiobutton_mesinscri_modrole_princ_trainer");
    GtkWidget *rb_assistant = lookup_widget(window_modrole, "radiobutton_mesinscri_modrole_assis_trainer");
    
    if (rb_principal && rb_assistant) {
        if (strcmp(current_role, "Principal") == 0) {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rb_principal), TRUE);
        } else {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(rb_assistant), TRUE);
        }
    } else {
        printf("DEBUG: Radio buttons not found\n");
        if (!rb_principal) printf("  - radiobutton_mesinscri_modrole_princ_trainer missing\n");
        if (!rb_assistant) printf("  - radiobutton_mesinscri_modrole_assis_trainer missing\n");
    }
    
    // Optional: Highlight the activated row in the treeview
    GtkTreeSelection *selection = gtk_tree_view_get_selection(treeview);
    gtk_tree_selection_select_path(selection, path);
    
    // Optional: Store the TreeIter reference in window data
    GtkTreeIter *iter_copy = g_new(GtkTreeIter, 1);
    *iter_copy = iter;
    g_object_set_data_full(G_OBJECT(window_modrole), "tree_iter", 
                           iter_copy, g_free);
    
    // Clean up
    g_free(centre_id);
    g_free(current_role);
    
    // Optional: Connect window signals
    // g_signal_connect(window_modrole, "destroy", G_CALLBACK(on_modrole_window_destroy), NULL);
}

void remplacer_espaces(char *str)
{
    if (str == NULL) return;
    for (int i = 0; str[i] != '\0'; i++)
    {
        if (str[i] == ' ') str[i] = '_';
    }
}

void charger_types_dans_combobox(GtkWidget *combobox, int ajouter_aucune)
{
    FILE *f;
    Equipement e;
    char types[100][30];
    int nb_types = 0, i, existe;
    
    if (combobox == NULL) return;
    
    GtkTreeModel *model = gtk_combo_box_get_model(GTK_COMBO_BOX(combobox));
    if (model != NULL) gtk_list_store_clear(GTK_LIST_STORE(model));
    
    if (ajouter_aucune) gtk_combo_box_append_text(GTK_COMBO_BOX(combobox), "Aucune");
    
    f = fopen("./src/database/equipements.txt", "r");
    if (f != NULL)
    {
        while (fscanf(f, "%s %s %s %d", e.id_equipement, e.nom_equipement, e.type, &e.nombre) != EOF)
        {
            existe = 0;
            for (i = 0; i < nb_types; i++)
            {
                if (strcmp(types[i], e.type) == 0) { existe = 1; break; }
            }
            if (!existe && nb_types < 100)
            {
                strcpy(types[nb_types], e.type);
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

void afficher_cours_entraineur(GtkWidget *liste, char *id_entraineur_param)
{
    GtkListStore *store;
    GtkTreeModel *model;
    GtkTreeIter iter;
    FILE *f_inscriptions;
    char id_entr_lu[MAX_STR], id_cours[MAX_STR];
    Cours c;
    char date_str[50], horaire_str[50];
    
    if (liste == NULL) return;
    
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(liste));
    
    if (model == NULL)
    {
        GtkCellRenderer *renderer;
        GtkTreeViewColumn *column;
        
        store = gtk_list_store_new(8, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING);
        
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
    }
    else
    {
        store = GTK_LIST_STORE(model);
        gtk_list_store_clear(store);
    }
    
    f_inscriptions = fopen("./src/database/inscriptionentraineur.txt", "r");
    if (f_inscriptions != NULL)
    {
        while (fscanf(f_inscriptions, "%s %s", id_entr_lu, id_cours) != EOF)
        {
            if (strcmp(id_entr_lu, id_entraineur_param) == 0)
            {
                c = lire_cours("./src/database/cours.txt", id_cours);
                
                if (strcmp(c.id_cours, "-1") != 0)
                {
                    sprintf(date_str, "%02d/%02d/%04d", c.jour, c.mois, c.annee);
                    sprintf(horaire_str, "%02d:00-%02d:00", c.heure_debut, c.heure_fin);
                    
                    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(liste)));
                    gtk_list_store_append(store, &iter);
                    gtk_list_store_set(store, &iter, 0, c.id_cours, 1, c.nom_cours, 2, c.type_activite, 3, date_str, 4, horaire_str, 5, get_niveau_nom(c.niveau), 6, c.places_max, 7, c.centre, -1);
                }
            }
        }
        fclose(f_inscriptions);
    }
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
        afficher_cours_entraineur(treeview, id_entraineur);
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
            if (strcmp(id_entr_lu, id_entraineur) == 0)
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

void afficher_equipements_disponibles_trainer(GtkWidget *liste)
{
    GtkListStore *store;
    GtkTreeModel *model;
    GtkTreeIter iter;
    Equipement e;
    FILE *f;
    
    if (liste == NULL) return;
    
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(liste));
    
    if (model == NULL)
    {
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
    }
    else
    {
        store = GTK_LIST_STORE(model);
        gtk_list_store_clear(store);
    }
    
    f = fopen("./src/database/equipements.txt", "r");
    if (f != NULL)
    {
        store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(liste)));
        while (fscanf(f, "%s %s %s %d", e.id_equipement, e.nom_equipement, e.type, &e.nombre) != EOF)
        {
            if (e.nombre > 0)
            {
                gtk_list_store_append(store, &iter);
                gtk_list_store_set(store, &iter, 0, e.id_equipement, 1, e.nom_equipement, 2, e.type, 3, e.nombre, -1);
            }
        }
        fclose(f);
    }
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
        if (treeview_dispo != NULL) afficher_equipements_disponibles_trainer(treeview_dispo);
        
        GtkWidget *combobox_filtre = lookup_widget(window_dispo, "combobox_filter_equipdispo_trainer");
        if (combobox_filtre != NULL) charger_types_dans_combobox(combobox_filtre, 1);
        
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
        afficher_equipements_disponibles_trainer(treeview);
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
    
    if (ajouter_reservation_equipement(id_entraineur, selected_cours_id, equip_id, equip_nom, equip_type, quantite_demandee))
    {
        afficher_equipements_disponibles_trainer(treeview);
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
    
    if (ajouter_reservation_equipement(id_entraineur, selected_cours_id, equipement_depassement_id, equipement_depassement_nom, equipement_depassement_type, quantite_finale))
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

void afficher_equipements(GtkWidget *liste)
{
    GtkListStore *store;
    GtkTreeModel *model;
    GtkTreeIter iter;
    Equipement e;
    FILE *f;
    
    if (liste == NULL) return;
    
    model = gtk_tree_view_get_model(GTK_TREE_VIEW(liste));
    
    if (model == NULL)
    {
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
    }
    else
    {
        store = GTK_LIST_STORE(model);
        gtk_list_store_clear(store);
    }
    
    f = fopen("./src/database/equipements.txt", "r");
    if (f != NULL)
    {
        store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(liste)));
        while (fscanf(f, "%s %s %s %d", e.id_equipement, e.nom_equipement, e.type, &e.nombre) != EOF)
        {
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter, 0, e.id_equipement, 1, e.nom_equipement, 2, e.type, 3, e.nombre, -1);
        }
        fclose(f);
    }
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
    if (combobox_type != NULL) charger_types_dans_combobox(combobox_type, 0);
    
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
        afficher_equipements(treeview);
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
    GtkWidget *window_ajouter, *combobox_type;
    window_ajouter = create_window_ajouter_equipement();
    combobox_type = lookup_widget(window_ajouter, "comboboxentry_ajout_typeequip_admin");
    if (combobox_type != NULL) charger_types_dans_combobox(combobox_type, 0);
    gtk_widget_show(window_ajouter);
}

void on_button_ajout_enregistrerequip_admin_clicked(GtkButton *button, gpointer user_data)
{
    GtkWidget *entry_id, *entry_nom, *combobox_type, *spinbutton_qte, *window_ajouter;
    Equipement e;
    gchar *type_text;
    
    window_ajouter = lookup_widget(GTK_WIDGET(button), "window_ajouter_equipement");
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
        if (global_treeview != NULL)
        {
            afficher_equipements(global_treeview);
            GtkWidget *toplevel = gtk_widget_get_toplevel(global_treeview);
            if (toplevel && GTK_IS_WINDOW(toplevel))
            {
                GtkWidget *combobox_filtre = lookup_widget(toplevel, "combobox_filter_equip_admin");
                if (combobox_filtre != NULL) charger_types_dans_combobox(combobox_filtre, 1);
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
        ouvrir_fenetre_modification(model, &iter);
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
        if (global_treeview != NULL)
        {
            afficher_equipements(global_treeview);
            GtkWidget *toplevel = gtk_widget_get_toplevel(global_treeview);
            if (toplevel && GTK_IS_WINDOW(toplevel))
            {
                GtkWidget *combobox_filtre = lookup_widget(toplevel, "combobox_filter_equip_admin");
                if (combobox_filtre != NULL) charger_types_dans_combobox(combobox_filtre, 1);
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
                afficher_equipements(treeview);
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
        if (treeview_reserve != NULL) afficher_equipements_reserves_trainer(treeview_reserve, id_entraineur, selected_cours_id);
        
        GtkWidget *combobox_filtre = lookup_widget(window_reserve, "combobox_filter_equipreserve_trainer");
        if (combobox_filtre != NULL) charger_types_dans_combobox(combobox_filtre, 1);
        
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
        afficher_equipements_reserves_trainer(treeview, id_entraineur, selected_cours_id);
        
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
        
        if (supprimer_toutes_reservations_cours(id_entraineur, selected_cours_id))
        {
            afficher_equipements_reserves_trainer(treeview, id_entraineur, selected_cours_id);
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
            if (supprimer_reservation_equipement(id_entraineur, selected_cours_id, selected_ids[i]))
            {
                success_count++;
            }
        }
        
        // Refresh the display
        afficher_equipements_reserves_trainer(treeview, id_entraineur, selected_cours_id);
        
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
void on_button_trier_date_clicked(GtkButton *button, gpointer user_data)
{
    // Sort courses by date in the admin interface
    GtkWidget *window = lookup_widget(GTK_WIDGET(button), "window_espace_admin");
    GtkWidget *treeview = lookup_widget(window, "treeview_liste_cours_admin");
    
    if (treeview != NULL) {
        // For now, just refresh the treeview
        // TODO: Implement actual date sorting functionality
        refresh_admin_courses_treeview(treeview);
    }
}

void on_treeview_listes_du_cours_row_activated(GtkTreeView *treeview, 
                                                 GtkTreePath *path, 
                                                 GtkTreeViewColumn *column, 
                                                 gpointer user_data)
{
    // Handle double-click on course list in member interface
    GtkTreeModel *model;
    GtkTreeIter iter;
    
    model = gtk_tree_view_get_model(treeview);
    if (gtk_tree_model_get_iter(model, &iter, path)) {
        // Get course information from the selected row
        gchar *cours_id, *cours_nom;
        gtk_tree_model_get(model, &iter, 
                          0, &cours_id,    // Column 0: ID
                          1, &cours_nom,   // Column 1: Nom
                          -1);
        
        // TODO: Show course details or registration dialog
        // For now, just show a message
        GtkWidget *window = gtk_widget_get_toplevel(GTK_WIDGET(treeview));
        GtkWidget *dialog = gtk_message_dialog_new(
            GTK_WINDOW(window),
            GTK_DIALOG_MODAL,
            GTK_MESSAGE_INFO,
            GTK_BUTTONS_OK,
            "Cours sélectionné:\nID: %s\nNom: %s",
            cours_id, cours_nom
        );
        gtk_dialog_run(GTK_DIALOG(dialog));
        gtk_widget_destroy(dialog);
        
        g_free(cours_id);
        g_free(cours_nom);
    }
}

// ===== ADMIN CALLBACKS =====

void on_button_chercher_parid_entraineur_clicked(GtkButton *button, gpointer user_data) {}

void on_button_ajouter_entraineur_admin_clicked(GtkButton *button, gpointer user_data) {}

void on_button_modifier_entraineur_admin_clicked(GtkButton *button, gpointer user_data) {}

void on_button_supprimer_entraineur_admin_clicked(GtkButton *button, gpointer user_data) {}

void on_treeview_listentraineurs_admin_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data) {}

void on_button_ajouter_membre_admmin_clicked(GtkButton *button, gpointer user_data) {}

void on_button_modifier_membre_admin_clicked(GtkButton *button, gpointer user_data) {}

void on_button_supprimer_membre_admin_clicked(GtkButton *button, gpointer user_data) {}

void on_button_chercher_membre_parid_admin_clicked(GtkButton *button, gpointer user_data) {}

void on_treeview_listemembre_admin_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data) {}













void on_button_chercher_parid_cours_admin_clicked(GtkButton *button, gpointer user_data) {}

void on_button_ajouter_cours_sportifs_admin_clicked(GtkButton *button, gpointer user_data) {}

void on_button_modifier_cours_admin_clicked(GtkButton *button, gpointer user_data) {}

void on_button_supprimer_cours_admin_clicked(GtkButton *button, gpointer user_data) {}

void on_treeview_liste_cours_admin_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data) {}

void on_button_chercher_parid_event_admin_clicked(GtkButton *button, gpointer user_data) {}

void on_button_ajouter_event_admin_clicked(GtkButton *button, gpointer user_data) {}

void on_button_modifier_event_admin_clicked(GtkButton *button, gpointer user_data) {}

void on_button_supprimer_event_admin_clicked(GtkButton *button, gpointer user_data) {}

void on_radiobutton_filtre_payant_event_admin_toggled(GtkToggleButton *togglebutton, gpointer user_data) {}

void on_radiobutton_filtre_gratuit_event_admin_toggled(GtkToggleButton *togglebutton, gpointer user_data) {}

void on_treeview_liste_event_admin_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data) {}

// ===== ENTRAINEUR CALLBACKS =====







void on_treeview_cours_dispo_entraineur_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data) {}

void on_button_sinscrirecours_entraineur_clicked(GtkButton *button, gpointer user_data) {}

void on_button_mescours_entraineur_clicked(GtkButton *button, gpointer user_data) {}

// ===== MEMBRE CALLBACKS =====

void on_treeview_listes_du_cours_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data) {}

void on_button_chercher_cours_membre_clicked(GtkButton *button, gpointer user_data) {}

void on_radiobutton_debutant_cours_membre_toggled(GtkToggleButton *togglebutton, gpointer user_data) {}

void on_radiobutton_intermidiaire_cours_membre_toggled(GtkToggleButton *togglebutton, gpointer user_data) {}

void on_button_sinscrirecours_membre_clicked(GtkButton *button, gpointer user_data) {}

void on_button_mescours_membre_clicked(GtkButton *button, gpointer user_data) {}

void on_radiobutton_avance_cours_membre_toggled(GtkToggleButton *togglebutton, gpointer user_data) {}

void on_treeview_listeentraineur_membre_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data) {}

void on_button_chercher_entraineur_membre_clicked(GtkButton *button, gpointer user_data) {}

void on_button_demander_entraineur_membre_clicked(GtkButton *button, gpointer user_data) {}

void on_button_mesentraineurs_membre_clicked(GtkButton *button, gpointer user_data) {}

void on_treeview_liste_event_membre_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data) {}

void on_button_chercher_event_membre_clicked(GtkButton *button, gpointer user_data) {}

void on_button_sinscrire_event_membre_clicked(GtkButton *button, gpointer user_data) {}

void on_button_mesinscriptions_membre_clicked(GtkButton *button, gpointer user_data) {}

void on_radiobutton_filtre_gratuit_membre_toggled(GtkToggleButton *togglebutton, gpointer user_data) {}

void on_radiobutton_filtre_payant_event_membre_toggled(GtkToggleButton *togglebutton, gpointer user_data) {}

// ===== ADD/MODIFY FORM CALLBACKS =====

void on_button_ajout_enregistrertrainer_admin_clicked(GtkButton *button, gpointer user_data) {}

void on_button_ajout_annulertrainer_admin_clicked(GtkButton *button, gpointer user_data) {}

void on_button_mod_enregistrer_admin_clicked(GtkButton *button, gpointer user_data) {}

void on_button_ajout_annulermembre_admin_clicked(GtkButton *button, gpointer user_data) {}

void on_button_ajout_enregistremembrer_admin_clicked(GtkButton *button, gpointer user_data) {}

void on_button_mod_enregistrermembre_admin_clicked(GtkButton *button, gpointer user_data) {}

void on_button_mod_annulermembre_admin_clicked(GtkButton *button, gpointer user_data) {}

void on_button_ajout_annulerequip_admin_clicked(GtkButton *button, gpointer user_data) {}

void on_button_ajout_enregistrerequip_admin_clicked(GtkButton *button, gpointer user_data) {}

void on_button_mod_annulerequip_admin_clicked(GtkButton *button, gpointer user_data) {}

void on_button_mod_enregistrerequip_admin_clicked(GtkButton *button, gpointer user_data) {}

void on_button_ajout_enregistrerevent_admin_clicked(GtkButton *button, gpointer user_data) {}

void on_button_ajout_annulerevent_admin_clicked(GtkButton *button, gpointer user_data) {}

void on_button_mod_enregistrerevent_admin_clicked(GtkButton *button, gpointer user_data) {}

void on_button_mod_annulerevent_admin_clicked(GtkButton *button, gpointer user_data) {}

void on_button_ajout_enregistrercours_admin_clicked(GtkButton *button, gpointer user_data) {}

void on_button_ajout_annulercours_admin_clicked(GtkButton *button, gpointer user_data) {}

void on_button_mod_enregistrercours_admin_clicked(GtkButton *button, gpointer user_data) {}

void on_button_mod_annulercours_admin_clicked(GtkButton *button, gpointer user_data) {}

// ===== TRAINER COURSE CALLBACKS =====

void on_button_supp_cours_trainer_clicked(GtkButton *button, gpointer user_data) {}

void on_button_mescours_chercherid_trainer_enter(GtkButton *button, gpointer user_data) {}

void on_button_mescours_retour_trainer_clicked(GtkButton *button, gpointer user_data) {}

void on_treeview_listcours_membre_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data) {}

void on_checkbutton_selectout_mescours_membre_toggled(GtkToggleButton *togglebutton, gpointer user_data) {}

void on_button_supp_cours_membre_clicked(GtkButton *button, gpointer user_data) {}

void on_button_mescours_retour_membre_clicked(GtkButton *button, gpointer user_data) {}

void on_treeview_liste_entraineur_membre_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data) {}

void on_checkbutton_selectionner_toutentraineur_membre_toggled(GtkToggleButton *togglebutton, gpointer user_data) {}

void on_button_supp_entraineur_membre_clicked(GtkButton *button, gpointer user_data) {}

void on_button_retour_mesentraineur_membre_clicked(GtkButton *button, gpointer user_data) {}

void on_treeview_listevent_membre_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data) {}

void on_checkbutton_selectall_event_membre_toggled(GtkToggleButton *togglebutton, gpointer user_data) {}

void on_button_supp_myevent_membre_clicked(GtkButton *button, gpointer user_data) {}

void on_button_mesevent_retour_membre_clicked(GtkButton *button, gpointer user_data) {}

// ===== EQUIPMENT CALLBACKS =====


// ===== MISC CALLBACKS =====

void on_button_mesinscri_centre_annuler_trainer_enter(GtkButton *button, gpointer user_data) {}
