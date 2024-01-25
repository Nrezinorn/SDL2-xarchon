/*--------------------------------------------------------------------------*/
/* canvas                                                                   */
/*--------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/keysymdef.h>
#include <X11/xpm.h>
#include <X11/extensions/shape.h>

#include "canvas.h"

/*--------------------------------------------------------------------------*/
/* defines                                                                  */
/*--------------------------------------------------------------------------*/

#define MAX_CLIP_RECTS 512

/*--------------------------------------------------------------------------*/
/* functions                                                                */
/*--------------------------------------------------------------------------*/

static unsigned short *canvas_image_to_rgbs(IMAGE *image, XImage *ximage);
static void canvas_image_from_rgbs(IMAGE *image, unsigned short *data, XImage *ximage);
static void canvas_mouse_drag_event(XMotionEvent *event);
static void canvas_mouse_button_event(XButtonEvent *event);

/*--------------------------------------------------------------------------*/
/* variables                                                                */
/*--------------------------------------------------------------------------*/

static Display *display;
static Window window;
static Screen *screen;
static int screen_num;
static Visual *visual;
static GC gc, gc1 = None;
static char *keys_down = NULL;

Colormap canvas_colormap = None;
long canvas_black_pixel;

static Pixmap canvas = None;
static int canvas_width, canvas_height;

static XRectangle *clip_rects;
static int num_clip_rects;

static Pixmap collision_map = None;

static int enabled = 1;

static char warp_mouse = 0;
static int mouse_x = 0, mouse_y = 0;
static char mouse_button_pushed = 0;

/*--------------------------------------------------------------------------*/
/* canvas_init                                                              */
/*--------------------------------------------------------------------------*/

void canvas_init(Display *_display, Window _window, int width, int height)
{
   XGCValues gcv;

   display = _display;
   window = _window;

   screen = DefaultScreenOfDisplay(display);
   screen_num = DefaultScreen(display);
   visual = DefaultVisualOfScreen(screen);
   canvas_colormap = XCreateColormap(display, window, visual, AllocNone);
   canvas_black_pixel = canvas_alloc_color(0, 0, 0);
   canvas_install_colormap(1);

   gcv.graphics_exposures = False;
   gc = XCreateGC(display, window, GCGraphicsExposures, &gcv);

   canvas_width = width;
   canvas_height = height;
   canvas = XCreatePixmap(display, window, canvas_width, canvas_height,
                          DefaultDepthOfScreen(screen));

   clip_rects = malloc(sizeof(XRectangle) * MAX_CLIP_RECTS);
   num_clip_rects = 0;

   keys_down = calloc(sizeof(char), 512);
   canvas_clear();
}

/*--------------------------------------------------------------------------*/
/* canvas_enable                                                            */
/*--------------------------------------------------------------------------*/

void canvas_enable(int yes)
{
   enabled = yes;
}

/*--------------------------------------------------------------------------*/
/* canvas_install_colormap                                                  */
/*--------------------------------------------------------------------------*/

void canvas_install_colormap(int yes)
{
   if (!enabled || visual->class != PseudoColor)
      return;
   if (yes) {
      XSetWindowColormap(display, window, canvas_colormap);
      XInstallColormap(display, canvas_colormap);
   } else {
      XSetWindowColormap(display, window, DefaultColormapOfScreen(screen));
      XUninstallColormap(display, canvas_colormap);
   }
}

/*--------------------------------------------------------------------------*/
/* canvas_close                                                             */
/*--------------------------------------------------------------------------*/

void canvas_close(void)
{
   canvas_install_colormap(0);
   XFreeGC(display, gc);
   XFreeColormap(display, canvas_colormap);
   XFreePixmap(display, canvas);
   free(clip_rects);
   free(keys_down);
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
   if (!enabled)
      return;
   canvas_rectangle(0, 0, canvas_width, canvas_height, canvas_black_pixel);
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

   if (!enabled)
      return;
   XSetClipRectangles(display, gc, 0, 0, clip_rects, num_clip_rects, Unsorted);
   XCopyArea(display, canvas, window, gc,
             0, 0, canvas_width, canvas_height, 0, 0);
   num_clip_rects = 0;
   XSync(display, False);
}

/*--------------------------------------------------------------------------*/
/* canvas_refresh_all                                                       */
/*--------------------------------------------------------------------------*/

void canvas_refresh_all(void)
{
   if (!enabled)
      return;
   XSetClipMask(display, gc, None);
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

   if (!enabled)
      return;
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

   if (!enabled)
      return;
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

   xpmattrs.valuemask = XpmSize | XpmCloseness | XpmColormap | XpmReturnPixels;
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
   image->colors = malloc(sizeof(long) * xpmattrs.npixels);
   memcpy(image->colors, xpmattrs.pixels, sizeof(long) * xpmattrs.npixels);
   image->num_colors = xpmattrs.npixels;
}

