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
   Pixmap pixmap;
   Pixmap mask;
   long *colors;
   int num_colors;
} IMAGE;

/*--------------------------------------------------------------------------*/
/* types                                                                    */
/*--------------------------------------------------------------------------*/

typedef void (*canvas_modify_func)(int width, int height,
                                   unsigned short *pixmap, char *mask);

/*--------------------------------------------------------------------------*/
/* functions                                                                */
/*--------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

void canvas_init(Display *_display, Window _window, int width, int height);
void canvas_close(void);
void canvas_enable(int yes);

void canvas_install_colormap(int yes);

long canvas_alloc_color(int red, int green, int blue);

void canvas_clear(void);
void canvas_refresh(void);
void canvas_refresh_all(void);

void canvas_rectangle(int x, int y, int width, int height, long pixel);

void *canvas_font_load(char *name);
void canvas_font_size(char *msg, void *_font, int *width, int *height);
void canvas_font_print(char *msg, int x, int y, void *_font, long pixel);

void canvas_image_load(char *filename, IMAGE *image);
void canvas_image_copy(IMAGE *oldimg, IMAGE *newimg);
void canvas_image_modify(IMAGE *image, canvas_modify_func func);
void canvas_image_free(IMAGE *image);
void canvas_image_paint(IMAGE *image, IMAGE *mask, int x, int y, int sx, int sy, int sw, int sh);
int canvas_image_intersect(IMAGE *image1, IMAGE *image2, int x_off, int y_off);

void   canvas_key_event(XKeyEvent *event);
KeySym canvas_key_down(KeySym keysym);
char  *canvas_keysym_name(KeySym keysym);
char  *canvas_keycode_name(KeyCode keycode);

void canvas_mouse_init(int absolute);
void canvas_mouse_cursor(int on);
void canvas_mouse_event(XEvent *event);
int canvas_mouse_get(int *x, int *y);

void canvas_init_mouse(int absolute);
void canvas_drag_event(XMotionEvent *event);
void get_mouse_xy(int *x, int *y);
void canvas_button_event(XButtonEvent *event);
int get_mouse_button_pushed();

void canvas_init_cursor(int on);

#ifdef __cplusplus
}
#endif

/*--------------------------------------------------------------------------*/
/* Variables                                                                */
/*--------------------------------------------------------------------------*/

extern Colormap canvas_colormap;
extern long canvas_black_pixel;

#endif /* __MY_CANVAS_H */
