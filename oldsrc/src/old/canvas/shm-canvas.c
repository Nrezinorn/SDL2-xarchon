/*--------------------------------------------------------------------------*/
/* canvas                                                                   */
/*--------------------------------------------------------------------------*/

/* choose your double buffering technique: */
#define USE_XIMAGE 0                    /* use XImage (w/ XShm) */
#define USE_PIXMAP 1                    /* use double buffering */
/* define both 0 for no double buffering */

/* note:  the following aren't implemented for USE_XIMAGE: */
/*        canvas_font_*()                                  */
/*        clipping with canvas_image_paint()               */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/keysymdef.h>
#include <X11/xpm.h>

#if USE_XIMAGE == 1
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#endif

#include "canvas.h"

/*--------------------------------------------------------------------------*/
/* variables                                                                */
/*--------------------------------------------------------------------------*/

static Display *display;
static Window window;
static GC gc;
static char *keys_down = NULL;

Colormap canvas_colormap;
long canvas_black_pixel;

#if USE_XIMAGE == 1
static XImage *canvas = NULL;
static XShmSegmentInfo shminfo = { };

#elif USE_PIXMAP == 1
static Pixmap canvas = 0;
static int canvas_width, canvas_height;
#define MAX_CLIP_RECTS 512
static XRectangle *clip_rects;
static int num_clip_rects;

#else /* no double buffering */
#define canvas window

#endif /* USE_... */

/*--------------------------------------------------------------------------*/
/* canvas_init                                                              */
/*--------------------------------------------------------------------------*/

void canvas_init(Display *_display, Window _window, int width, int height)
{
   Screen *screen;
   XColor xc;

   display = _display;
   window = _window;
   gc = XCreateGC(display, window, 0, NULL);
   XSetGraphicsExposures(display, gc, False);
   keys_down = calloc(sizeof(char), 512);

   screen = DefaultScreenOfDisplay(display);
   canvas_colormap = XCreateColormap(display, window,
                                     DefaultVisualOfScreen(screen), AllocNone);
   XAllocNamedColor(display, canvas_colormap, "#000000", &xc, &xc);
   canvas_black_pixel = xc.pixel;
   XSetWindowColormap(display, window, canvas_colormap);
   XInstallColormap(display, canvas_colormap);
   
#if USE_XIMAGE == 1
   canvas = XShmCreateImage(display, DefaultVisualOfScreen(screen),
                         DefaultDepthOfScreen(screen), ZPixmap, NULL,
                         &shminfo, width, height);
   shminfo.shmid = shmget(IPC_PRIVATE, width * height, IPC_CREAT | 0600);
   shminfo.shmaddr = canvas->data = shmat(shminfo.shmid, 0, 0);
   shminfo.readOnly = 0;
   XShmAttach(display, &shminfo);
   atexit(canvas_close);

#elif USE_PIXMAP == 1
   canvas = XCreatePixmap(display, window, width, height,
                          DefaultDepthOfScreen(screen));
   canvas_width = width;
   canvas_height = height;
   clip_rects = malloc(sizeof(XRectangle) * MAX_CLIP_RECTS);
   num_clip_rects = 0;

#endif /* USE_... */
   canvas_clear();
}

/*--------------------------------------------------------------------------*/
/* canvas_close                                                             */
/*--------------------------------------------------------------------------*/

void canvas_close(void)
{
#if USE_XIMAGE == 1
   if (shminfo.shmid != 0) {
      XShmDetach(display, &shminfo);
      XDestroyImage(canvas);
      shmctl(shminfo.shmid, IPC_RMID, NULL);
      shmdt(shminfo.shmaddr);
      shminfo.shmid = 0;
   }
#endif
}

/*--------------------------------------------------------------------------*/
/* canvas_dirty                                                             */
/*--------------------------------------------------------------------------*/

