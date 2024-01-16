/*--------------------------------------------------------------------------*/
/* gtk+ toolkit:  human configuration                                       */
/*--------------------------------------------------------------------------*/

#include <config.h>

#ifdef HAVE_LINUX_JOYSTICK_H
#include <fcntl.h>
#include <math.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/joystick.h>
#endif

#include <string.h>

#include <gtk/gtk.h>
#include "gtk-callbacks.h"
#include "gtk-interface.h"
#include "gtk-support.h"

#include <gtk-iface.h>
#include <gtk-toolkit-priv.h>
#include <human.h>
#include <canvas.h>

/*--------------------------------------------------------------------------*/
/* variables                                                                */
/*--------------------------------------------------------------------------*/

static HUMAN_CONFIG *orig_config, work_config;

static GtkWidget *window;

static int snooper_id = -1;
static char *keysym_clicked = NULL;

static int side_i;

static char *button_name[] = {
   "fire_key_button",
   "up_key_button",
   "down_key_button",
   "left_key_button",
   "right_key_button",
   "up_left_key_button",
   "down_left_key_button",
   "up_right_key_button",
   "down_right_key_button"
};

static char *direction_name[] = {
   "Fire",
   "Up",
   "Down",
   "Left",
   "Right",
   "Up Left",
   "Down Left",
   "Up Right",
   "Down Right"
};

/*--------------------------------------------------------------------------*/
/* functions                                                                */
/*--------------------------------------------------------------------------*/

static void human_init_optionmenu(char *menu_name, GtkSignalFunc activate);
static int human_get_active_option(char *menu_name);
static void human_refresh_keysyms(GtkMenuItem *item, gpointer data);
static gint human_snooper(GtkWidget *widget,
                          GdkEventKey *event, gpointer data);

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_human_edit_config                                            */
/*--------------------------------------------------------------------------*/

void gtk_toolkit_human_edit_config(HUMAN_CONFIG *config)
{
   GtkWidget *widget;

   orig_config = config;

   memcpy(&work_config, orig_config, sizeof(HUMAN_CONFIG));
   window = create_human_window();

   human_init_optionmenu("side", human_refresh_keysyms);
   snooper_id = gtk_key_snooper_install(human_snooper, NULL);

   human_init_optionmenu("non_keyboard", NULL);
   widget = lookup_widget(window, "non_keyboard");
   gtk_option_menu_set_history(GTK_OPTION_MENU(widget),
                               work_config.non_keyboard + 1);

   gtk_widget_show(window);
   human_refresh_keysyms(NULL, 0);
}

/*--------------------------------------------------------------------------*/
/* human_init_optionmenu                                                    */
/*--------------------------------------------------------------------------*/

void human_init_optionmenu(char *menu_name, GtkSignalFunc activate)
{
   GtkWidget *widget;
   GList *items, *item;
   int i;

   widget = lookup_widget(window, menu_name);
   widget = gtk_option_menu_get_menu(GTK_OPTION_MENU(widget));
   items = gtk_container_children(GTK_CONTAINER(widget));
   i = 0;
   for (item = items; item != NULL; item = item->next) {
      gtk_object_set_data(GTK_OBJECT(item->data), "i", (gpointer)i);
      if (activate != NULL)
         gtk_signal_connect(GTK_OBJECT(item->data), "activate", activate, NULL);
      i++;
   }
   g_list_free(items);
}

/*--------------------------------------------------------------------------*/
/* human_get_active_option                                                  */
/*--------------------------------------------------------------------------*/

int human_get_active_option(char *menu_name)
{
   GtkWidget *widget;

   widget = lookup_widget(window, menu_name);
   widget = gtk_option_menu_get_menu(GTK_OPTION_MENU(widget));
   widget = gtk_menu_get_active(GTK_MENU(widget));
   return (int)gtk_object_get_data(GTK_OBJECT(widget), "i");
}
   
/*--------------------------------------------------------------------------*/
/* human_refresh_keysyms                                                    */
/*--------------------------------------------------------------------------*/

