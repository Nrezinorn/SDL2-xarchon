/*--------------------------------------------------------------------------*/
/* main                                                                     */
/*--------------------------------------------------------------------------*/

#include <config.h>

#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <X11/xpm.h>

#include "main.h"
#include "canvas.h"
#include "theme.h"
#include "board.h"
#include "iface.h"
#include "audio.h"
#include "field.h"
#include "toolkit.h"

#include "main-random.h"

/*--------------------------------------------------------------------------*/
/* defines                                                                  */
/*--------------------------------------------------------------------------*/

#define APP_NAME        "xarchon"

#define UPF             (1000000 / FPS)         /* usecs per frame */

#define CONFIG_VERSION  5

/*--------------------------------------------------------------------------*/
/* functions                                                                */
/*--------------------------------------------------------------------------*/

static void progress_func(char *msg, float progress);
static void init_gui(void);
static void init(int argc, char **argv);
static void play(void);
static void loop(void);

/*--------------------------------------------------------------------------*/
/* variables                                                                */
/*--------------------------------------------------------------------------*/

int random_index;

static Atom wm_protocols, wm_delete_window;

static int initializing = 1;
static int quitting = 0;

static char *private_config, *public_config;

/*--------------------------------------------------------------------------*/
/* main_random                                                              */
/*--------------------------------------------------------------------------*/

int main_random(void)
{
   int v;

   v = random_numbers[random_index % RANDOM_COUNT];
   random_index++;
   return v;
}

/*--------------------------------------------------------------------------*/
/* main_expose_event                                                        */
/*--------------------------------------------------------------------------*/

void main_expose_event(void)
{
   /* redraw canvas only if a game is in progress */
   if (!toolkit_is_toolkit_active() && board_pause_game(-1))
      canvas_refresh_all();
}

/*--------------------------------------------------------------------------*/
/* main_destroy_event                                                       */
/*--------------------------------------------------------------------------*/

void main_destroy_event(void)
{
   if (initializing)
      exit(EXIT_SUCCESS);
   quitting = 1;
   toolkit_set_toolkit_active(0);
}

/*--------------------------------------------------------------------------*/
/* main_new_game                                                            */
/*--------------------------------------------------------------------------*/

void main_new_game(int game_type)
{
   char *msg;
   int light_or_order_first;

   toolkit_set_visible_gui(0);
   msg = iface_start(&light_or_order_first);
   quitting = 0;                        /* this is important */
   if (msg != NULL) {
      toolkit_set_visible_gui(1);
      progress_func(msg, 1.0);
      return;
   }
   toolkit_set_sensitive_options(1);
   board_start_game(game_type, light_or_order_first);
   toolkit_set_toolkit_active(0);
}

/*--------------------------------------------------------------------------*/
/* main_game_unpause                                                        */
/*--------------------------------------------------------------------------*/

void main_game_unpause(void)
{
   if (board_pause_game(0))
      toolkit_set_toolkit_active(0);
}

/*--------------------------------------------------------------------------*/
/* main_game_stop                                                           */
/*--------------------------------------------------------------------------*/

void main_game_stop(void)
{
   board_end_game();
   toolkit_set_sensitive_options(0);
   progress_func("Game Ended", 1.0);
}

/*--------------------------------------------------------------------------*/
/* main_toggle_sound                                                        */
/*--------------------------------------------------------------------------*/

void main_toggle_sound(void)
{
   if (audio_isenabled())
      audio_disable();
   else
      audio_enable();
}

/*--------------------------------------------------------------------------*/
/* main_game_exit                                                           */
/*--------------------------------------------------------------------------*/

void main_game_exit(void)
{
   quitting = 1;
   toolkit_set_toolkit_active(0);
}

/*--------------------------------------------------------------------------*/
/* progress_func                                                            */
/*--------------------------------------------------------------------------*/

void progress_func(char *msg, float progress)
{
   toolkit_set_progressbar(msg, progress);
}

/*--------------------------------------------------------------------------*/
/* main_config_read                                                         */
/*--------------------------------------------------------------------------*/

