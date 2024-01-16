/*--------------------------------------------------------------------------*/
/* gtk+ toolkit:  help windows                                              */
/*--------------------------------------------------------------------------*/

#include <config.h>

#include <gtk/gtk.h>
#include "gtk-callbacks.h"
#include "gtk-interface.h"
#include "gtk-support.h"

#include <gtk-toolkit-priv.h>

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_help_tips                                                    */
/*--------------------------------------------------------------------------*/

void gtk_toolkit_help_tips(GtkMenuItem *menuitem, gpointer user_data)
{
   static GtkWidget *window = NULL;
   static int destroying = 0;

   if (window == NULL) {
      window = create_tips_window();
      gtk_widget_show(window);
   } else if (!destroying) {
      destroying = 1;
      gtk_widget_destroy(window);
      window = NULL;
      destroying = 0;
   }
}

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_help_tips                                                    */
/*--------------------------------------------------------------------------*/

void gtk_toolkit_help_tips_ok_clicked(GtkButton *button,
                                      gpointer user_data)
{
   gtk_toolkit_help_tips(NULL, NULL);
}

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_help_tips_destroy                                            */
/*--------------------------------------------------------------------------*/

void gtk_toolkit_help_tips_destroy(GtkObject *object, gpointer user_data)
{
   gtk_toolkit_help_tips(NULL, NULL);
}

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_help_about                                                   */
/*--------------------------------------------------------------------------*/

void gtk_toolkit_help_about(GtkMenuItem *menuitem, gpointer user_data)
{
   static GtkWidget *window = NULL;
   static int destroying = 0;

   if (window == NULL) {
      window = create_about_window();
      gtk_widget_show(window);
   } else if (!destroying) {
      destroying = 1;
      gtk_widget_destroy(window);
      window = NULL;
      destroying = 0;
   }
}

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_help_about_ok_clicked                                        */
/*--------------------------------------------------------------------------*/

void gtk_toolkit_help_about_ok_clicked(GtkButton *button,
                                       gpointer user_data)
{
   gtk_toolkit_help_about(NULL, NULL);
}

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_help_about_destroy                                           */
/*--------------------------------------------------------------------------*/

void gtk_toolkit_help_about_destroy(GtkObject *object, gpointer user_data)
{
   gtk_toolkit_help_about(NULL, NULL);
}
