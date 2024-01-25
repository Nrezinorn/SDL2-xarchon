/*--------------------------------------------------------------------------*/
/* gtk+ toolkit:  main window                                               */
/*--------------------------------------------------------------------------*/

#include <config.h>

#include <gtk/gtk.h>
#include "gtk-callbacks.h"
#include "gtk-interface.h"
#include "gtk-support.h"

#include <gtk-main-window.h>
#include <gtk-toolkit-priv.h>
#include <main.h>

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_main_window_expose_event                                     */
/*--------------------------------------------------------------------------*/

gboolean gtk_toolkit_main_window_expose_event(GtkWidget *widget,
                                              GdkEventExpose  *event,
                                              gpointer user_data)
{
   main_expose_event();
   return TRUE;
}

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_main_window_destroy_event                                    */
/*--------------------------------------------------------------------------*/

void gtk_toolkit_main_window_destroy_event(GtkObject *object, gpointer data)
{
   main_destroy_event();
}

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_create_main_window                                           */
/*--------------------------------------------------------------------------*/

void gtk_toolkit_create_main_window(void *_priv)
{
   PRIV *priv;
   GtkPixmap *pixmap;

   priv = (PRIV *)_priv;

   priv->window = create_main_window();
   gtk_widget_set_usize(priv->window, CANVAS_WIDTH, CANVAS_HEIGHT);
   
   gtk_widget_show(priv->window);

   pixmap = GTK_PIXMAP(create_pixmap(priv->window, "icon.xpm"));
   gdk_window_set_icon(priv->window->window, NULL,
                       pixmap->pixmap, pixmap->mask);
   gtk_widget_destroy(GTK_WIDGET(pixmap));

   gtk_signal_connect(GTK_OBJECT(priv->window), "delete_event",
                     GTK_SIGNAL_FUNC(gtk_false), NULL);

   priv->menubar = lookup_widget(priv->window, "menubar");
   priv->progressbar = lookup_widget(priv->window, "progressbar");

   toolkit_set_sensitive_options(0);
   gtk_widget_set_sensitive(priv->menubar, FALSE);
}

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_main_game_new_archon                                         */
/*--------------------------------------------------------------------------*/

void gtk_toolkit_main_game_new_archon(GtkMenuItem *menuitem,
                                      gpointer user_data)
{
   main_new_game(GAME_ARCHON);
}

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_main_game_new_adept                                          */
/*--------------------------------------------------------------------------*/

void gtk_toolkit_main_game_new_adept(GtkMenuItem *menuitem,
                                     gpointer user_data)
{
   main_new_game(GAME_ADEPT);
}

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_main_game_unpause                                            */
/*--------------------------------------------------------------------------*/

void gtk_toolkit_main_game_unpause(GtkMenuItem *menuitem,
                                   gpointer user_data)
{
   main_game_unpause();
}

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_main_game_stop                                               */
/*--------------------------------------------------------------------------*/

void gtk_toolkit_main_game_stop(GtkMenuItem *menuitem,
                                gpointer user_data)
{
   main_game_stop();
}

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_main_game_exit                                               */
/*--------------------------------------------------------------------------*/

void gtk_toolkit_main_game_exit(GtkMenuItem *menuitem,
                                gpointer user_data)
{
   main_game_exit();
}

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_main_toggle_sound                                            */
/*--------------------------------------------------------------------------*/

void gtk_toolkit_main_toggle_sound(GtkMenuItem *menuitem,
                                   gpointer user_data)
{
   main_toggle_sound();
}
