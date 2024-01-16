/*--------------------------------------------------------------------------*/
/* gtk+ toolkit:  network configuration                                     */
/*--------------------------------------------------------------------------*/

#include <config.h>

#include <stdlib.h>
#include <string.h>

#include <gtk/gtk.h>
#include "gtk-callbacks.h"
#include "gtk-interface.h"
#include "gtk-support.h"

#include <gtk-toolkit-priv.h>
#include <network.h>

/*--------------------------------------------------------------------------*/
/* variables                                                                */
/*--------------------------------------------------------------------------*/

static NETWORK_CONFIG *orig_config;
static GtkWidget *window;

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_network_edit_config                                          */
/*--------------------------------------------------------------------------*/

void gtk_toolkit_network_edit_config(NETWORK_CONFIG *config)
{
   GtkWidget *widget;
   char port[5];

   orig_config = config;

   window = create_network_window();

   widget = lookup_widget(window, "address");
   gtk_entry_set_text(GTK_ENTRY(widget), orig_config->address);
   widget = lookup_widget(window, "port");
   sprintf(port, "%d", orig_config->port);
   gtk_entry_set_text(GTK_ENTRY(widget), port);

   gtk_widget_show(window);
}

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_network_ok_clicked                                           */
/*--------------------------------------------------------------------------*/

void gtk_toolkit_network_ok_clicked(GtkButton *button, gpointer data)
{
   GtkWidget *widget;
   char *str;
   char port[8];

   widget = lookup_widget(window, "address");
   str = gtk_editable_get_chars(GTK_EDITABLE(widget), 0, sizeof(orig_config->address) - 2);
   strcpy(orig_config->address, str);
   widget = lookup_widget(window, "port");
   str = gtk_editable_get_chars(GTK_EDITABLE(widget), 0, sizeof(port) - 2);
   orig_config->port = atoi(str);
   gtk_widget_destroy(window);
}

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_network_cancel_clicked                                       */
/*--------------------------------------------------------------------------*/

void gtk_toolkit_network_cancel_clicked(GtkObject *object, gpointer data)
{
   gtk_widget_destroy(window);
}