/*--------------------------------------------------------------------------*/
/* canvas_image_copy                                                        */
/*--------------------------------------------------------------------------*/

void canvas_image_copy(IMAGE *oldimg, IMAGE *newimg)
{
   XColor *xcs;
   int i;

   XSetClipMask(display, gc, None);
   newimg->width = oldimg->width;
   newimg->height = oldimg->width;
   newimg->pixmap = XCreatePixmap(display, canvas,
                                  newimg->width, newimg->height,
                                  DefaultDepthOfScreen(screen));
   XCopyArea(display, oldimg->pixmap, newimg->pixmap, gc,
             0, 0, newimg->width, newimg->height, 0, 0);

   newimg->colors = malloc(sizeof(long) * oldimg->num_colors);
   memcpy(newimg->colors, oldimg->colors, sizeof(long) * oldimg->num_colors);
   newimg->num_colors = oldimg->num_colors;
   xcs = malloc(sizeof(XColor) * newimg->num_colors);
   for (i = 0; i < newimg->num_colors; i++)
      xcs[i].pixel = newimg->colors[i];
   XQueryColors(display, canvas_colormap, xcs, newimg->num_colors);
   for (i = 0; i < newimg->num_colors; i++) {
      xcs[i].flags = DoRed | DoGreen | DoBlue;
      XAllocColor(display, canvas_colormap, &xcs[i]);
   }
   free(xcs);

   if (oldimg->mask != None) {
      newimg->mask = XCreatePixmap(display, canvas,
                                   newimg->width, newimg->height, 1);
      if (gc1 == None)
         gc1 = XCreateGC(display, oldimg->mask, 0, NULL);
      XCopyArea(display, oldimg->mask, newimg->mask, gc1,
                0, 0, newimg->width, newimg->height, 0, 0);
   } else
      newimg->mask = None;
}

/*--------------------------------------------------------------------------*/
/* canvas_image_to_rgbs                                                     */
/*--------------------------------------------------------------------------*/

unsigned short *canvas_image_to_rgbs(IMAGE *image, XImage *ximage)
{
   XColor *xcs, *xc;
   int i;
   int x, y;
   long pixel;
   unsigned short *data, *datap;

   xcs = malloc(sizeof(XColor) * image->num_colors);
   for (i = 0; i < image->num_colors; i++)
      xcs[i].pixel = image->colors[i];
   XQueryColors(display, canvas_colormap, xcs, image->num_colors);

   data = malloc(image->width * image->height * sizeof(unsigned short) * 3);
   for (y = 0; y < image->height; y++) {
      datap = data + y * image->width * 3;
      for (x = 0; x < image->width; x++) {
         pixel = XGetPixel(ximage, x, y);
         for (xc = xcs; xc->pixel != pixel; xc++)
            ;
         *(datap + 0) = xc->red;
         *(datap + 1) = xc->green;
         *(datap + 2) = xc->blue;
         datap += 3;
      }
   }

   free(xcs);
   return data;
}

/*--------------------------------------------------------------------------*/
/* canvas_image_from_rgbs                                                   */
/*--------------------------------------------------------------------------*/

void canvas_image_from_rgbs(IMAGE *image, unsigned short *data, XImage *ximage)
{
   XColor *xcs, xc;
   int count, i;
   int x, y;
   unsigned short *datap;

   XFreeColors(display, canvas_colormap, image->colors, image->num_colors, 0);

   xcs = malloc(sizeof(XColor) * image->width * image->height);
   count = 0;
   for (y = 0; y < image->height; y++) {
      datap = data + y * image->width * 3;
      for (x = 0; x < image->width; x++) {
         xcs[count].red = *(datap + 0);
         xcs[count].green = *(datap + 1);
         xcs[count].blue = *(datap + 2);
         datap += 3;
         for (i = 0; i < count; i++)
            if (xcs[i].red == xcs[count].red &&
                xcs[i].green == xcs[count].green &&
                xcs[i].blue == xcs[count].blue)
               break;
         if (i == count) {
            xcs[count].flags = DoRed | DoGreen | DoBlue;
            memcpy(&xc, &xcs[count], sizeof(XColor));
            XAllocColor(display, canvas_colormap, &xc);
            xcs[count].pixel = xc.pixel;
            count++;
         }
         XPutPixel(ximage, x, y, xcs[i].pixel);
      }
   }

   free(image->colors);
   image->colors = malloc(sizeof(long) * count);
   for (i = 0; i < count; i++)
      image->colors[i] = xcs[i].pixel;
   image->num_colors = count;
   free(xcs);

   XSetClipMask(display, gc, None);
   XPutImage(display, image->pixmap, gc, ximage, 0, 0, 0, 0,
             image->width, image->height);
   free(data);
}

