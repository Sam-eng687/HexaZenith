#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <gtk/gtk.h>
#include "treeview.h"
// Forward declarations for structures
#define id_existe(file, id) equipement_id_existe(file, id)
#define ajouter(file, e) ajouter_equipement(file, e)
#define modifier(file, id, e) modifier_equipement(file, id, e)
#define supprimer(file, id) supprimer_equipement(file, id)

// External variables declarations
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

extern int current_membre_id;
extern int current_trainer_id;
extern char current_membre_id_str[50];
extern char current_trainer_id_str[50];

// External window pointers
extern GtkWidget *window_login;
extern GtkWidget *window_espace_admin;
extern GtkWidget *window_espace_entraineur;
extern GtkWidget *window_espace_membres;
// Authentication
int authenticate_user(const char *username, const char *password, int *user_id);
void show_message(GtkWidget *parent, const char *message);

// Login/Logout callbacks
void on_button_login_clicked(GtkWidget *button, gpointer user_data);
void on_button_logout_admin_clicked(GtkWidget *button, gpointer user_data);
void on_button_logout_entraineur_clicked(GtkWidget *button, gpointer user_data);
void on_button_logout_membre_clicked(GtkWidget *button, gpointer user_data);

// Window show callbacks
void on_window_espace_admin_show(GtkWidget *widget, gpointer user_data);
void on_window_espace_entraineur_show(GtkWidget *widget, gpointer user_data);
void on_window_espace_membres_show(GtkWidget *widget, gpointer user_data);

// Admin Centre Management
void on_button_ajouter_centre_admin_clicked(GtkButton *button, gpointer user_data);
void on_button_ajout_enregistrer_admin_clicked(GtkButton *button, gpointer user_data);
void on_button_ajout_annuler_admin_clicked(GtkButton *button, gpointer user_data);
void on_button_modifier_centre_admin_clicked(GtkButton *button, gpointer user_data);
void on_button_mod_enrregistrer_admin_clicked(GtkButton *button, gpointer user_data);
void on_button_mod_annuler_admin_clicked(GtkButton *button, gpointer user_data);
void on_button_supprimer_centre_admin_clicked(GtkButton *button, gpointer user_data);

// Admin Search/Filter/Sort
void on_button_chercher_par_idcentre_clicked(GtkButton *button, gpointer user_data);
void on_combobox_filtrer_partype_changed(GtkComboBox *combobox, gpointer user_data);
void on_spinbutton_cap_min_admin_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_checkbutton_Centres_ouverts_filtrage_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_radiobutton_tri_nom_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_radiobutton_tri_ville_toggled(GtkToggleButton *togglebutton, gpointer user_data);

// Trainer Centre Management
void on_button_chercher_centre_entraineur_clicked(GtkButton *button, gpointer user_data);
void on_combobox_filtrer_partype_entraineur_changed(GtkComboBox *combobox, gpointer user_data);
void on_spinbutton_cap_min_entraineur_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_checkbutton_Centres_ouverts_filtrage_entraineur_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_radiobutton_tri_nom_entraineur_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_radiobutton_tri_ville_entraineur_toggled(GtkToggleButton *togglebutton, gpointer user_data);

// Trainer Inscription Management
void on_button_sinscrire_entraineur_clicked(GtkButton *button, gpointer user_data);
void on_button_inscri_confirmer_trainer_clicked(GtkButton *button, gpointer user_data);
void on_button_inscri_annuler_trainer_clicked(GtkButton *button, gpointer user_data);
void on_button_voir_inscriptions_entraineur_clicked(GtkButton *button, gpointer user_data);
void on_checkbutton_mesinscri_selectall_trainer_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_button_mesincri_centre_modrole_trainer_clicked(GtkButton *button, gpointer user_data);
void on_button_mesinscri_centre_annuler_trainer_clicked(GtkButton *button, gpointer user_data);
void on_button_mesinscri_retour_trainer_clicked(GtkButton *button, gpointer user_data);
void on_button_mesinscri_enregistrermod_trainer_clicked(GtkButton *button, gpointer user_data);
void on_button_mesinscri_modannuler_trainer_clicked(GtkButton *button, gpointer user_data);

