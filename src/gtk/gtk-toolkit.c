/*--------------------------------------------------------------------------*/
/* gtk+ toolkit                                                             */
/*--------------------------------------------------------------------------*/

#include <config.h>

#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include "gtk-callbacks.h"
#include "gtk-interface.h"
#include "gtk-support.h"

#include <gtk-toolkit-priv.h>
#include <gtk-main-window.h>
#include <toolkit.h>

/*--------------------------------------------------------------------------*/
/* functions                                                                */
/*--------------------------------------------------------------------------*/

static void gtk_toolkit_close(void *_priv);
static int gtk_toolkit_is_toolkit_active(void *_priv);
static Display *gtk_toolkit_get_xdisplay(void *_priv);
static Window gtk_toolkit_get_xwindow(void *_priv);
void gtk_toolkit_message_box(void *_priv, char *msg);
static void gtk_toolkit_set_progressbar(void *_priv,
                                        char *msg, float progress);
static void gtk_toolkit_set_sensitive_options(void *_priv, int game_active);
static void gtk_toolkit_set_toolkit_active(void *_priv, int active);
static void gtk_toolkit_set_visible_gui(void *_priv, int visible);
static void gtk_toolkit_xflush(void *_priv);

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_init                                                         */
/*--------------------------------------------------------------------------*/

void *gtk_toolkit_init(TOOLKIT *toolkit, int *argc, char **argv)
{
   PRIV *priv;
   
   gtk_init(argc, &argv);
   add_pixmap_directory(DATADIR);       /* a glade thingy */
   
   toolkit->close = gtk_toolkit_close;

   toolkit->create_main_window    = gtk_toolkit_create_main_window;
   toolkit->get_xdisplay          = gtk_toolkit_get_xdisplay;
   toolkit->get_xwindow           = gtk_toolkit_get_xwindow;
   toolkit->is_toolkit_active     = gtk_toolkit_is_toolkit_active;
   toolkit->message_box           = gtk_toolkit_message_box;
   toolkit->set_progressbar       = gtk_toolkit_set_progressbar;
   toolkit->set_sensitive_options = gtk_toolkit_set_sensitive_options;
   toolkit->set_toolkit_active    = gtk_toolkit_set_toolkit_active;
   toolkit->set_visible_gui       = gtk_toolkit_set_visible_gui;
   toolkit->xflush                = gtk_toolkit_xflush;
   
   priv = g_malloc(sizeof(PRIV));
   
   return priv;
}

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_close                                                        */
/*--------------------------------------------------------------------------*/

static void gtk_toolkit_close(void *_priv)
{
   g_free(_priv);
}

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_is_toolkit_active                                            */
/*--------------------------------------------------------------------------*/

static int gtk_toolkit_is_toolkit_active(void *_priv)
{
   return gtk_main_level();
}

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_get_xdisplay                                                 */
/*--------------------------------------------------------------------------*/

static Display *gtk_toolkit_get_xdisplay(void *_priv)
{
   return GDK_DISPLAY();
}

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_get_xwindow                                                  */
/*--------------------------------------------------------------------------*/

static Window gtk_toolkit_get_xwindow(void *_priv)
{
   PRIV *priv;
   
   priv = (PRIV *)_priv;
   return GDK_WINDOW_XWINDOW(priv->window->window);
}

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_message_box                                                  */
/*--------------------------------------------------------------------------*/

void gtk_toolkit_message_box(void *_priv, char *msg)
{
   GtkWidget *dialog, *label, *okay_button;

   dialog = gtk_dialog_new();
   label = gtk_label_new(msg);
   okay_button = gtk_button_new_with_label("OK");
   gtk_signal_connect_object(GTK_OBJECT(okay_button), "clicked",
                             GTK_SIGNAL_FUNC(gtk_widget_destroy),
                             GTK_OBJECT(dialog));
   gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->action_area),
                     okay_button);
   gtk_container_add(GTK_CONTAINER(GTK_DIALOG(dialog)->vbox), label);
   gtk_window_set_modal(GTK_WINDOW(dialog), 1);
   gtk_widget_show_all(dialog);
}

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_set_progressbar                                              */
/*--------------------------------------------------------------------------*/

static void gtk_toolkit_set_progressbar(void *_priv,
                                        char *msg, float progress)
{
   PRIV *priv;

   priv = (PRIV *)_priv;

   if (progress == 0.0)
      gtk_widget_set_sensitive(priv->menubar, FALSE);
   if (progress == 1.0)
      gtk_widget_set_sensitive(priv->menubar, TRUE);

   gtk_progress_set_format_string(GTK_PROGRESS(priv->progressbar), msg);
   gtk_progress_bar_update(GTK_PROGRESS_BAR(priv->progressbar), progress);
 
   if (gtk_main_level() == 0) {
      while (gtk_events_pending())
         gtk_main_iteration();
      toolkit_xflush();
   }
}

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_set_sensitive_options                                        */
/*--------------------------------------------------------------------------*/

static void gtk_toolkit_set_sensitive_options(void *_priv, int game_active)
{
   PRIV *priv;
   
   priv = (PRIV *)_priv;
   
   gtk_widget_set_sensitive(
      lookup_widget(priv->window, "mnu_game_new_archon"), !game_active);
   gtk_widget_set_sensitive(
      lookup_widget(priv->window, "mnu_game_new_adept"),  !game_active);
   gtk_widget_set_sensitive(
      lookup_widget(priv->window, "mnu_game_stop"),        game_active);
   gtk_widget_set_sensitive(
      lookup_widget(priv->window, "mnu_settings_define"), !game_active);
   gtk_widget_set_sensitive(
      lookup_widget(priv->window, "mnu_settings_select"), !game_active);
   gtk_widget_set_sensitive(
      lookup_widget(priv->window, "mnu_settings_theme"),  !game_active);
}

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_set_toolkit_active                                           */
/*--------------------------------------------------------------------------*/

static void gtk_toolkit_set_toolkit_active(void *_priv, int active)
{
   if (active)
      gtk_main();
   else
      gtk_main_quit();
}

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_set_visible_gui                                              */
/*--------------------------------------------------------------------------*/

static void gtk_toolkit_set_visible_gui(void *_priv, int visible)
{
   PRIV *priv;
   
   priv = (PRIV *)_priv;

   if (visible) {
      gtk_widget_show(priv->menubar);
      gtk_widget_show(lookup_widget(priv->window, "logo"));
      gtk_widget_show(priv->progressbar);
   } else {
      gtk_widget_hide(priv->menubar);
      gtk_widget_hide(lookup_widget(priv->window, "logo"));
      gtk_widget_hide(priv->progressbar);
   }
   gdk_flush();
}

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_xflush                                                       */
/*--------------------------------------------------------------------------*/

static void gtk_toolkit_xflush(void *_priv)
{
   gdk_flush();
}
