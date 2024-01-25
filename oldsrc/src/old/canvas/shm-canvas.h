/*--------------------------------------------------------------------------*/
/* canvas                                                                   */
/*--------------------------------------------------------------------------*/

#ifndef __MY_CANVAS_H
#define __MY_CANVAS_H

#include <X11/Xlib.h>

/*--------------------------------------------------------------------------*/
/* structures                                                               */
/*--------------------------------------------------------------------------*/

typedef struct {
   int width, height;
#if 0                                   /* stuff for USE_XIMAGE */
   char *pixels;
   char *not_mask;                      /* the "not" of the original mask */
#endif
   Pixmap pixmap;
   Pixmap mask;
} IMAGE;
   
/*--------------------------------------------------------------------------*/
/* functions                                                                */
/*--------------------------------------------------------------------------*/

void canvas_init(Display *_display, Window _window, int width, int height);
void canvas_close(void);

void canvas_clear(void);
void canvas_refresh(void);

void canvas_rectangle(int x, int y, int width, int height, long pixel);

void *canvas_font_load(char *name);
void canvas_font_size(char *msg, void *_font, int *width, int *height);
void canvas_font_print(char *msg, int x, int y, void *_font, long pixel);

void canvas_image_load(char *filename, IMAGE *image);
void canvas_image_free(IMAGE *image);
void canvas_image_paint(IMAGE *image, int x, int y, int sx, int sy, int sw, int sh);
void canvas_image_get(IMAGE *image, XImage *image);
void canvas_image_put(IMAGE *image, XImage *image);

void   canvas_key_event(XKeyEvent *event);
KeySym canvas_key_down(KeySym keysym);

/*--------------------------------------------------------------------------*/
/* Variables                                                                */
/*--------------------------------------------------------------------------*/

extern Colormap canvas_colormap;
extern long canvas_black_pixel;

#endif /* __MY_CANVAS_H */
