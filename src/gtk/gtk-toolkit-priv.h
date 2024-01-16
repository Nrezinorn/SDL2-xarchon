/*--------------------------------------------------------------------------*/
/* gtk+ toolkit                                                             */
/*--------------------------------------------------------------------------*/

#ifndef __MY_GTK_TOOLKIT_PRIV_H
#define __MY_GTK_TOOLKIT_PRIV_H

#include <gtk/gtk.h>

#include <gtk-toolkit.h>

/*--------------------------------------------------------------------------*/
/* structures                                                               */
/*--------------------------------------------------------------------------*/

typedef struct {

   GtkWidget *window;
   GtkWidget *menubar;
   GtkWidget *progressbar;

} PRIV;

#endif /* __MY_GTK_TOOLKIT_H_PRIV */
