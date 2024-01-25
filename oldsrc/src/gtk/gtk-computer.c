/*--------------------------------------------------------------------------*/
/* gtk+ toolkit:  computer configuration                                    */
/*--------------------------------------------------------------------------*/

#include <config.h>

#include <string.h>

#include <gtk/gtk.h>
#include "gtk-callbacks.h"
#include "gtk-interface.h"
#include "gtk-support.h"

#include <gtk-iface.h>
#include <gtk-toolkit-priv.h>
#include <computer.h>

/*--------------------------------------------------------------------------*/
/* variables                                                                */
/*--------------------------------------------------------------------------*/

static COMPUTER_CONFIG *orig_config, work_config;

static GtkWidget *window;

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_computer_edit_config                                         */
/*--------------------------------------------------------------------------*/

void gtk_toolkit_computer_edit_config(COMPUTER_CONFIG *config)
{
   GtkWidget *widget;

   orig_config = config;

   memcpy(&work_config, orig_config, sizeof(COMPUTER_CONFIG));
   window = create_computer_window();

   widget = lookup_widget(window, work_config.rules);
   if (widget != NULL)
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), TRUE);
   if (orig_config->old_board_mode) {
      widget = lookup_widget(window, "old_board_mode");
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), TRUE);
   }

   gtk_widget_show(window);
}

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_computer_skill_toggled                                       */
/*--------------------------------------------------------------------------*/

void gtk_toolkit_computer_skill_toggled(GtkToggleButton *button,
                                        gpointer data)
{
   strcpy(work_config.rules, data);
}

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_computer_ok_clicked                                          */
/*--------------------------------------------------------------------------*/

void gtk_toolkit_computer_ok_clicked(GtkButton *button, gpointer data)
{
   GtkWidget *widget;

   memcpy(orig_config, &work_config, sizeof(COMPUTER_CONFIG));

   widget = lookup_widget(window, "old_board_mode");
   orig_config->old_board_mode =
      gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

   gtk_widget_destroy(window);
}

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_computer_cancel_clicked                                      */
/*--------------------------------------------------------------------------*/

void gtk_toolkit_computer_cancel_clicked(GtkObject *object, gpointer data)
{
   gtk_widget_destroy(window);
}
