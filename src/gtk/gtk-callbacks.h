#include <gtk/gtk.h>


gboolean
main_window_expose_event               (GtkWidget       *widget,
                                        GdkEventExpose  *event,
                                        gpointer         user_data);

void
main_window_destroy_event              (GtkObject       *object,
                                        gpointer         user_data);

void
main_game_new_archon                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
main_game_new_adept                    (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
main_game_unpause                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
main_game_stop                         (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
main_game_exit                         (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
iface_define_players                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
iface_select_players                   (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
theme_select_theme                     (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
main_toggle_sound                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
main_help_tips                         (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
main_help_about                        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
iface_cancel_clicked                   (GtkObject       *object,
                                        gpointer         user_data);

void
iface_list_selection_changed           (GtkList         *list,
                                        gpointer         user_data);

void
iface_name_changed                     (GtkEditable     *editable,
                                        gpointer         user_data);

void
iface_type_toggled                     (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
iface_configure_clicked                (GtkButton       *button,
                                        gpointer         user_data);

void
iface_delete_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
iface_insert_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
iface_ok_clicked                       (GtkButton       *button,
                                        gpointer         user_data);

void
theme_cancel_clicked                   (GtkObject       *object,
                                        gpointer         user_data);

void
theme_list_selection_changed           (GtkList         *list,
                                        gpointer         user_data);

void
theme_ok_clicked                       (GtkButton       *button,
                                        gpointer         user_data);

void
computer_cancel_clicked                (GtkObject       *object,
                                        gpointer         user_data);

void
computer_skill_toggled                 (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
computer_ok_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
iface_side_list_selection_changed      (GtkList         *list,
                                        gpointer         user_data);

void
iface_first_toggled                    (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
network_cancel_clicked                 (GtkObject       *object,
                                        gpointer         user_data);

void
network_ok_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
human_cancel_clicked                   (GtkObject       *object,
                                        gpointer         user_data);

void
human_keysym_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
human_ok_clicked                       (GtkButton       *button,
                                        gpointer         user_data);