void human_refresh_keysyms(GtkMenuItem *item, gpointer data)
{
   GtkLabel *keysym;
   int direction;
   KeySym k;
   char label[64];

   side_i = human_get_active_option("side");
   for (direction = STATE_FIRE; direction <= STATE_MOVE_LAST; direction++) {
      keysym = GTK_LABEL(GTK_BIN(lookup_widget(window, button_name[direction]))->child);
      if (side_i == 2 &&
            work_config.keys[0][direction] != work_config.keys[1][direction])
         sprintf(label, "%s: <different>", direction_name[direction]);
      else {
         k = work_config.keys[side_i % 2][direction];
         sprintf(label, "%s: %s (%lX)", direction_name[direction],
               canvas_keysym_name(k), k);
      }
      gtk_label_set_text(keysym, label);
   }
}

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_human_keysym_clicked                                         */
/*--------------------------------------------------------------------------*/

void gtk_toolkit_human_keysym_clicked(GtkButton *button, gpointer data)
{
   if (keysym_clicked != NULL)
      return;
   gtk_label_set_text(GTK_LABEL(GTK_BIN(button)->child), "Press Key Now");
   keysym_clicked = (char *) data;
}

/*--------------------------------------------------------------------------*/
/* human_snooper                                                            */
/*--------------------------------------------------------------------------*/

gint human_snooper(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
   int direction;
   int key;

   if (!keysym_clicked)
      return FALSE;
   switch (keysym_clicked[0]) {
      case 'u':
         switch (keysym_clicked[1]) {
            case 'l':
               direction = STATE_MOVE_UP_LEFT;
               break;
            case 'r':
               direction = STATE_MOVE_UP_RIGHT;
               break;
            case '\0':
               direction = STATE_MOVE_UP;
               break;
            default:
               return FALSE;
         }
         break;
      case 'd':
         switch (keysym_clicked[1]) {
            case 'l':
               direction = STATE_MOVE_DOWN_LEFT;
               break;
            case 'r':
               direction = STATE_MOVE_DOWN_RIGHT;
               break;
            case '\0':
               direction = STATE_MOVE_DOWN;
               break;
            default:
               return FALSE;
         }
         break;
      case 'r':
         direction = STATE_MOVE_RIGHT;
         break;
      case 'l':
         direction = STATE_MOVE_LEFT;
         break;
      case 'f':
         direction = STATE_FIRE;
         break;
      default:
         return FALSE;
   }
   keysym_clicked = NULL;
   if (side_i == 0 || side_i == 2) {
      for (key = 0; key < STATE_MOVE_COUNT; key++)
         if (work_config.keys[0][key] == event->keyval)
            work_config.keys[0][key] = 0;
      work_config.keys[0][direction] = event->keyval;
   }
   if (side_i == 1 || side_i == 2) {
      for (key = 0; key < STATE_MOVE_COUNT; key++)
         if (work_config.keys[1][key] == event->keyval)
            work_config.keys[1][key] = 0;
      work_config.keys[1][direction] = event->keyval;
   }
   human_refresh_keysyms(NULL, 0);
   return TRUE;
}

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_human_ok_clicked                                             */
/*--------------------------------------------------------------------------*/

void gtk_toolkit_human_ok_clicked(GtkButton *button, gpointer data)
{
   work_config.non_keyboard = human_get_active_option("non_keyboard") - 1;
   if (work_config.non_keyboard != -1 &&
       work_config.non_keyboard < HUMAN_JOY_MAX_DEV) {
#ifdef HAVE_LINUX_JOYSTICK_H
      if (!human_joystick_init(work_config.non_keyboard)) {
#endif
         toolkit_message_box("Joystick not available");
         return;
#ifdef HAVE_LINUX_JOYSTICK_H
      }
#endif
   }

   memcpy(orig_config, &work_config, sizeof(HUMAN_CONFIG));
   gtk_key_snooper_remove(snooper_id);
   snooper_id = -1;
   gtk_widget_destroy(window);
}

/*--------------------------------------------------------------------------*/
/* gtk_toolkit_human_cancel_clicked                                         */
/*--------------------------------------------------------------------------*/

void gtk_toolkit_human_cancel_clicked(GtkObject *button, gpointer data)
{
   if (snooper_id != -1) {
      gtk_key_snooper_remove(snooper_id);
      snooper_id = -1;
   }
   gtk_widget_destroy(window);
}