void main_config_read(void)
{
   FILE *fp;
   int version;

   if (private_config == NULL) {
      if (!getenv("HOME")) {
         fprintf(stderr, "Your home directory is not known.\n"
                         "Please set the HOME environment variable to your home directory.\n");
         exit(EXIT_FAILURE);
      }
      private_config = malloc(PATH_MAX);
      sprintf(private_config, "%s/.xarchon", getenv("HOME"));
   }
   fp = fopen(private_config, "r");

   if (fp != NULL) {
      fscanf(fp, "%d", &version);
      if (version < CONFIG_VERSION) {
         fprintf(stderr, "Your configuration file was created by an older version of this program.\n"
                         "Its format is no longer readable by this version.  It will be deleted.\n");
         fclose(fp);
         unlink(private_config);
         fp = NULL;
      }
   }
   
   if (fp == NULL) {
      if (public_config == NULL) {
         public_config = malloc(PATH_MAX);
         sprintf(public_config, "%s/xarchon.default", DATADIR);
      }
      fp = fopen(public_config, "r");
      if (fp == NULL) {
         fprintf(stderr, "Could not find either `%s' or `%s'.  Giving up.\n",
                 private_config, public_config);
         exit(EXIT_FAILURE);
      }

      fscanf(fp, "%d", &version);
      if (version != CONFIG_VERSION) {
         fprintf(stderr, "Unknown version code in configuration file.  Giving up.\n");
         exit(EXIT_FAILURE);
      }
   }

   toolkit_config_read(fp);
   iface_config_read(fp);

   fclose(fp);
}

/*--------------------------------------------------------------------------*/
/* main_config_write                                                        */
/*--------------------------------------------------------------------------*/

void main_config_write(void)
{
   FILE *fp;
   
   fp = fopen(private_config, "w");
   if (fp == NULL) {
      fprintf(stderr, "Could not create `%s'.\n", private_config);
      return;
   }
   
   fprintf(fp, "%d\n", CONFIG_VERSION);
   
   toolkit_config_write(fp);
   iface_config_write(fp);
   
   fclose(fp);
}

/*--------------------------------------------------------------------------*/
/* init_gui                                                                 */
/*--------------------------------------------------------------------------*/

void init_gui(void)
{
   Display *display;
   Window window;

   toolkit_create_main_window();

   display = toolkit_get_xdisplay();
   window = toolkit_get_xwindow();
   
   wm_protocols = XInternAtom(display, "WM_PROTOCOLS", False);
   wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);

   XSelectInput(display, window,
                KeyPressMask | KeyReleaseMask |
                PointerMotionMask | ButtonPressMask | ButtonReleaseMask |
                ExposureMask | FocusChangeMask);
}

/*--------------------------------------------------------------------------*/
/* init                                                                     */
/*--------------------------------------------------------------------------*/

void init(int argc, char *argv[])
{
   char str[64];
   Display *dpy;  Window w;

   toolkit_init(argc, argv);
   main_config_read();
   init_gui();
   dpy = toolkit_get_xdisplay();
   w = toolkit_get_xwindow();
   canvas_init(dpy, w,
               CANVAS_WIDTH, CANVAS_HEIGHT);
   canvas_install_colormap(0);
   audio_init();
   theme_init(progress_func);
   srandom(time(NULL));
   random_index = random() % RANDOM_COUNT;
   initializing = 0;
   sprintf(str, "Welcome to X ARCHON, version %s", VERSION);
   progress_func(str, 1.0);
}

/*--------------------------------------------------------------------------*/
/* display_event                                                            */
/*--------------------------------------------------------------------------*/

#if DEBUG
static void display_event(XEvent *ev)
{
   static char *Event_Names[LASTEvent] = {
      NULL, /* 0 */
      NULL,
      "KeyPress",
      "KeyRelease",
      "ButtonPress",
      "ButtonRelease",
      "MotionNotify",
      "EnterNotify",
      "LeaveNotify",
      "FocusIn",
      "FocusOut",
      "KeymapNotify",
      "Expose",
      "GraphicsExpose",
      "NoExpose",
      "VisibilityExpose",
      NULL, /* 16 */
      NULL,
      "UnmapNotify",
      "MapNotify",
      "MapRequest",
      NULL, /* 21 */
      NULL,
      NULL,
      NULL,
      NULL,
      NULL,
      NULL,
      NULL,
      NULL,
      NULL,
      NULL,
      NULL,
      "ClientMessage",
      "MappingNotify"
   };
   printf("Event %s (%d) Received\n", Event_Names[ev->type], ev->type);
}
#endif

/*--------------------------------------------------------------------------*/
/* main_handle_events                                                       */
/*--------------------------------------------------------------------------*/