#if USE_PIXMAP == 1
void canvas_dirty(int x, int y, int width, int height)
{
   if (num_clip_rects >= MAX_CLIP_RECTS) {
      printf("canvas_dirty():  too many dirty rectangles.\n");
      exit(EXIT_FAILURE);
   }
   clip_rects[num_clip_rects].x = x;
   clip_rects[num_clip_rects].y = y;
   clip_rects[num_clip_rects].width = width;
   clip_rects[num_clip_rects].height = height;
   num_clip_rects++;
}
#endif

/*--------------------------------------------------------------------------*/
/* canvas_clear                                                             */
/*--------------------------------------------------------------------------*/

void canvas_clear(void)
{
#if USE_XIMAGE == 1
   memset(canvas->data, 0, canvas->width * canvas->height);

#elif USE_PIXMAP == 1
   canvas_rectangle(0, 0, canvas_width, canvas_height, canvas_black_pixel);
/*                  BlackPixelOfScreen(DefaultScreenOfDisplay(display)));*/
   num_clip_rects = 0;
   canvas_dirty(0, 0, canvas_width, canvas_height);

#else /* no double buffering */
   XSetWindowBackground(display, window, canvas_black_pixel);
/*                      BlackPixelOfScreen(DefaultScreenOfDisplay(display)));*/
   XClearWindow(display, window);

#endif /* USE_... */
}

/*--------------------------------------------------------------------------*/
/* canvas_refresh                                                           */
/*--------------------------------------------------------------------------*/

void canvas_refresh(void)
{
#if USE_XIMAGE == 1
   XShmPutImage(display, window, gc, canvas, 0, 0, 0, 0,
             canvas->width, canvas->height, False);

#elif USE_PIXMAP == 1
   XSetClipRectangles(display, gc, 0, 0, clip_rects, num_clip_rects, Unsorted);
   XCopyArea(display, canvas, window, gc,
             0, 0, canvas_width, canvas_height, 0, 0);
   num_clip_rects = 0;

#endif /* USE_... */
   XSync(display, False);
}

/*--------------------------------------------------------------------------*/
/* canvas_rectangle                                                         */
/*--------------------------------------------------------------------------*/

void canvas_rectangle(int x, int y, int width, int height, long pixel)
{
#if USE_XIMAGE == 1
   char *dest;

   dest = canvas->data + y * canvas->width + x;
   for (; height > 0; height--) {
      memset(dest, pixel, width);
      dest += canvas->width;
   }

#else /* USE_PIXMAP or no double buffering */
   XGCValues gcv;

   gcv.foreground = pixel;
   gcv.clip_x_origin = 0;
   gcv.clip_y_origin = 0;
   gcv.clip_mask = None;
   XChangeGC(display, gc, GCForeground | 
             GCClipXOrigin | GCClipYOrigin | GCClipMask, &gcv);
   XFillRectangle(display, canvas, gc, x, y, width, height);
#if USE_PIXMAP == 1
   canvas_dirty(x, y, width, height);
#endif

#endif /* USE_... */
}

/*--------------------------------------------------------------------------*/
/* canvas_font_load                                                         */
/*--------------------------------------------------------------------------*/

void *canvas_font_load(char *name)
{
   XFontStruct *fs;

   fs = XLoadQueryFont(display, name);
   if (fs == NULL) {
      printf("canvas_font_load():  cannot load font `%s'\n", name);
      exit(EXIT_FAILURE);
   }
#if USE_XIMAGE == 1
   printf("canvas_font_load():  will not do fonts to an XImage\n");
#endif
   return fs;
}

/*--------------------------------------------------------------------------*/
/* canvas_font_size                                                         */
/*--------------------------------------------------------------------------*/

