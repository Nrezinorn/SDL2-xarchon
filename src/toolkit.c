/*--------------------------------------------------------------------------*/
/* toolkit abstraction layer                                                */
/*--------------------------------------------------------------------------*/

#include <config.h>

#include <stdlib.h>
#include <string.h>

#include "toolkit.h"

#include "gtk/gtk-toolkit.h"
#include "qt/qt-toolkit.h"

/*--------------------------------------------------------------------------*/
/* structures                                                               */
/*--------------------------------------------------------------------------*/

struct _TOOLKIT_PRIV {

   int argc;
   char **argv;

   char *current;
};

/*--------------------------------------------------------------------------*/
/* functions                                                                */
/*--------------------------------------------------------------------------*/

static void toolkit_switch(char *name);

/*--------------------------------------------------------------------------*/
/* variables                                                                */
/*--------------------------------------------------------------------------*/

TOOLKIT _toolkit;

/*--------------------------------------------------------------------------*/
/* toolkit_init                                                             */
/*--------------------------------------------------------------------------*/

void toolkit_init(int argc, char **argv)
{
   TOOLKIT *toolkit;
   TOOLKIT_PRIV *priv;

   toolkit = &_toolkit;

   priv = (TOOLKIT_PRIV *)malloc(sizeof(TOOLKIT_PRIV));

   priv->argc = argc;
   priv->argv = argv;

   priv->current = NULL;

   toolkit->priv = priv;
}

/*--------------------------------------------------------------------------*/
/* toolkit_switch                                                           */
/*--------------------------------------------------------------------------*/

void toolkit_switch(char *name)
{
   TOOLKIT *toolkit;
   TOOLKIT_PRIV *priv;
   int argc;
   
   toolkit = &_toolkit;
   priv = toolkit->priv;
   
   if (priv->current && !strcmp(name, priv->current))
      return;
   
   if (priv->current) {
      toolkit->close(toolkit->libpriv);
      toolkit->libpriv = NULL;
      free(priv->current);
   }

   argc = priv->argc;
   
   toolkit->libpriv = NULL;
#ifdef HAVE_GTK
   if (!strcmp(name, "GTK+"))
      toolkit->libpriv = gtk_toolkit_init(toolkit, &argc, priv->argv);
#endif
#ifdef HAVE_QT
   if (!strcmp(name, "Qt"))
      toolkit->libpriv = qt_toolkit_init(toolkit, &argc, priv->argv);
#endif
   if (!toolkit->libpriv)
      fprintf(stderr, "Toolkit error, or unknown toolkit name `%s\'\n", name);
   
   priv->current = strdup(name);
}

/*--------------------------------------------------------------------------*/
/* toolkit_config_read                                                      */
/*--------------------------------------------------------------------------*/

void toolkit_config_read(FILE *fp)
{
   char name[33];
   
   fscanf(fp, "%32s", name);
   toolkit_switch(name);
}

/*--------------------------------------------------------------------------*/
/* toolkit_config_write                                                     */
/*--------------------------------------------------------------------------*/

void toolkit_config_write(FILE *fp)
{
   TOOLKIT *toolkit;
   TOOLKIT_PRIV *priv;
   
   toolkit = &_toolkit;
   priv = toolkit->priv;
   
   fprintf(fp, "%s\n", priv->current);
}