int main_handle_events(int in_board_game)
{
   Display *display;
   XEvent ev;
   int key;

   display = toolkit_get_xdisplay();
   while (XEventsQueued(display, QueuedAfterReading) > 0) {
      XNextEvent(display, &ev);
#ifdef DEBUG
      display_event(&ev);
#endif
      switch (ev.type) {
         case KeyPress:
            if (in_board_game) {
               key = XLookupKeysym((XKeyEvent *)&ev, 0);
               /* if Escape on game that has ended, or if F12 */
               if ((key == XK_Escape && !board_pause_game(-1)) ||
                   key == XK_F12) {
                  main_game_stop();
                  return 0;
               }
               /* if Escape on a non-network game */
               if (key == XK_Escape && iface_is_pausable()) {
                  board_pause_game(1);
                  progress_func("Game Paused", 1.0);
                  return 0;
               }
               /* if F11 */
               if (key == XK_F11)
                  main_toggle_sound();
            }
            /* fallthrough */
         case KeyRelease:
            canvas_key_event((XKeyEvent *)&ev);
            break;
         case MotionNotify:
         case ButtonPress:
         case ButtonRelease:
            canvas_mouse_event(&ev);
            break;
         case Expose:
            if (ev.xexpose.count == 0)
               canvas_refresh_all();
            break;
         case MappingNotify:
            XRefreshKeyboardMapping(&ev.xmapping);
            break;
         case FocusIn:
         case FocusOut:
            if (in_board_game && iface_is_pausable())
               board_pause_game(ev.type == FocusOut);
            break;
         case ClientMessage:
            if (ev.xclient.message_type == wm_protocols &&
                ev.xclient.data.l[0] == wm_delete_window) {
               quitting = 1;
               return 0;
            }
            break;
      }
   }
   return 1;
}

/*--------------------------------------------------------------------------*/
/* play                                                                     */
/*--------------------------------------------------------------------------*/

void play()
{
   int delay;
   struct timeval before, after;

   delay = UPF;
   gettimeofday(&before, NULL);
   while (1) {
      if (!main_handle_events(1))
         return;
      board_frame();
      /* timing logic:  at 40 fps, we need to get control every 25000us, */
      /* but the Linux timer ticks at 10ms == 10000us.  Control would be */
      /* returned only after 30000us, which is like working at 33 fps.   */
      /* Workaround:  track the actual time taken, and correct the pause */
      /* to average to the desired 25000us per frame.  */
      usleep(delay);
      gettimeofday(&after, NULL);
      /* correct the interval by the number of usecs over UPF we went */
      delay -= 1000000*(after.tv_sec - before.tv_sec) +
            after.tv_usec - before.tv_usec - UPF;
      if (delay <= 0)
         /* we've accumulated so much error that we've lost a frame */
         delay = UPF;
      before = after;
   }
}

/*--------------------------------------------------------------------------*/
/* loop                                                                     */
/*--------------------------------------------------------------------------*/

void loop(void)
{
   while (1) {
      toolkit_set_toolkit_active(1);
      if (quitting)
         return;
      toolkit_set_visible_gui(0);
      canvas_install_colormap(1);
      canvas_refresh_all();
      canvas_init_cursor(1);
      play();
      canvas_init_cursor(0);
      if (quitting)
         return;
      canvas_install_colormap(0);
      toolkit_set_visible_gui(1);
   }
}

/*--------------------------------------------------------------------------*/
/* main                                                                     */
/*--------------------------------------------------------------------------*/

#ifndef GENETICS_MAIN

int main(int argc, char *argv[])
{
   init(argc, argv);

#ifndef AUTOPILOT
   loop();

#else
   {
   int game_num = 1, i;
   int lgt, drk;
   int lgt_x, lgt_y;
   int drk_x, drk_y;
   CELL *cell;
   ACTOR *winner;

   if (0) loop();
   printf("main:   running an autopilot game.\n");
   canvas_enable(0);
   audio_enable(0);
   iface_start(&lgt);

   for (i = 0; i < 20; i++)
   for (lgt = ACTOR_LIGHT_FIRST; lgt <= ACTOR_LIGHT_LAST; lgt++)
      for (drk = ACTOR_DARK_FIRST; drk <= ACTOR_DARK_LAST; drk++) {
         board_start_game(GAME_ARCHON, lgt);
         printf("* * * * * actor %d battles %d * * * * * (%d)\n", lgt, drk, i);
         fprintf(stderr, "* * * * * actor %d battles %d * * * * * (%d)\n", lgt, drk, i);
         board_find_actor(lgt, &lgt_x, &lgt_y);
         board_find_actor(drk, &drk_x, &drk_y);
         cell = &board_cells[drk_y][drk_x];
         cell->flags &= ~CELL_LUMI_MASK;
         cell->flags |= CELL_LIGHT;
         field_start_game(board_cells[lgt_y][lgt_x].actor,
                          board_cells[drk_y][drk_x].actor,
                          cell, 0, 0);
         do
            winner = field_frame();
         while (winner == NULL);
         /*
         board_end_game();
         */
      }
   iface_config_write();
   printf("All Done\n");
   }
#endif

   return 0;
}

#endif /* GENETICS_MAIN */
