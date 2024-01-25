/*--------------------------------------------------------------------------*/
/* canvas                                                                   */
/*--------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/keysymdef.h>
#include <X11/xpm.h>

#include "canvas.h"

/*--------------------------------------------------------------------------*/
/* defines                                                                  */
/*--------------------------------------------------------------------------*/

#define MAX_CLIP_RECTS 512

/*--------------------------------------------------------------------------*/
/* variables                                                                */
/*--------------------------------------------------------------------------*/

static Display *display;
static Window window;
static Visual *visual;
static int depth;
static GC gc;
static GC gc_and;
static GC gc_or;
static char *keys_down = NULL;

Colormap canvas_colormap;
long canvas_black_pixel;

static Pixmap canvas = None;
static int canvas_width, canvas_height;

static XRectangle *clip_rects;
static int num_clip_rects;

/*--------------------------------------------------------------------------*/
/* canvas_init                                                              */
/*--------------------------------------------------------------------------*/

void canvas_init(Display *_display, Window _window, int width, int height)
{
   Screen *screen;
   XGCValues gcv;

   display = _display;
   window = _window;

   gcv.graphics_exposures = False;
   gc = XCreateGC(display, window, GCGraphicsExposures, &gcv);
   gcv.function = GXand;
   gc_and = XCreateGC(display, window, GCGraphicsExposures | GCFunction, &gcv);
   gcv.function = GXor;
   gc_or = XCreateGC(display, window, GCGraphicsExposures | GCFunction, &gcv);

   screen = DefaultScreenOfDisplay(display);
   visual = DefaultVisualOfScreen(screen);
   depth = DefaultDepthOfScreen(screen);
   canvas_colormap = XCreateColormap(display, window, visual, AllocNone);
   canvas_black_pixel = canvas_alloc_color(0, 0, 0);
   if (visual->class == PseudoColor) {
      XSetWindowColormap(display, window, canvas_colormap);
      XInstallColormap(display, canvas_colormap);
   }

   canvas = XCreatePixmap(display, window, width, height,
                          DefaultDepthOfScreen(screen));
   canvas_width = width;
   canvas_height = height;
   clip_rects = malloc(sizeof(XRectangle) * MAX_CLIP_RECTS);
   num_clip_rects = 0;

   keys_down = calloc(sizeof(char), 512);
   canvas_clear();
}

/*--------------------------------------------------------------------------*/
/* canvas_close                                                             */
/*--------------------------------------------------------------------------*/

void canvas_close(void)
{
}

/*--------------------------------------------------------------------------*/
/* canvas_dirty                                                             */
/*--------------------------------------------------------------------------*/

void canvas_dirty(int x, int y, int width, int height)
{
   int i;
   XRectangle *rect;

   for (i = 0; i < num_clip_rects; i++) {
      if (clip_rects[i].x <= x &&
          clip_rects[i].y <= y &&
          clip_rects[i].x + clip_rects[i].width >= x + width &&
          clip_rects[i].y + clip_rects[i].height >= y + height)
         return;
      rect++;
   }
   if (num_clip_rects >= MAX_CLIP_RECTS) {
      fprintf(stderr, "canvas_dirty():  too many dirty rectangles.\n");
      exit(EXIT_FAILURE);
   }
   clip_rects[num_clip_rects].x = x;
   clip_rects[num_clip_rects].y = y;
   clip_rects[num_clip_rects].width = width;
   clip_rects[num_clip_rects].height = height;
   num_clip_rects++;
}

/*--------------------------------------------------------------------------*/
/* canvas_alloc_color                                                       */
/*--------------------------------------------------------------------------*/

long canvas_alloc_color(int red, int green, int blue)
{
   XColor xc;

   xc.red = red << 8;
   xc.green = green << 8;
   xc.blue = blue << 8;
   xc.flags = DoRed | DoGreen | DoBlue;
   XAllocColor(display, canvas_colormap, &xc);
   return xc.pixel;
}

/*--------------------------------------------------------------------------*/
/* canvas_clear                                                             */
/*--------------------------------------------------------------------------*/

void canvas_clear(void)
{
   canvas_rectangle(0, 0, canvas_width, canvas_height, canvas_black_pixel);
/*                  BlackPixelOfScreen(DefaultScreenOfDisplay(display)));*/
   num_clip_rects = 0;
   canvas_dirty(0, 0, canvas_width, canvas_height);
}