void canvas_font_size(char *msg, void *_font, int *width, int *height)
{
#if USE_XIMAGE == 1
   printf("canvas_font_size():  will not do fonts to an XImage\n");
#else /* USE_PIXMAP or no double buffering */
   int dir, ascent, descent;
   XCharStruct cs;

   XTextExtents((XFontStruct *)_font, msg, strlen(msg),
                &dir, &ascent, &descent, &cs);
   *width = cs.rbearing - cs.lbearing;
   *height = ascent + descent;
#endif /* USE_... */
}

/*--------------------------------------------------------------------------*/
/* canvas_font_print                                                        */
/*--------------------------------------------------------------------------*/

void canvas_font_print(char *msg, int x, int y, void *_font, long pixel)
{
#if USE_XIMAGE == 1
   printf("canvas_font_print():  will not do fonts to an XImage\n");
#else /* USE_PIXMAP or no double buffering */
   XGCValues gcv;
#if USE_PIXMAP == 1
   XCharStruct *cs_min, *cs_max;
#endif

   gcv.foreground = pixel;
   gcv.font = ((XFontStruct *)_font)->fid;
   gcv.clip_x_origin = 0;
   gcv.clip_y_origin = 0;
   gcv.clip_mask = None;
   XChangeGC(display, gc, GCForeground | GCFont |
             GCClipXOrigin | GCClipYOrigin | GCClipMask, &gcv);
   XDrawString(display, canvas, gc, x, y, msg, strlen(msg));

#if USE_PIXMAP == 1
   cs_min = &((XFontStruct *)_font)->min_bounds;
   cs_max = &((XFontStruct *)_font)->max_bounds;
   canvas_dirty(x + cs_min->lbearing, y - cs_max->ascent,
                (cs_max->rbearing - cs_min->lbearing) * strlen(msg),
                cs_max->ascent + cs_max->descent);
#endif

#endif /* USE_... */
}

/*--------------------------------------------------------------------------*/
/* canvas_image_load                                                        */
/*--------------------------------------------------------------------------*/

void canvas_image_load(char *filename, IMAGE *image)
{
   XpmAttributes xpmattrs;
   int err;
#if USE_XIMAGE == 1
   XImage *pixels = NULL, *mask = NULL;
   int i;
#endif

   xpmattrs.valuemask = XpmSize | XpmCloseness | XpmColormap;
   xpmattrs.closeness = 65535;
   xpmattrs.colormap = canvas_colormap;
#if USE_XIMAGE == 1
   err = XpmReadFileToImage(display, filename, &pixels, &mask, &xpmattrs);
#else /* USE_PIXMAP or no double buffering */
   err = XpmReadFileToPixmap(display, canvas, filename,
                             &image->pixmap, &image->mask, &xpmattrs);
#endif
   if (err != XpmSuccess) {
      fprintf(stderr, "error reading image `%s', xpm error code = %d\n", filename, err);
      exit(EXIT_FAILURE);
   }
   image->width = xpmattrs.width;
   image->height = xpmattrs.height;
#if USE_XIMAGE == 1
   image->pixels = malloc(image->width * image->height);
   memcpy(image->pixels, pixels->data, image->width * image->height);
   XDestroyImage(pixels);
   if (mask != NULL) {
      image->not_mask = malloc(image->width * image->height);
      for (i = 0; i < image->width * image->height; i++)
         image->not_mask[i] = 0xFF * !(mask->data[i / 8] & (1 << i % 8));
      XDestroyImage(mask);
   } else
      image->not_mask = NULL;
#endif
}

/*--------------------------------------------------------------------------*/
/* canvas_image_free                                                        */
/*--------------------------------------------------------------------------*/

void canvas_image_free(IMAGE *image)
{
#if USE_XIMAGE == 1
   if (image->pixels != NULL) {
      free(image->pixels);
      image->pixels = NULL;
   }
   if (image->not_mask != NULL) {
      free(image->not_mask);
      image->not_mask = NULL;
   }
#else /* USE_PIXMAP or no double buffering */
   if (image->pixmap != 0) {
      XFreePixmap(display, image->pixmap);
      image->pixmap = 0;
   }
   if (image->mask != 0) {
      XFreePixmap(display, image->mask);
      image->mask = 0;
   }
#endif /* USE_... */
}