/*--------------------------------------------------------------------------*/
/* canvas_image_modify                                                      */
/*--------------------------------------------------------------------------*/

void canvas_image_modify(IMAGE *image, canvas_modify_func func)
{
   XImage *mask, *pixmap;
   unsigned short *rgbs;

   if (image->mask != None)
      mask = XGetImage(display, image->mask, 0, 0,
                       image->width, image->height, AllPlanes, ZPixmap);
   else
      mask = NULL;

   pixmap = XGetImage(display, image->pixmap, 0, 0,
                      image->width, image->height, AllPlanes, ZPixmap);
   rgbs = canvas_image_to_rgbs(image, pixmap);
   func(image->width, image->height, rgbs, mask->data);
   canvas_image_from_rgbs(image, rgbs, pixmap);
   XDestroyImage(pixmap);

   if (image->mask != None) {
      if (gc1 == None)
         gc1 = XCreateGC(display, image->mask, 0, NULL);
      XPutImage(display, image->mask, gc1, mask, 0, 0, 0, 0,
                image->width, image->height);
      XDestroyImage(mask);
   }
}

/*--------------------------------------------------------------------------*/
/* canvas_image_free                                                        */
/*--------------------------------------------------------------------------*/

void canvas_image_free(IMAGE *image)
{
   if (image->pixmap != None) {
      XFreePixmap(display, image->pixmap);
      image->pixmap = 0;
      XFreeColors(display, canvas_colormap,
                  image->colors, image->num_colors, 0);
      free(image->colors);
      image->colors = 0;
      image->num_colors = 0;
   }
   if (image->mask != None) {
      XFreePixmap(display, image->mask);
      image->mask = 0;
   }
}

/*--------------------------------------------------------------------------*/
/* canvas_image_paint                                                       */
/*--------------------------------------------------------------------------*/

void canvas_image_paint(IMAGE *image, IMAGE *mask_image, int x, int y, int sx, int sy, int sw, int sh)
{
   XGCValues gcv;

   if (!enabled)
      return;
   gcv.clip_x_origin = x - sx;
   gcv.clip_y_origin = y - sy;
   gcv.clip_mask = mask_image->mask;
   XChangeGC(display, gc, GCClipXOrigin | GCClipYOrigin | GCClipMask, &gcv);
   XCopyArea(display, image->pixmap, canvas, gc, sx, sy, sw, sh, x, y);
   canvas_dirty(x, y, image->width, image->height);
}

/*--------------------------------------------------------------------------*/
/* canvas_image_intersect                                                   */
/*--------------------------------------------------------------------------*/

int canvas_image_intersect(IMAGE *image1, IMAGE *image2, int x_off, int y_off)
{
   XImage *intersection;
   int y1, y2;
   int sub_w, sub_h;
   int x, y;
   int result = 0;

   if (image1->mask == None && image2->mask == None)
      return 1;         /* if they're both rectangular, and we've got here, */
                        /* then they intersect by definition */
   if (gc1 == None) {
      if (image1->mask == None)
         gc1 = XCreateGC(display, image2->mask, 0, NULL);
      else
         gc1 = XCreateGC(display, image1->mask, 0, NULL);
   }
   XSetForeground(display, gc1, BlackPixel(display, screen_num));
   if (collision_map == None) {
      collision_map = XCreatePixmap(display, window,
            image1->width, image1->height, 1);
      XFillRectangle(display, collision_map, gc1, 0, 0, image1->width,
            image1->height);
   }
   sub_w = image1->width - x_off;
   if (y_off < 0) {
      y1 = -y_off;
      y2 = 0;
      sub_h = image1->height + y_off;
   } else {
      y1 = 0;
      y2 = y_off;
      sub_h = image1->height - y_off;
   }
   XSetClipOrigin(display, gc1, 0, 0);
   /* if image1->mask == None, this is fine - turning off the clip mask is */
   /* the right thing to do in that circumstance (image1 being rectangular) */
   XSetClipMask(display, gc1, image1->mask);
   if (image2->mask != None)
      XCopyArea(display, image2->mask, collision_map, gc1, 0, y1, sub_w,
            sub_h, x_off, y2);
   else {
      /* if image2->mask == None, use the whole of images2's rectangle */
      /* (clipped with image1's mask) */
      XSetForeground(display, gc1, WhitePixel(display, screen_num));
      XFillRectangle(display, collision_map, gc1, x_off, y2, sub_w, sub_h);
      XSetForeground(display, gc1, BlackPixel(display, screen_num));
   }
   XSetClipMask(display, gc1, None);
   intersection = XGetImage(display, collision_map, x_off, y2, sub_w, sub_h,
         AllPlanes, ZPixmap);
   XFillRectangle(display, collision_map, gc1, x_off, y2, sub_w, sub_h);
   for (y = 0; !result && y < intersection->height; y++)
      for (x = 0; !result && x < intersection->width; x++) {
         if (XGetPixel(intersection, x, y))
            result = 1;
      }
   XDestroyImage(intersection);
   return result;
}

