/*--------------------------------------------------------------------------*/
/* sprite management                                                        */
/*--------------------------------------------------------------------------*/

#ifndef __MY_SPRITE_H
#define __MY_SPRITE_H

#include "canvas.h"

#include <X11/Xlib.h>

/*--------------------------------------------------------------------------*/
/* defines                                                                  */
/*--------------------------------------------------------------------------*/

#define SPRITE_STOP         0           /* keep direction, reset stepping */

/*--------------------------------------------------------------------------*/
/* functions                                                                */
/*--------------------------------------------------------------------------*/

void sprite_set_path(char *path);
void sprite_add_state(int id, char *name);

void *sprite_load(char *name);
void *sprite_copy(void *_sprite, int full_copy);
void sprite_modify(void *_sprite, canvas_modify_func func);
void sprite_free(void *_sprite);

void sprite_set_state(void *_sprite, int state, int stepping);
int sprite_get_state(void *_sprite);
int sprite_get_stepping(void *_sprite);

void sprite_paint(void *sprite, int state, int x, int y);
void sprite_paint_clipped(void *sprite, int state, int x, int y, int sx, int sy, int sw, int sh);
void sprite_paint_clipped_mask(void *sprite, int state, void *mask_sprite, int mask_state, int x, int y, int sx, int sy, int sw, int sh);

IMAGE *sprite_get_image(void *_sprite, int state, int stepping);
int sprite_intersect(void *sprite1, void *sprite2, int x_off, int y_off);

#endif /* __MY_SPRITE_H */