/*--------------------------------------------------------------------------*/
/* canvas_image_paint                                                       */
/*--------------------------------------------------------------------------*/

#if 0
void canvas_image_paint(IMAGE *image, int x, int y)
{
   canvas_image_paint_clipped(image, x, y, 0, 0, image->width, image->height);
}
#endif

/*--------------------------------------------------------------------------*/
/* canvas_image_paint                                                       */
/*--------------------------------------------------------------------------*/

void canvas_image_paint(IMAGE *image, int x, int y, int sx, int sy, int sw, int sh)
{
#if USE_XIMAGE == 1
   char *dest, *pixels, *not_mask;
   int i, j;
   int b;

   if (sw != image->width || sh != image->height)
      printf("canvas_image_paint_clipped():  will not clip to an XImage\n");
   dest = canvas->data + y * canvas->width + x;
   pixels = image->pixels;
   not_mask = image->not_mask;
   if ((not_mask + 1) != NULL)
      for (i = 0; i < image->height; i++) {
         memcpy(dest, pixels, image->width);
         dest += canvas->width;
         pixels += image->width;
      }
   else
      for (i = 0; i < image->height; i++) {
         for (j = image->width - 1; j >= 0; j--) {
            b = *dest & not_mask[j];
            *dest++ = b | pixels[j];
         }
         dest += canvas->width - image->width;
         pixels += image->width;
         not_mask += image->width;
      }

#else /* USE_PIXMAP or no double buffering */
   XGCValues gcv;

   gcv.clip_x_origin = x;
   gcv.clip_y_origin = y;
   gcv.clip_mask = image->mask;
   XChangeGC(display, gc, GCClipXOrigin | GCClipYOrigin | GCClipMask, &gcv);
   XCopyArea(display, image->pixmap, canvas, gc, sx, sy, sw, sh, x, y);

#if USE_PIXMAP == 1
   canvas_dirty(x, y, image->width, image->height);
#endif

#endif /* USE_... */
}

/*--------------------------------------------------------------------------*/
/* canvas_key_event                                                         */
/*--------------------------------------------------------------------------*/

void canvas_key_event(XKeyEvent *event)
{
   KeySym keysym;
   XEvent event_space;

   while (1) {
      if (event->type == KeyPress || event->type == KeyRelease) {
         keysym = XLookupKeysym(event, event->state);
         /* only XK_MISCELLANY or XK_LATIN1 keycodes (see keysymdef.h) */
         if ((keysym & 0xFF00) == 0xFF00 || (keysym & 0xFF00) == 0x0000) {
            /* also, if XK_MISCELLANY, keep only 8th bit, for indexing */
            keysym &= 0x1FF;
            keys_down[keysym & 0x01FF] = (event->type == KeyPress);
         }
      }
      if (XCheckMaskEvent(display, KeyPress | KeyRelease, &event_space))
         event = (XKeyEvent *)&event_space;
      else
         break;
   }
}

/*--------------------------------------------------------------------------*/
/* canvas_key_down                                                          */
/*--------------------------------------------------------------------------*/

KeySym canvas_key_down(KeySym keysym)
{
   char *p;
   KeySym k = XK_VoidSymbol;

   if (keysym == NoSymbol) {            /* return first key found down */
      p = memchr(keys_down, 1, 512);
      if (p != NULL) {                  /* only if found key down */
         k = p - keys_down;
         if ((k & 0x0100) == 0x0100)    /* if 8th bit is on */
            k = k | 0xFF00;
      }
      return k;                         /* return translated keysym */
   }
   if ((keysym & 0xFF00) == 0xFF00 || (keysym & 0xFF00) == 0x0000)
      return keys_down[keysym & 0x01FF];
   return 0;                            /* nonstandard keys are always down */
}
