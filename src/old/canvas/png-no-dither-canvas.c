/*--------------------------------------------------------------------------*/
/* canvas                                                                   */
/*--------------------------------------------------------------------------*/

/* The PNG and dithering portions in this source were written by
   Matt Kimball <mkimball@xmission.com> */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/keysymdef.h>
#include <X11/xpm.h>
#include <png.h>

#include "canvas.h"
#include "dither.h"

/*--------------------------------------------------------------------------*/
/* defines                                                                  */
/*--------------------------------------------------------------------------*/

#define MAX_CLIP_RECTS 512

/*--------------------------------------------------------------------------*/
/* functions                                                                */
/*--------------------------------------------------------------------------*/

static void canvas_dirty(int x, int y, int width, int height);
static void canvas_image_load_xpm(char *filename, IMAGE *image);
static int canvas_mask_to_shift(int mask);
static void canvas_image_dither(unsigned char **rows, IMAGE *image);
static void canvas_image_load_png(FILE *png, IMAGE *image);

/*--------------------------------------------------------------------------*/
/* variables                                                                */
/*--------------------------------------------------------------------------*/

static Display *display;
static Window window;
static Visual *visual;
static GC gc;
static char *keys_down = NULL;

Colormap canvas_colormap;
long canvas_black_pixel;

static Pixmap canvas = 0;
static int canvas_width, canvas_height;
static int depth;

static XRectangle *clip_rects;
static int num_clip_rects;

/*--------------------------------------------------------------------------*/
/* canvas_init                                                              */
/*--------------------------------------------------------------------------*/

void canvas_init(Display *_display, Window _window, int width, int height)
{
   Screen *screen;

   display = _display;
   window = _window;
   gc = XCreateGC(display, window, 0, NULL);
   XSetGraphicsExposures(display, gc, False);
   keys_down = calloc(sizeof(char), 512);

   screen = DefaultScreenOfDisplay(display);
   visual = DefaultVisualOfScreen(screen);
   canvas_colormap = XCreateColormap(display, window, visual, AllocNone);
   canvas_black_pixel = canvas_alloc_color(0, 0, 0);
   XSetWindowColormap(display, window, canvas_colormap);
   XInstallColormap(display, canvas_colormap);

   canvas = XCreatePixmap(display, window, width, height,
                          DefaultDepthOfScreen(screen));
   canvas_width = width;
   canvas_height = height;
   depth = DefaultDepthOfScreen(screen);

   clip_rects = malloc(sizeof(XRectangle) * MAX_CLIP_RECTS);
   num_clip_rects = 0;

   canvas_clear();
}

/*--------------------------------------------------------------------------*/
/* canvas_close                                                             */
/*--------------------------------------------------------------------------*/