// TreeView row activation
void on_treeview_list_centres_admin_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data);
void on_treeview_listecentres_entraineur_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data);
void on_treeview_mesinscriptions_centre_trainer_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data);

// Helper function to find admin window
GtkWidget* find_admin_window(void);



// Authentication
int authenticate_user(const char *username, const char *password, int *user_id);
void show_message(GtkWidget *parent, const char *message);

// Login/Logout callbacks
void on_button_login_clicked(GtkWidget *button, gpointer user_data);
void on_button_logout_admin_clicked(GtkWidget *button, gpointer user_data);
void on_button_logout_entraineur_clicked(GtkWidget *button, gpointer user_data);
void on_button_logout_membre_clicked(GtkWidget *button, gpointer user_data);

// Window show callbacks
void on_window_espace_admin_show(GtkWidget *widget, gpointer user_data);
void on_window_espace_entraineur_show(GtkWidget *widget, gpointer user_data);
void on_window_espace_membres_show(GtkWidget *widget, gpointer user_data);

// Admin Centre Management
void on_button_ajouter_centre_admin_clicked(GtkButton *button, gpointer user_data);
void on_button_ajout_enregistrer_admin_clicked(GtkButton *button, gpointer user_data);
void on_button_ajout_annuler_admin_clicked(GtkButton *button, gpointer user_data);
void on_button_modifier_centre_admin_clicked(GtkButton *button, gpointer user_data);
void on_button_mod_enrregistrer_admin_clicked(GtkButton *button, gpointer user_data);
void on_button_mod_annuler_admin_clicked(GtkButton *button, gpointer user_data);
void on_button_supprimer_centre_admin_clicked(GtkButton *button, gpointer user_data);

// Admin Search/Filter/Sort
void on_button_chercher_par_idcentre_clicked(GtkButton *button, gpointer user_data);
void on_combobox_filtrer_partype_changed(GtkComboBox *combobox, gpointer user_data);
void on_spinbutton_cap_min_admin_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_checkbutton_Centres_ouverts_filtrage_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_radiobutton_tri_nom_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_radiobutton_tri_ville_toggled(GtkToggleButton *togglebutton, gpointer user_data);

// Trainer Centre Management
void on_button_chercher_centre_entraineur_clicked(GtkButton *button, gpointer user_data);
void on_combobox_filtrer_partype_entraineur_changed(GtkComboBox *combobox, gpointer user_data);
void on_spinbutton_cap_min_entraineur_value_changed(GtkSpinButton *spinbutton, gpointer user_data);
void on_checkbutton_Centres_ouverts_filtrage_entraineur_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_radiobutton_tri_nom_entraineur_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_radiobutton_tri_ville_entraineur_toggled(GtkToggleButton *togglebutton, gpointer user_data);

// Trainer Inscription Management
void on_button_sinscrire_entraineur_clicked(GtkButton *button, gpointer user_data);
void on_button_inscri_confirmer_trainer_clicked(GtkButton *button, gpointer user_data);
void on_button_inscri_annuler_trainer_clicked(GtkButton *button, gpointer user_data);
void on_button_voir_inscriptions_entraineur_clicked(GtkButton *button, gpointer user_data);
void on_checkbutton_mesinscri_selectall_trainer_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_button_mesincri_centre_modrole_trainer_clicked(GtkButton *button, gpointer user_data);
void on_button_mesinscri_centre_annuler_trainer_clicked(GtkButton *button, gpointer user_data);
void on_button_mesinscri_retour_trainer_clicked(GtkButton *button, gpointer user_data);
void on_button_mesinscri_enregistrermod_trainer_clicked(GtkButton *button, gpointer user_data);
void on_button_mesinscri_modannuler_trainer_clicked(GtkButton *button, gpointer user_data);

// TreeView row activation
void on_treeview_list_centres_admin_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data);
void on_treeview_listecentres_entraineur_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data);
void on_treeview_mesinscriptions_centre_trainer_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data);

// Helper function to find admin window
GtkWidget* find_admin_window(void);

