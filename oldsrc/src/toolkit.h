/*--------------------------------------------------------------------------*/
/* toolkit abstraction layer                                                */
/*--------------------------------------------------------------------------*/

#ifndef __MY_TOOLKIT_H
#define __MY_TOOLKIT_H

#include <stdio.h>
#include <X11/Xlib.h>

/*--------------------------------------------------------------------------*/
/* macros                                                                   */
/*--------------------------------------------------------------------------*/

#define toolkit_create_main_window() \
   (_toolkit.create_main_window(_toolkit.libpriv))

#define toolkit_get_xdisplay() \
   (_toolkit.get_xdisplay(_toolkit.libpriv))

#define toolkit_get_xwindow() \
   (_toolkit.get_xwindow(_toolkit.libpriv))

#define toolkit_is_toolkit_active() \
   (_toolkit.is_toolkit_active(_toolkit.libpriv))

#define toolkit_message_box(msg) \
   (_toolkit.message_box(_toolkit.libpriv, msg))

#define toolkit_set_progressbar(msg,progress) \
   (_toolkit.set_progressbar(_toolkit.libpriv, msg, progress))

#define toolkit_set_sensitive_options(game_active) \
   (_toolkit.set_sensitive_options(_toolkit.libpriv, game_active))

#define toolkit_set_visible_gui(visible) \
   (_toolkit.set_visible_gui(_toolkit.libpriv, visible))

#define toolkit_set_toolkit_active(active) \
   (_toolkit.set_toolkit_active(_toolkit.libpriv, active))

#define toolkit_xflush() \
   (_toolkit.xflush(_toolkit.libpriv))

/*--------------------------------------------------------------------------*/
/* types                                                                    */
/*--------------------------------------------------------------------------*/

typedef struct _TOOLKIT TOOLKIT;

typedef struct _TOOLKIT_PRIV TOOLKIT_PRIV;

typedef void *(*toolkit_init_func)(TOOLKIT *toolkit);
typedef void (*toolkit_close_func)(void *priv);

/*--------------------------------------------------------------------------*/
/* structures                                                               */
/*--------------------------------------------------------------------------*/

struct _TOOLKIT {

   toolkit_close_func close;

   void         (*create_main_window)(void *priv);
   Display     *(*get_xdisplay)(void *priv);
   Window       (*get_xwindow)(void *priv);
   int          (*is_toolkit_active)(void *priv);
   void         (*message_box)(void *priv, char *msg);
   void         (*set_progressbar)(void *priv, char *msg, float progress);
   void         (*set_sensitive_options)(void *priv, int game_active);
   void         (*set_toolkit_active)(void *priv, int active);
   void         (*set_visible_gui)(void *priv, int visible);
   void         (*xflush)(void *priv);

   TOOLKIT_PRIV *priv;
   void *libpriv;
};

/*--------------------------------------------------------------------------*/
/* functions                                                                */
/*--------------------------------------------------------------------------*/

void toolkit_init(int argc, char **argv);
void toolkit_config_read(FILE *fp);
void toolkit_config_write(FILE *fp);

#if 0

void toolkit_create_main_window(void);
Display *toolkit_get_xdisplay(void);
Window toolkit_get_xwindow(void);
int toolkit_is_toolkit_active(void);
void toolkit_message_box(char *msg);
void toolkit_set_progressbar(char *msg, float progress);
void toolkit_set_sensitive_options(int game_active);
void toolkit_set_toolkit_active(int active);
void toolkit_set_visible_gui(int visible);
void toolkit_xflush(void);

#endif

/*--------------------------------------------------------------------------*/
/* variables                                                                */
/*--------------------------------------------------------------------------*/

extern TOOLKIT _toolkit;

#endif /* __MY_TOOLKIT_H */