/*--------------------------------------------------------------------------*/
/* canvas_refresh                                                           */
/*--------------------------------------------------------------------------*/

void canvas_refresh(void)
{
#if 0
   int i, j;

   if (num_clip_rects > 5) {
      /*
      printf("*** Minimizing %d Clips ***\n", num_clip_rects);
      */
      for (i = 0; i < num_clip_rects; i++)
         for (j = i + 1; j < num_clip_rects; j++) {
            if (clip_rects[j].y == clip_rects[i].y &&
                clip_rects[j].height == clip_rects[i].height &&
                clip_rects[j].x == clip_rects[i].x + clip_rects[i].width) {
               clip_rects[i].width += clip_rects[j].width;
               memcpy(&clip_rects[j], &clip_rects[j + 1],
                      (num_clip_rects - j - 1) * sizeof(XRectangle));
               num_clip_rects--;
               j--;
               continue;
            }
            if (clip_rects[j].x == clip_rects[i].x &&
                clip_rects[j].width == clip_rects[i].width &&
                clip_rects[j].y == clip_rects[i].y + clip_rects[i].height) {
               clip_rects[i].height += clip_rects[j].height;
               memcpy(&clip_rects[j], &clip_rects[j + 1],
                      (num_clip_rects - j - 1) * sizeof(XRectangle));
               num_clip_rects--;
               j--;
               continue;
            }
            /*
            if (clip_rects[i].x <= clip_rects[i].x &&
                clip_rects[i].y <= clip_rects[i].y &&
                clip_rects[i].x + clip_rects[j].width >= clip_rects[j].x + clip_rects[j].width &&
                clip_rects[i].y + clip_rects[j].height >= clip_rects[j].y + clip_rects[j].height) {
               memcpy(&clip_rects[j], &clip_rects[j + 1],
                      (num_clip_rects - j - 1) * sizeof(XRectangle));
               num_clip_rects--;
               j--;
               continue;
            }
            */
         }
      /*
      printf("*** Reduced to %d Clips ***\n", num_clip_rects);
      */
   }
#endif

   XSetClipRectangles(display, gc, 0, 0, clip_rects, num_clip_rects, Unsorted);
   XCopyArea(display, canvas, window, gc,
             0, 0, canvas_width, canvas_height, 0, 0);
   num_clip_rects = 0;
   XSync(display, False);
}

/*--------------------------------------------------------------------------*/
/* canvas_rectangle                                                         */
/*--------------------------------------------------------------------------*/

void canvas_rectangle(int x, int y, int width, int height, long pixel)
{
   XGCValues gcv;

   gcv.foreground = pixel;
   gcv.clip_x_origin = 0;
   gcv.clip_y_origin = 0;
   gcv.clip_mask = None;
   XChangeGC(display, gc, GCForeground | 
             GCClipXOrigin | GCClipYOrigin | GCClipMask, &gcv);
   XFillRectangle(display, canvas, gc, x, y, width, height);
   canvas_dirty(x, y, width, height);
}

/*--------------------------------------------------------------------------*/
/* canvas_font_load                                                         */
/*--------------------------------------------------------------------------*/

void *canvas_font_load(char *name)
{
   XFontStruct *fs;

   fs = XLoadQueryFont(display, name);
   if (fs == NULL) {
      fprintf(stderr, "canvas_font_load():  cannot load font `%s'\n", name);
      exit(EXIT_FAILURE);
   }
   return fs;
}

/*--------------------------------------------------------------------------*/
/* canvas_font_size                                                         */
/*--------------------------------------------------------------------------*/

void canvas_font_size(char *msg, void *_font, int *width, int *height)
{
   int dir, ascent, descent;
   XCharStruct cs;

   XTextExtents((XFontStruct *)_font, msg, strlen(msg),
                &dir, &ascent, &descent, &cs);
   *width = cs.rbearing - cs.lbearing;
   *height = ascent + descent;
}

/*--------------------------------------------------------------------------*/
/* canvas_font_print                                                        */
/*--------------------------------------------------------------------------*/

