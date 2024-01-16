/*--------------------------------------------------------------------------*/
/* gtk+ toolkit:  interface configuration                                   */
/*--------------------------------------------------------------------------*/

#include <config.h>

#include <string.h>

#include <gtk/gtk.h>
#include "gtk-callbacks.h"
#include "gtk-interface.h"
#include "gtk-support.h"

#include <gtk-iface.h>
#include <gtk-toolkit-priv.h>
#include <main.h>

/*--------------------------------------------------------------------------*/
/* functions                                                                */
/*--------------------------------------------------------------------------*/

static GtkWidget *iface_get_selection_widget(void);
static IFACE_PLAYER *iface_get_selection(void);

/*--------------------------------------------------------------------------*/
/* variables                                                                */
/*--------------------------------------------------------------------------*/

static GtkWidget *window;
static int in_selection;

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_iface_define_players                                         */
/*--------------------------------------------------------------------------*/

void gtk_toolkit_iface_define_players(GtkMenuItem *menuitem,
                                      gpointer user_data)
{
   IFACE_CONFIG *config;
   GtkWidget *list, *widget;
   IFACE_PLAYER *player;

   config = iface_get_config();

   window = create_define_window();

   list = lookup_widget(window, "list");
   for (player = list_head(&config->players);
        player != NULL;
        player = list_next(player)) {
      widget = gtk_list_item_new_with_label(player->name);
      gtk_container_add(GTK_CONTAINER(list), widget);
      gtk_widget_show(widget);
      gtk_object_set_data(GTK_OBJECT(widget), "player", player);
   }

   widget = lookup_widget(window, "config_box");
   gtk_widget_hide(widget);
   gtk_widget_show(window);
   in_selection = 0;
}

/*--------------------------------------------------------------------------*/
/* iface_get_selection_widget                                               */
/*--------------------------------------------------------------------------*/

GtkWidget *iface_get_selection_widget(void)
{
   GtkWidget *list;
   GList *item;

   list = lookup_widget(window, "list");
   item = GTK_LIST(list)->selection;
   if (item == NULL)
      return NULL;
   return GTK_WIDGET(item->data);
}

/*--------------------------------------------------------------------------*/
/* iface_get_selection                                                      */
/*--------------------------------------------------------------------------*/

IFACE_PLAYER *iface_get_selection(void)
{
   GtkWidget *widget;

   widget = iface_get_selection_widget();
   if (widget == NULL)
      return NULL;
   return gtk_object_get_data(GTK_OBJECT(widget), "player");
}

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_iface_list_selection_changed                                 */
/*--------------------------------------------------------------------------*/

void gtk_toolkit_iface_list_selection_changed(GtkList *list, gpointer data)
{
   IFACE_PLAYER *player;
   GtkWidget *widget;

   player = iface_get_selection();
   widget = lookup_widget(window, "config_box");
   if (player == NULL) {
      gtk_widget_hide(widget);
      return;
   }

   in_selection = 1;
   gtk_widget_show(widget);
   widget = lookup_widget(window, "name");
   gtk_entry_set_text(GTK_ENTRY(widget), player->name);
   widget = lookup_widget(window, "human");
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),
                                player->type == IFACE_HUMAN);
   widget = lookup_widget(window, "computer");
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),
                                player->type == IFACE_COMPUTER);
   widget = lookup_widget(window, "network");
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),
                                player->type == IFACE_NETWORK);
   in_selection = 0;
}

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_iface_name_changed                                           */
/*--------------------------------------------------------------------------*/

void gtk_toolkit_iface_name_changed(GtkEditable *editable, gpointer data)
{
   IFACE_CONFIG *config;
   IFACE_PLAYER *player;
   char *name, *ch;
   GtkWidget *widget;

   if (in_selection)
      return;

   config = iface_get_config();

   player = iface_get_selection();
   name = gtk_editable_get_chars(editable, 0, sizeof(player->name) - 2);
   for (ch = name; *ch != 0; ch++)
      *ch = (*ch == ' ') ? '-' : *ch;
   in_selection = 1;
   gtk_entry_set_text(GTK_ENTRY(editable), name);
   in_selection = 0;
   if (strcmp(config->light_name, player->name) == 0)
      strcpy(config->light_name, name);
   if (strcmp(config->dark_name, player->name) == 0)
      strcpy(config->dark_name, name);
   strcpy(player->name, name);

   widget = GTK_BIN(iface_get_selection_widget())->child;
   gtk_label_set_text(GTK_LABEL(widget), name);
}

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_iface_type_toggled                                           */
/*--------------------------------------------------------------------------*/

void gtk_toolkit_iface_type_toggled(GtkToggleButton *button, gpointer data)
{
   IFACE_PLAYER *player;

   if (in_selection)
      return;
   player = iface_get_selection();
   if (strcmp(data, "human") == 0)
      player->type = IFACE_HUMAN;
   else if (strcmp(data, "computer") == 0)
      player->type = IFACE_COMPUTER;
   else if (strcmp(data, "network") == 0)
      player->type = IFACE_NETWORK;
}

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_iface_configure_clicked                                      */
/*--------------------------------------------------------------------------*/

