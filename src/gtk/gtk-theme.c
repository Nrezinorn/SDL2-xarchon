/*--------------------------------------------------------------------------*/
/* gtk+ toolkit:  theme selection                                           */
/*--------------------------------------------------------------------------*/

#include <config.h>

#include <string.h>

#include <gtk/gtk.h>
#include "gtk-callbacks.h"
#include "gtk-interface.h"
#include "gtk-support.h"

#include <gtk-toolkit-priv.h>
#include <theme.h>

/*--------------------------------------------------------------------------*/
/* variables                                                                */
/*--------------------------------------------------------------------------*/

static GtkWidget *window;

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_theme_select_theme                                           */
/*--------------------------------------------------------------------------*/

void gtk_toolkit_theme_select_theme(GtkMenuItem *menuitem, gpointer user_data)
{
   GtkWidget *list, *widget;
   THEME *themes, *theme;

   window = create_theme_window();

   themes = theme_get_themes();
   list = lookup_widget(window, "list");

   for (theme = &themes[0]; theme->name != NULL; theme++) {
      widget = gtk_list_item_new_with_label(theme->name);
      gtk_container_add(GTK_CONTAINER(list), widget);
      gtk_widget_show(widget);
      gtk_object_set_data(GTK_OBJECT(widget), "theme", theme);
   }

   gtk_widget_show(GTK_WIDGET(window));
}

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_theme_list_selection_changed                                 */
/*--------------------------------------------------------------------------*/

void gtk_toolkit_theme_list_selection_changed(GtkList *list, gpointer data)
{
   GtkWidget *descr, *author, *widget;
   GList *item;
   THEME *theme;

   descr = lookup_widget(window, "descr");
   author = lookup_widget(window, "author");

   widget = lookup_widget(window, "list");
   item = GTK_LIST(list)->selection;
   if (item == NULL) {
      gtk_entry_set_text(GTK_ENTRY(descr), "");
      gtk_entry_set_text(GTK_ENTRY(author), "");
      return;
   }

   theme = gtk_object_get_data(GTK_OBJECT(item->data), "theme");
   gtk_entry_set_text(GTK_ENTRY(descr), theme->descr);
   gtk_entry_set_text(GTK_ENTRY(author), theme->author);
}

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_theme_ok_clicked                                             */
/*--------------------------------------------------------------------------*/

void gtk_toolkit_theme_ok_clicked(GtkButton *button, gpointer data)
{
   GtkWidget *widget;
   GList *item;
   THEME *theme;

   widget = lookup_widget(window, "list");
   item = GTK_LIST(widget)->selection;
   if (item != NULL)
      theme = gtk_object_get_data(GTK_OBJECT(item->data), "theme");
   else
      theme = NULL;
   gtk_widget_destroy(window);
   if (theme != NULL)
      theme_select(theme->name);
}

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_theme_cancel_clicked                                         */
/*--------------------------------------------------------------------------*/

void gtk_toolkit_theme_cancel_clicked(GtkObject *object, gpointer data)
{
   gtk_widget_destroy(window);
}