// Admin - Trainer Management
void on_button_chercher_parid_entraineur_clicked(GtkButton *button, gpointer user_data);
void on_button_ajouter_entraineur_admin_clicked(GtkButton *button, gpointer user_data);
void on_button_modifier_entraineur_admin_clicked(GtkButton *button, gpointer user_data);
void on_button_supprimer_entraineur_admin_clicked(GtkButton *button, gpointer user_data);
void on_treeview_listentraineurs_admin_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data);
void on_button_ajout_enregistrertrainer_admin_clicked(GtkButton *button, gpointer user_data);
void on_button_ajout_annulertrainer_admin_clicked(GtkButton *button, gpointer user_data);
void on_button_mod_enregistrer_admin_clicked(GtkButton *button, gpointer user_data);

// Admin - Member Management
void on_button_ajouter_membre_admmin_clicked(GtkButton *button, gpointer user_data);
void on_button_modifier_membre_admin_clicked(GtkButton *button, gpointer user_data);
void on_button_supprimer_membre_admin_clicked(GtkButton *button, gpointer user_data);
void on_button_chercher_membre_parid_admin_clicked(GtkButton *button, gpointer user_data);
void on_treeview_listemembre_admin_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data);
void on_button_ajout_annulermembre_admin_clicked(GtkButton *button, gpointer user_data);
void on_button_ajout_enregistremembrer_admin_clicked(GtkButton *button, gpointer user_data);
void on_button_mod_enregistrermembre_admin_clicked(GtkButton *button, gpointer user_data);
void on_button_mod_annulermembre_admin_clicked(GtkButton *button, gpointer user_data);
void on_button_trier_date_clicked(GtkButton *button, gpointer user_data);

// Admin - Equipment Management
void on_button_chercher_equipid_admin_clicked(GtkButton *button, gpointer user_data);
void on_button_ajouter_equip_admin_clicked(GtkButton *button, gpointer user_data);
void on_button_modifier_equip_admin_clicked(GtkButton *button, gpointer user_data);
void on_button_supprimer_equip_admin_clicked(GtkButton *button, gpointer user_data);
void on_treeview_listeequipements_admin_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data);
void on_button_ajout_annulerequip_admin_clicked(GtkButton *button, gpointer user_data);
void on_button_ajout_enregistrerequip_admin_clicked(GtkButton *button, gpointer user_data);
void on_button_mod_annulerequip_admin_clicked(GtkButton *button, gpointer user_data);
void on_button_mod_enregistrerequip_admin_clicked(GtkButton *button, gpointer user_data);

// Admin - Course Management
void on_button_chercher_parid_cours_admin_clicked(GtkButton *button, gpointer user_data);
void on_button_ajouter_cours_sportifs_admin_clicked(GtkButton *button, gpointer user_data);
void on_button_modifier_cours_admin_clicked(GtkButton *button, gpointer user_data);
void on_button_supprimer_cours_admin_clicked(GtkButton *button, gpointer user_data);
void on_treeview_liste_cours_admin_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data);
void on_button_ajout_enregistrercours_admin_clicked(GtkButton *button, gpointer user_data);
void on_button_ajout_annulercours_admin_clicked(GtkButton *button, gpointer user_data);
void on_button_mod_enregistrercours_admin_clicked(GtkButton *button, gpointer user_data);
void on_button_mod_annulercours_admin_clicked(GtkButton *button, gpointer user_data);

// Admin - Event Management
void on_button_chercher_parid_event_admin_clicked(GtkButton *button, gpointer user_data);
void on_button_ajouter_event_admin_clicked(GtkButton *button, gpointer user_data);
void on_button_modifier_event_admin_clicked(GtkButton *button, gpointer user_data);
void on_button_supprimer_event_admin_clicked(GtkButton *button, gpointer user_data);
void on_radiobutton_filtre_payant_event_admin_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_radiobutton_filtre_gratuit_event_admin_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_treeview_liste_event_admin_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data);
void on_button_ajout_enregistrerevent_admin_clicked(GtkButton *button, gpointer user_data);
void on_button_ajout_annulerevent_admin_clicked(GtkButton *button, gpointer user_data);
void on_button_mod_enregistrerevent_admin_clicked(GtkButton *button, gpointer user_data);
void on_button_mod_annulerevent_admin_clicked(GtkButton *button, gpointer user_data);