void canvas_font_print(char *msg, int x, int y, void *_font, long pixel)
{
   XGCValues gcv;
   XCharStruct *cs_min, *cs_max;

   gcv.foreground = pixel;
   gcv.font = ((XFontStruct *)_font)->fid;
   gcv.clip_x_origin = 0;
   gcv.clip_y_origin = 0;
   gcv.clip_mask = None;
   XChangeGC(display, gc, GCForeground | GCFont |
             GCClipXOrigin | GCClipYOrigin | GCClipMask, &gcv);
   XDrawString(display, canvas, gc, x, y, msg, strlen(msg));

   cs_min = &((XFontStruct *)_font)->min_bounds;
   cs_max = &((XFontStruct *)_font)->max_bounds;
   canvas_dirty(x + cs_min->lbearing, y - cs_max->ascent,
                (cs_max->rbearing - cs_min->lbearing) * strlen(msg),
                cs_max->ascent + cs_max->descent);
}

/*--------------------------------------------------------------------------*/
/* canvas_image_load                                                        */
/*--------------------------------------------------------------------------*/

void canvas_image_load(char *filename, IMAGE *image)
{
   XpmAttributes xpmattrs;
   int err;
   XImage *omask, *nmask;
   char *ndata;
   int x, y;
   long pixel;

   xpmattrs.valuemask = XpmSize | XpmCloseness | XpmColormap;
   xpmattrs.closeness = 65535;
   xpmattrs.colormap = canvas_colormap;
   err = XpmReadFileToPixmap(display, canvas, filename,
                             &image->pixmap, &image->mask, &xpmattrs);
   if (err != XpmSuccess) {
      fprintf(stderr, "error reading image `%s', xpm error code = %d\n", filename, err);
      exit(EXIT_FAILURE);
   }
   image->width = xpmattrs.width;
   image->height = xpmattrs.height;

   if (image->mask == None)
      return;
   omask = XGetImage(display, image->mask, 0, 0, image->width, image->height,
                     AllPlanes, ZPixmap);
   ndata = malloc(image->width * image->height);
   nmask = XCreateImage(display, visual, depth, ZPixmap, 0, ndata,
                        image->width, image->height, 8, 0);
   for (y = 0; y < omask->height; y++)
      for (x = 0; x < omask->width; x++) {
         if (XGetPixel(omask, x, y) == 0)
            pixel = (1 << depth) - 1;
         else
            pixel = 0;
         XPutPixel(nmask, x, y, pixel);
      }
   XFreePixmap(display, image->mask);
   image->mask = XCreatePixmap(display, window,
                               image->width, image->height, depth);
   XPutImage(display, image->mask, gc, nmask, 0, 0, 0, 0,
             image->width, image->height);
   XDestroyImage(omask);
   XDestroyImage(nmask);
}

/*--------------------------------------------------------------------------*/
/* canvas_image_free                                                        */
/*--------------------------------------------------------------------------*/

void canvas_image_free(IMAGE *image)
{
   if (image->pixmap != 0) {
      XFreePixmap(display, image->pixmap);
      image->pixmap = 0;
   }
   if (image->mask != 0) {
      XFreePixmap(display, image->mask);
      image->mask = 0;
   }
}

/*--------------------------------------------------------------------------*/
/* canvas_image_paint                                                       */
/*--------------------------------------------------------------------------*/

void canvas_image_paint(IMAGE *image, int x, int y, int sx, int sy, int sw, int sh)
{
   if (image->mask == None)
      XCopyArea(display, image->pixmap, canvas, gc, sx, sy, sw, sh, x, y);
   else {
      XCopyArea(display, image->mask, canvas, gc_and, sx, sy, sw, sh, x, y);
      XCopyArea(display, image->pixmap, canvas, gc_or, sx, sy, sw, sh, x, y);
   }
   canvas_dirty(x, y, image->width, image->height);

#if 0
   XGCValues gcv;

   gcv.clip_x_origin = x;
   gcv.clip_y_origin = y;
   gcv.clip_mask = image->mask;
   XChangeGC(display, gc, GCClipXOrigin | GCClipYOrigin | GCClipMask, &gcv);
   XCopyArea(display, image->pixmap, canvas, gc, sx, sy, sw, sh, x, y);
   canvas_dirty(x, y, image->width, image->height);
#endif
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