void gtk_toolkit_iface_configure_clicked(GtkButton *widget, gpointer data)
{
   IFACE_PLAYER *player;

   player = iface_get_selection();
   if (player->type == IFACE_HUMAN)
      gtk_toolkit_human_edit_config((HUMAN_CONFIG *)player->human);
   else if (player->type == IFACE_COMPUTER)
      gtk_toolkit_computer_edit_config((COMPUTER_CONFIG *)player->computer);
   else if (player->type == IFACE_NETWORK)
      gtk_toolkit_network_edit_config((NETWORK_CONFIG *)player->network);
}

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_iface_insert_clicked                                         */
/*--------------------------------------------------------------------------*/

void gtk_toolkit_iface_insert_clicked(GtkButton *button, gpointer data)
{
   IFACE_CONFIG *config;
   IFACE_PLAYER *player;
   GtkWidget *list, *widget;
   
   config = iface_get_config();
   
   player = iface_new_player();
   player->type = IFACE_HUMAN;
   strcpy(player->name, "New-Player");

   list = lookup_widget(window, "list");
   widget = gtk_list_item_new_with_label(player->name);
   gtk_container_add(GTK_CONTAINER(list), widget);
   gtk_widget_show(widget);
   gtk_object_set_data(GTK_OBJECT(widget), "player", player);
}

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_iface_delete_clicked                                         */
/*--------------------------------------------------------------------------*/

void gtk_toolkit_iface_delete_clicked(GtkButton *widget, gpointer data)
{
   IFACE_CONFIG *config;
   IFACE_PLAYER *player;
   GtkWidget *list;
   GList *items;
   
   config = iface_get_config();
   
   player = iface_get_selection();
   iface_delete_player(player);
   
   list = lookup_widget(window, "list");
   items = g_list_append(NULL, iface_get_selection_widget());
   gtk_list_remove_items(GTK_LIST(list), items);
   g_list_free(items);
}

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_iface_ok_clicked                                             */
/*--------------------------------------------------------------------------*/

void gtk_toolkit_iface_ok_clicked(GtkButton *widget, gpointer data)
{
   in_selection = 0;
   main_config_write();
   gtk_widget_destroy(window);
}

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_iface_cancel_clicked                                         */
/*--------------------------------------------------------------------------*/

void gtk_toolkit_iface_cancel_clicked(GtkObject *object, gpointer data)
{
   in_selection = 0;
   main_config_read();
   gtk_widget_destroy(window);
}

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_iface_select_players                                         */
/*--------------------------------------------------------------------------*/

void gtk_toolkit_iface_select_players(GtkMenuItem *menuitem,
                                      gpointer user_data)
{
   IFACE_CONFIG *config;
   static char *list_name[2] = { "light_list", "dark_list" };
   int i;
   GtkWidget *list, *widget;
   IFACE_PLAYER *player;

   config = iface_get_config();
   
   in_selection = 0;
   window = create_select_window();

   for (i = 0; i < 2; i++) {
      list = lookup_widget(window, list_name[i]);
      for (player = list_head(&config->players);
           player != NULL;
           player = list_next(player)) {
         widget = gtk_list_item_new_with_label(player->name);
         gtk_container_add(GTK_CONTAINER(list), widget);
         gtk_widget_show(widget);
         gtk_object_set_data(GTK_OBJECT(widget), "player", player);
         if ((i == 0 && strcmp(config->light_name, player->name) == 0) ||
             (i == 1 && strcmp(config->dark_name, player->name) == 0))
            gtk_list_select_child(GTK_LIST(list), widget);
      }
   }

   widget = lookup_widget(window, "light_first");
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),
                                config->light_first == 1);
   widget = lookup_widget(window, "dark_first");
   gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),
                                config->light_first == 0);

   gtk_widget_show(GTK_WIDGET(window));
   in_selection = 1;
}

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_iface_side_list_selection_changed                            */
/*--------------------------------------------------------------------------*/

void gtk_toolkit_iface_side_list_selection_changed(GtkList *list,
                                                   gpointer data)
{
   IFACE_CONFIG *config;
   IFACE_PLAYER *player;
   char *name = NULL;
   GList *item;

   config = iface_get_config();
   
   if (!in_selection)
      return;
   if (strcmp(data, "light") == 0)
      name = config->light_name;
   if (strcmp(data, "dark") == 0)
      name = config->dark_name;
   item = list->selection;
   if (item == NULL)
      strcpy(name, "?");
   else {
      player = gtk_object_get_data(GTK_OBJECT(item->data), "player");
      strcpy(name, player->name);
   }
}

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_iface_first_toggled                                          */
/*--------------------------------------------------------------------------*/

void gtk_toolkit_iface_first_toggled(GtkToggleButton *button, gpointer data)
{
   IFACE_CONFIG *config;
   
   config = iface_get_config();
   
   if (strcmp(data, "light") == 0)
      config->light_first = 1;
   else if (strcmp(data, "dark") == 0)
      config->light_first = 0;
}