// Trainer - Course Management
void on_button_chercher_mescoursid_entraineur_clicked(GtkButton *button, gpointer user_data);
void on_button_reserver_equip_entraineur_clicked(GtkButton *button, gpointer user_data);
void on_button_equip_reserve_entraineur_clicked(GtkButton *button, gpointer user_data);
void on_button_sinscrirecours_entraineur_clicked(GtkButton *button, gpointer user_data);
void on_button_mescours_entraineur_clicked(GtkButton *button, gpointer user_data);
void on_treeview_cours_dispo_entraineur_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data);
void on_button_mesinscri_centre_annuler_trainer_enter(GtkButton *button, gpointer user_data);
void on_button_supp_cours_trainer_clicked(GtkButton *button, gpointer user_data);
void on_button_mescours_chercherid_trainer_enter(GtkButton *button, gpointer user_data);
void on_button_mescours_retour_trainer_clicked(GtkButton *button, gpointer user_data);

// Member - Course Management
void on_button_chercher_cours_membre_clicked(GtkButton *button, gpointer user_data);
void on_radiobutton_debutant_cours_membre_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_radiobutton_intermidiaire_cours_membre_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_button_sinscrirecours_membre_clicked(GtkButton *button, gpointer user_data);
void on_button_mescours_membre_clicked(GtkButton *button, gpointer user_data);
void on_radiobutton_avance_cours_membre_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_treeview_listes_du_cours_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data);
void on_treeview_listcours_membre_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data);
void on_checkbutton_selectout_mescours_membre_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_button_supp_cours_membre_clicked(GtkButton *button, gpointer user_data);
void on_button_mescours_retour_membre_clicked(GtkButton *button, gpointer user_data);

// Member - Trainer Management
void on_button_chercher_entraineur_membre_clicked(GtkButton *button, gpointer user_data);
void on_button_demander_entraineur_membre_clicked(GtkButton *button, gpointer user_data);
void on_button_mesentraineurs_membre_clicked(GtkButton *button, gpointer user_data);
void on_treeview_listeentraineur_membre_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data);
void on_treeview_liste_entraineur_membre_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data);
void on_checkbutton_selectionner_toutentraineur_membre_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_button_supp_entraineur_membre_clicked(GtkButton *button, gpointer user_data);
void on_button_retour_mesentraineur_membre_clicked(GtkButton *button, gpointer user_data);

// Member - Event Management
void on_button_chercher_event_membre_clicked(GtkButton *button, gpointer user_data);
void on_button_sinscrire_event_membre_clicked(GtkButton *button, gpointer user_data);
void on_button_mesinscriptions_membre_clicked(GtkButton *button, gpointer user_data);
void on_radiobutton_filtre_gratuit_membre_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_radiobutton_filtre_payant_event_membre_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_treeview_liste_event_membre_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data);
void on_treeview_listevent_membre_row_activated(GtkTreeView *treeview, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data);
void on_checkbutton_selectall_event_membre_toggled(GtkToggleButton *togglebutton, gpointer user_data);
void on_button_supp_myevent_membre_clicked(GtkButton *button, gpointer user_data);
void on_button_mesevent_retour_membre_clicked(GtkButton *button, gpointer user_data);

// Trainer - Equipment Management
void on_button_ajout_equipreserve_trainer_clicked(GtkButton *button, gpointer user_data);
void on_button_retour_equipdispo_trainer_clicked(GtkButton *button, gpointer user_data);
void on_button_rechercheid_equipdispo_trainer_clicked(GtkButton *button, gpointer user_data);
void on_button_confirmer_nouv_nombre_trainer_clicked(GtkButton *button, gpointer user_data);
void on_button_chercher_equipreserve_trainer_clicked(GtkButton *button, gpointer user_data);
void on_button_retour_equireservee_trainer_clicked(GtkButton *button, gpointer user_data);
void on_button_supp_equipreservee_trainer_clicked(GtkButton *button, gpointer user_data);


#endif // CALLBACKS_H
