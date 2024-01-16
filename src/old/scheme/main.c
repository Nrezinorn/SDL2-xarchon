/*--------------------------------------------------------------------------*/
/* main                                                                     */
/*--------------------------------------------------------------------------*/

#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>
#include <X11/xpm.h>

#include "misc.h"
#include "actors.h"
#include "canvas.h"
#include "board.h"
#include "iface.h"
#include "scheme.h"

#include "lib/misc/icon.xpm"

/*--------------------------------------------------------------------------*/
/* defines                                                                  */
/*--------------------------------------------------------------------------*/

#define APP_NAME        "xarchon"

#define UPF             (1000000 / FPS)         /* usecs per frame */
#define TIMER_RES       10000                   /* OS timer resolution */
#define UPF_MIN         (UPF - UPF % TIMER_RES)
#define UPF_MAX         (UPF + UPF % TIMER_RES)

/*--------------------------------------------------------------------------*/
/* functions                                                                */
/*--------------------------------------------------------------------------*/

static XrmDatabase init_display(int *argc, char **argv);
static void init_window(XrmDatabase database, int argc, char **argv);
static void init(int *argc, char *argv[]);
/*
static void close_app(void);
static void estae(int signum);
*/
static void handle_events(void);
static void play(int n);

/*--------------------------------------------------------------------------*/
/* variables                                                                */
/*--------------------------------------------------------------------------*/

CONFIG config = {
   1, 1,                                /* human plays light side */
   {                                    /* keys:  first light, then dark */
      { XK_space, XK_Up, XK_Down, XK_Left, XK_Right, XK_Home, XK_End, XK_Page_Up, XK_Page_Down },
      { XK_Tab,   XK_w,  XK_x,    XK_a,    XK_d,     XK_q,    XK_z,   XK_e,       XK_c         }
   }
};

Display *display = NULL;
Window window;

static Atom protocol_atom, delete_window_atom;

/*--------------------------------------------------------------------------*/
/* init_display                                                             */
/*--------------------------------------------------------------------------*/

XrmDatabase init_display(int *argc, char **argv)
{
   static XrmOptionDescRec options[] = {
      { "-display", ".display", XrmoptionSepArg, NULL },
      { "--display", ".display", XrmoptionSepArg, NULL },
      { "-version", ".version", XrmoptionNoArg, "false" },
      { "--version", ".version", XrmoptionNoArg, "false" },
      { "-geometry", ".geometry", XrmoptionSepArg, NULL },
      { "--geometry", ".geometry", XrmoptionSepArg, NULL },
      { "-synchronous", ".synchronous", XrmoptionNoArg, "false" }
   };
   XrmDatabase database;
   char *str;
   char *type;
   XrmValue value;
   
   XrmInitialize();
   database = NULL;
   XrmParseCommand(&database, options, sizeof(options) / sizeof(options[0]),
                   "xarchon", argc, argv);
   if (*argc != 1) {
      printf("X Archon:  unknown option `%s'.\n", argv[1]);
      exit(EXIT_FAILURE);
   }
   if (XrmGetResource(database, "xarchon.version", "xarchon.version", &type, &value)) {
      printf("X Archon version 1.0, by Ronen Tzur <rtzur@shani.net>.\n");
      exit(EXIT_SUCCESS);
   }

   if (!XrmGetResource(database, "xarchon.display", "xarchon.display", &type, &value))
      value.addr = getenv("DISPLAY");
   display = XOpenDisplay(value.addr);
   if (display == NULL) {
      printf("X Archon:  cannot open display `%s'\n", value.addr);
      exit(EXIT_FAILURE);
   }

   str = XScreenResourceString(XDefaultScreenOfDisplay(display));
   if (str != NULL)
      XrmCombineDatabase(XrmGetStringDatabase(str), &database, False);
   str = XResourceManagerString(display);
   if (str != NULL)
      XrmCombineDatabase(XrmGetStringDatabase(str), &database, False);
   XrmCombineDatabase(XrmGetFileDatabase("/etc/X11/app-defaults/XArchon"),
                      &database, False);

   if (XrmGetResource(database, "xarchon.synchronous", "xarchon.synchronous", &type, &value))
      XSynchronize(display, True);

   return database;
}

/*--------------------------------------------------------------------------*/
/* init_window                                                              */
/*--------------------------------------------------------------------------*/

void init_window(XrmDatabase database, int argc, char **argv)
{
   Screen *screen;
   char *type;
   XrmValue value;
   int mask;
   Pixmap pixmap;
   XpmAttributes pma;
   XSizeHints sh;
   XWMHints wmh;
   XClassHint ch;

   screen = DefaultScreenOfDisplay(display);

   sh.flags = 0;
   if (!XrmGetResource(database, "xarchon.geometry", "xarchon.geometry", &type, &value)) {
      mask = XParseGeometry(value.addr, &sh.x, &sh.y, &sh.width, &sh.height);
      if (mask & (XValue | YValue))
         sh.flags |= USPosition;
   }
   sh.flags |= PSize | PMinSize | PResizeInc;
   sh.width = sh.min_width = CANVAS_WIDTH;
   sh.height = sh.min_height = CANVAS_HEIGHT;
   sh.width_inc = 0;
   sh.height_inc = 0;

   window = XCreateSimpleWindow(display, RootWindowOfScreen(screen),
                                sh.x, sh.y, sh.width, sh.height, 1,
                                BlackPixelOfScreen(screen), BlackPixelOfScreen(screen));
   XSelectInput(display, window, KeyPressMask | KeyReleaseMask | ExposureMask | FocusChangeMask);

   pma.valuemask = XpmCloseness;
   pma.closeness = 65535;
   XpmCreatePixmapFromData(display, window, icon_xpm, &pixmap, NULL, &pma);

   wmh.flags = InputHint | StateHint | IconPixmapHint;
   wmh.input = True;
   wmh.initial_state = NormalState;
   wmh.icon_pixmap = pixmap;
   ch.res_name = NULL;
   ch.res_class = "xarchon";
   XmbSetWMProperties(display, window, "X Archon", "X Archon", argv, argc,
                      &sh, &wmh, &ch);

   protocol_atom = XInternAtom(display, "WM_PROTOCOLS", False);
   delete_window_atom = XInternAtom(display, "WM_DELETE_WINDOW", False);
   XSetWMProtocols(display, window, &delete_window_atom, 1);

   XMapWindow(display, window);
}