/*--------------------------------------------------------------------------*/
/* canvas_key_event                                                         */
/*--------------------------------------------------------------------------*/

void canvas_key_event(XKeyEvent *event)
{
   KeySym keysym;
   XEvent event_space;

   if (event == NULL) {
      memset(keys_down, 0, sizeof(char) * 512);
      return;
   }

   while (1) {
      if (event->type == KeyPress || event->type == KeyRelease) {
         keysym = XLookupKeysym(event, 0); /* event->state); */
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

/*--------------------------------------------------------------------------*/
/* canvas_keysym_name                                                       */
/*--------------------------------------------------------------------------*/

char *canvas_keysym_name(KeySym keysym)
{
   return XKeysymToString(keysym);
}

/*--------------------------------------------------------------------------*/
/* canvas_keycode_name                                                      */
/*--------------------------------------------------------------------------*/

char *canvas_keycode_name(KeyCode keycode)
{
   return canvas_keysym_name(XKeycodeToKeysym(display, keycode, 0));
}

/*--------------------------------------------------------------------------*/
/* canvas_mouse_init                                                        */
/*--------------------------------------------------------------------------*/

void canvas_mouse_init(int absolute)
{
   warp_mouse = !absolute;
}

/*--------------------------------------------------------------------------*/
/* canvas_init_cursor                                                       */
/*--------------------------------------------------------------------------*/

void canvas_init_cursor(int on)
{
   XColor c;
   Pixmap p;
   static Cursor cursor = -1;

   if (warp_mouse) {
      if (on) {
         /* confine the pointer & turn off cursor in window */
         if (cursor == -1) {
            memset(&c, 0, sizeof(c));
            p = XCreatePixmap(display, window, 1, 1, 1);
            cursor = XCreatePixmapCursor(display, p, p, &c, &c, 0, 0);
            XFreePixmap(display, p);
         }
         XGrabPointer(display, window, True, 0, GrabModeAsync, GrabModeAsync,
               window, cursor, CurrentTime);
      } else {
         /* release the pointer, turn the cursor back on */
         XUngrabPointer(display, CurrentTime);
         XUndefineCursor(display, window);
      }
   }
}

/*--------------------------------------------------------------------------*/
/* canvas_mouse_drag_event                                                  */
/*--------------------------------------------------------------------------*/

void canvas_mouse_drag_event(XMotionEvent *event)
{
   static int last_x = -1, last_y;

   if (warp_mouse) {
      if (last_x != -1) {
         mouse_x = event->x - last_x;
         mouse_y = event->y - last_y;
         if (mouse_x*mouse_x + mouse_y*mouse_y < 100) {
            /* wait until we have a substantial movement of the mouse */
            mouse_x = mouse_y = 0;
            return;
         }
      }
      if (event->x < 40 || event->x > canvas_width - 40 ||
            event->y < 40 || event->y > canvas_height - 40) {
         last_x = canvas_width/2;
         last_y = canvas_height/2;
         XWarpPointer(display, 0, window, 0, 0, 0, 0, last_x, last_y);
      } else {
         last_x = event->x;
         last_y = event->y;
      }
   } else {
      mouse_x = event->x;
      mouse_y = event->y;
   }
}

/*--------------------------------------------------------------------------*/
/* canvas_mouse_button_event                                                */
/*--------------------------------------------------------------------------*/

void canvas_mouse_button_event(XButtonEvent *event)
{
   if (event->button == 1)
      mouse_button_pushed = (event->type == ButtonPress);
}

/*--------------------------------------------------------------------------*/
/* canvas_mouse_event                                                       */
/*--------------------------------------------------------------------------*/

void canvas_mouse_event(XEvent *event)
{
  if (event->type == MotionNotify)
     canvas_mouse_drag_event((XMotionEvent *)event);
  if (event->type == ButtonPress || event->type == ButtonRelease)
     canvas_mouse_button_event((XButtonEvent *)event);
}

/*--------------------------------------------------------------------------*/
/* canvas_mouse_get                                                         */
/*--------------------------------------------------------------------------*/

int canvas_mouse_get(int *x, int *y)
{
   if (x != NULL && y != NULL) {
      *x = mouse_x;
      *y = mouse_y;
      if (warp_mouse)
         mouse_x = mouse_y = 0;
   }
   return mouse_button_pushed;
}