void canvas_close(void)
{
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
/* canvas_dirty                                                             */
/*--------------------------------------------------------------------------*/

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

/*--------------------------------------------------------------------------*/
/* canvas_clear                                                             */
/*--------------------------------------------------------------------------*/

void canvas_clear(void)
{
   canvas_rectangle(0, 0, canvas_width, canvas_height, canvas_black_pixel);
   num_clip_rects = 0;
   canvas_dirty(0, 0, canvas_width, canvas_height);
}

/*--------------------------------------------------------------------------*/
/* canvas_refresh                                                           */
/*--------------------------------------------------------------------------*/

void canvas_refresh(void)
{
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
      printf("canvas_font_load():  cannot load font `%s'\n", name);
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
/* canvas_image_load_xpm                                                    */
/*--------------------------------------------------------------------------*/

void canvas_image_load_xpm(char *filename, IMAGE *image)
{
   XpmAttributes xpmattrs;
   int err;

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
}

/*--------------------------------------------------------------------------*/
/* canvas_mask_to_shift                                                     */
/*--------------------------------------------------------------------------*/

int canvas_mask_to_shift(int mask)
{
   int shift = 0;
   int count = 0;

   while ((mask & 1) == 0 && shift < 32) {
      shift++;
      mask = mask >> 1;
   }
   while ((mask & 1) == 1 && shift + count < 32) {
      count++;
      mask = mask >> 1;
   }

   return shift - (8 - count);
}

/*--------------------------------------------------------------------------*/
/* canvas_image_dither                                                      */
/*--------------------------------------------------------------------------*/

void canvas_image_dither(unsigned char **rows, IMAGE *image)
{
   char *color_data, *mask_data;
   XImage *color, *mask;
   int color_row, mask_row, color_bpp;
   int red_shift, green_shift, blue_shift;
   int x, y;
   GC bit_gc;

   color_bpp = (depth + 7) / 8;
   color_row = image->width * color_bpp;
   mask_row = (image->width + 7) / 8;

   color_data = malloc(color_row * image->height);
   mask_data = malloc(mask_row * image->height);

   if (color_data == NULL || mask_data == NULL) {
      fprintf(stderr, "Out of memory while dithering image\n");
      exit(EXIT_FAILURE);
   }

   memset(color_data, 0, color_row * image->height); 
   memset(mask_data, 0, mask_row * image->height);

   color = XCreateImage(display, visual, depth, ZPixmap, 0, 
                        color_data, image->width, image->height, 8, color_row);
   mask = XCreateImage(display, visual, 1, XYPixmap, 0, 
                       mask_data, image->width, image->height, 8, mask_row); 

   red_shift = canvas_mask_to_shift(mask->red_mask);
   green_shift = canvas_mask_to_shift(mask->green_mask);
   blue_shift = canvas_mask_to_shift(mask->blue_mask);
   for(y = 0; y < image->height; y++) {
	   unsigned char *color_at, *source_at, *mask_at;
	   int mask_shift;

	   mask_shift = 0;
	   color_at = color_data + color_row * y;
	   mask_at = mask_data + mask_row * y;
	   source_at = rows[y];

	   for(x = 0; x < image->width; x++) {
		   int r, g, b;
		   int c;
                   int dith;

                   dith = (DM[y & 127][x & 127] << 2) | 7;

		   r = source_at[0];
		   g = source_at[1];
		   b = source_at[2];

		   if(visual->class == PseudoColor) {
			   /*
			   r = ((r * 5) + dith) >> 8;
			   g = ((g * 5) + (262 - dith)) >> 8;
			   b = ((b * 5) + dith) >> 8;
			   */

			   c = canvas_alloc_color(r, g, b);

		   } else {
			   c = 0;
			   if(red_shift > 0)
				   c |= (r << red_shift) & mask->red_mask;
			   else
				   c |= (r >> -red_shift) & mask->red_mask;
			   if(green_shift > 0)
				   c |= (g << green_shift) & mask->green_mask;
			   else
				   c |= (g >> -green_shift) & mask->green_mask;
			   if(blue_shift > 0)
				   c |= (b << blue_shift) & mask->blue_mask;
			   else
				   c |= (b >> -blue_shift) & mask->blue_mask;
		   }

		   if(color->byte_order == LSBFirst) {
			   *(color_at++) = c;
			   if(color_bpp >= 2)
				   *(color_at++) = c >> 8;
			   if(color_bpp >= 3)
				   *(color_at++) = c >> 16;
			   if(color_bpp >= 4)
				   *(color_at++) = c >> 24;
		   } else {
			   *(color_at++) = c >> (color_bpp * 8 - 8);
			   if(color_bpp >= 2)
				   *(color_at++) = c >> (color_bpp * 8 - 16);
			   if(color_bpp >= 3)
				   *(color_at++) = c >> (color_bpp * 8 - 24);
			   if(color_bpp >= 4)
				   *(color_at++) = c >> (color_bpp * 8 - 32);
		   }
		   
           /*  Always dither alpha because X11 doesn't have support for 
               alpha.  */
		   if(source_at[3] + dith >= 0x100) {
			   if(mask->bitmap_bit_order == LSBFirst)
				   *mask_at = *mask_at | (1 << mask_shift);
			   else
				   *mask_at = *mask_at | (1 << (7 - mask_shift));
		   }

		   mask_shift++;
		   if(mask_shift >= 8) {
			   mask_shift = 0;
			   mask_at++;
		   }
		   source_at += 4;
	   }
   }
   
   image->pixmap = XCreatePixmap(display, canvas, 
                                 image->width, image->height, depth);
   XPutImage(display, image->pixmap, gc, color, 0, 0, 0, 0, 
             image->width, image->height);

   image->mask = XCreatePixmap(display, canvas, 
                               image->width, image->height, 1);
   bit_gc = XCreateGC(display, image->mask, 0, NULL); 
   XPutImage(display, image->mask, bit_gc, mask, 0, 0, 0, 0, 
             image->width, image->height);


   XDestroyImage(color);
   XDestroyImage(mask);
   XFreeGC(display, bit_gc);
}

/*--------------------------------------------------------------------------*/
/* canvas_image_load_png                                                    */
/*--------------------------------------------------------------------------*/

void canvas_image_load_png(FILE *png, IMAGE *image)
{
   png_structp png_ptr;
   png_infop info_ptr;
   unsigned char **rows, *color_data;
   int at;

   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
   if(png_ptr != NULL)
      info_ptr = png_create_info_struct(png_ptr);

   if (png_ptr == NULL || info_ptr == NULL) {
      fprintf(stderr, "Error initializing libpng\n");
	  exit(EXIT_FAILURE);
   }

   png_init_io(png_ptr, png);
   png_set_sig_bytes(png_ptr, 8);
   png_read_info(png_ptr, info_ptr);
   image->width = png_get_image_width(png_ptr, info_ptr);
   image->height = png_get_image_height(png_ptr, info_ptr);
   png_set_expand(png_ptr);
   png_set_filler(png_ptr, 255, PNG_FILLER_AFTER);
   png_set_strip_16(png_ptr);
   png_set_gray_to_rgb(png_ptr);

   rows = malloc(sizeof(char *) * image->height);
   color_data = malloc(4 * image->width * image->height);

   if (rows == NULL || color_data == NULL) {
      fprintf(stderr, "Out of memory while loading PNG\n");
      exit(EXIT_FAILURE);
   }

   for (at = 0; at < image->height; at++) {
	  rows[at] = color_data + 4 * image->width * at;
   }

   png_read_image(png_ptr, rows);

   canvas_image_dither(rows, image);

   free(rows);
   free(color_data);
} 

/*--------------------------------------------------------------------------*/
/* canvas_image_load                                                        */
/*--------------------------------------------------------------------------*/

void canvas_image_load(char *filename, IMAGE *image)
{
   FILE *png;
   char header[8];

   png = fopen(filename, "rb");
   if (png == NULL) {
      fprintf(stderr, "error opening `%s'\n", filename);
      exit(EXIT_FAILURE);
      return;
   }

   fread(header, 1, 8, png);
   if (png_sig_cmp(header, 0, 8)) {
      fclose(png);
      canvas_image_load_xpm(filename, image);
   } else {
      canvas_image_load_png(png, image);
      fclose(png);
   }
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
   XGCValues gcv;

   gcv.clip_x_origin = x;
   gcv.clip_y_origin = y;
   gcv.clip_mask = image->mask;
   XChangeGC(display, gc, GCClipXOrigin | GCClipYOrigin | GCClipMask, &gcv);
   XCopyArea(display, image->pixmap, canvas, gc, sx, sy, sw, sh, x, y);
   canvas_dirty(x, y, image->width, image->height);
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