/*--------------------------------------------------------------------------*/
/* init                                                                     */
/*--------------------------------------------------------------------------*/

void init(int *argc, char *argv[])
{
   XrmDatabase database;

/*
   signal(SIGSEGV, estae);
   signal(SIGQUIT, estae);
   signal(SIGFPE, estae);
   signal(SIGTERM, estae);
   signal(SIGINT, estae);
*/

   printf("Initializing X interface...\n");
   database = init_display(argc, argv);
   init_window(database, *argc, argv);
   XSync(display, False);
   canvas_init(display, window, CANVAS_WIDTH, CANVAS_HEIGHT);
   printf("Loading sprites...\n");
   actors_init();
   printf("Initializing Scheme environment...\n");
   scheme_init(*argc, argv);
}

/*--------------------------------------------------------------------------*/
/* close_app                                                                */
/*--------------------------------------------------------------------------*/

/*
void close_app(void)
{
   canvas_close();
   XCloseDisplay(display);
}
*/

/*--------------------------------------------------------------------------*/
/* estae                                                                    */
/*--------------------------------------------------------------------------*/

/*
void estae(int signum)
{
   static int dying = 0;

   if (dying)
      exit(EXIT_FAILURE);
   dying = 1;
   fprintf(stderr, "Signal %d caught.\n", signum);
   close_app();
}
*/

/*--------------------------------------------------------------------------*/
/* handle_events                                                            */
/*--------------------------------------------------------------------------*/

void handle_events(void)
{
   XEvent ev;

   while (XEventsQueued(display, QueuedAfterReading) > 0) {
      XNextEvent(display, &ev);
      switch (ev.type) {
         case KeyPress:
         case KeyRelease:
            canvas_key_event((XKeyEvent *)&ev);
            break;
         case Expose:
            if (ev.xexpose.count == 0)
               board_refresh();
            break;
         case MappingNotify:
            XRefreshKeyboardMapping(&ev.xmapping);
            /* fallthrough */
         case FocusIn:
         case FocusOut:
         case UnmapNotify:
            board_pause_game(ev.type == FocusOut || ev.type == UnmapNotify);
            break;
         case ClientMessage:
            if (ev.xclient.message_type == protocol_atom &&
                ev.xclient.data.l[0] == delete_window_atom)
               exit(EXIT_SUCCESS);
            break;
      }
   }
}

/*--------------------------------------------------------------------------*/
/* play                                                                     */
/*--------------------------------------------------------------------------*/

void play(int n)
{
   struct itimerval itv;
   struct timeval tv1, tv2;
   int u_frame, count = 0;

   if (n != 0)                          /* if acting as signal handler */
      return;
   signal(SIGALRM, play);
   itv.it_interval.tv_sec = itv.it_value.tv_sec = 0;
   while (1) {

      /* timing logic:  at 40 fps, we need to get control every 25000us, */
      /* but the Linux timer ticks at 10ms == 10000us.  Control would be */
      /* returned only after 30000us, which is like working at 33 fps.   */
      /* Workaround:  alternate the pause between 20000 and 30000 usec,  */
      /* keeping an average of 25000us per frame, which is 40 fps.       */

      if (itv.it_interval.tv_usec == UPF_MIN)
         itv.it_interval.tv_usec = itv.it_value.tv_usec = UPF_MAX;
      else
         itv.it_interval.tv_usec = itv.it_value.tv_usec = UPF_MIN;
      setitimer(ITIMER_REAL, &itv, NULL);
      pause();
      gettimeofday(&tv1, NULL);
      handle_events();
      board_frame();
      gettimeofday(&tv2, NULL);
      u_frame = (tv2.tv_sec - tv1.tv_sec) * 1000000 + tv2.tv_usec - tv1.tv_usec;
      if (u_frame <= UPF)
         count++;
      else {
         printf("After %d good frames, one took %d usec (instead of %d)\n",
                count, u_frame, UPF);
         count = 0;
      }
   }
}

/*--------------------------------------------------------------------------*/
/* main                                                                     */
/*--------------------------------------------------------------------------*/

int main(int argc, char *argv[])
{
   static time_t t = 0;

   if (t == 0) {
      t = time(NULL);
      /* init() calls scheme_init(), which in turn calls main() again */
      init(&argc, argv);
      return 0;

   } else {
      printf("Total initialization took %d seconds\n", (int)(time(NULL) - t));
      iface_start(IFACE_HUMAN, "joeblow");
      board_start_game(1);
      play(0);
      return 0;
   }
}
