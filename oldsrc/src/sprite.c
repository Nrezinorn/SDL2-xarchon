/*--------------------------------------------------------------------------*/
/* sprite management                                                        */
/*--------------------------------------------------------------------------*/

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "sprite.h"
#include "canvas.h"
#include "list.h"

/*--------------------------------------------------------------------------*/
/* structures                                                               */
/*--------------------------------------------------------------------------*/

typedef struct {
   LIST_ELEM list_elem;
   int id;                              /* state id */
   char *name;
} STATE;

typedef struct {
   LIST_ELEM list_elem;
   int state, stepping;                 /* map identifier */
   IMAGE *image;                        /* if inode == -1, image points */
   int inode;                           /*   to the image of another map */
} MAP;

typedef struct {
   char *name;                          /* sprite name */
   int state, stepping;                 /* current state and stepping */
   LIST maps;                           /* all the maps of the sprite */
   int copy;                            /* if copy of another sprite */
} SPRITE;

/*--------------------------------------------------------------------------*/
/* functions                                                                */
/*--------------------------------------------------------------------------*/

static void load_state(LIST *maps, char *sprname, STATE *state);
static IMAGE *sprite_find_image(void *_sprite, int state);

/*--------------------------------------------------------------------------*/
/* variables                                                                */
/*--------------------------------------------------------------------------*/

static LIST *states = NULL;
static char *sprites_path = NULL;

/*--------------------------------------------------------------------------*/
/* sprite_set_path                                                          */
/*--------------------------------------------------------------------------*/

void sprite_set_path(char *path)
{
   if (sprites_path == NULL)
      sprites_path = malloc(PATH_MAX);
   strcpy(sprites_path, path);
}

/*--------------------------------------------------------------------------*/
/* sprite_add_state                                                         */
/*--------------------------------------------------------------------------*/

void sprite_add_state(int id, char *name)
{
   STATE *state;

   if (states == NULL) {
      states = malloc(sizeof(LIST));
      list_create(states);
   }
   state = list_insert_after(states, NULL, sizeof(STATE));
   state->id = id;
   state->name = strdup(name);
}

/*--------------------------------------------------------------------------*/
/* load_state                                                               */
/*--------------------------------------------------------------------------*/

static void load_state(LIST *maps, char *sprname, STATE *state)
{
   char *path;
   int stepping = 0;
   struct stat st;
   MAP *newmap, *oldmap;

   path = malloc(PATH_MAX);
   while (1) {
      sprintf(path, "%s/%s/%s.%d.xpm",
              sprites_path, sprname, state->name, stepping);
      if (stat(path, &st) == -1)
         break;
      newmap = list_insert_after(maps, NULL, sizeof(MAP));
      newmap->state = state->id;
      newmap->stepping = stepping;
      stepping++;
      for (oldmap = list_head(maps); oldmap != NULL; oldmap = list_next(oldmap))
         if (st.st_ino == oldmap->inode) {
            newmap->image = oldmap->image;
            newmap->inode = -1;
            break;
         }
      if (oldmap == NULL) {
         newmap->image = malloc(sizeof(IMAGE));
         canvas_image_load(path, newmap->image);
         newmap->inode = st.st_ino;
      }
   }
   free(path);
}

/*--------------------------------------------------------------------------*/
/* sprite_load                                                              */
/*--------------------------------------------------------------------------*/

void *sprite_load(char *name)
{
   SPRITE *sprite;
   STATE *state;

   sprite = malloc(sizeof(SPRITE));
   sprite->name = strdup(name);
   sprite->state = -1;                  /* dummy state */
   list_create(&sprite->maps);
   for (state = list_head(states); state != NULL; state = list_next(state))
      load_state(&sprite->maps, name, state);
   sprite->copy = 0;                    /* indicate non-copy sprite */
   return sprite;
}

/*--------------------------------------------------------------------------*/
/* sprite_copy                                                              */
/*--------------------------------------------------------------------------*/

void *sprite_copy(void *_sprite, int full_copy)
{
   SPRITE *sprite, *newspr;
   MAP *map, *newmap;

   sprite = (SPRITE *)_sprite;
   newspr = malloc(sizeof(SPRITE));
   newspr->name = sprite->name;
   newspr->state = -1;                  /* dummy state */
   list_create(&newspr->maps);
   for (map = list_head(&sprite->maps); map != NULL; map = list_next(map)) {
      newmap = list_insert_after(&newspr->maps, NULL, sizeof(MAP));
      newmap->state = map->state;
      newmap->stepping = map->stepping;
      if (full_copy) {
         newmap->image = malloc(sizeof(IMAGE));
         canvas_image_copy(map->image, newmap->image);
         newmap->inode = 0;             /* indicate image is "real" */
      } else {
         newmap->image = map->image;
         newmap->inode = -1;             /* indicate image is just a link */
      }
   }
   newspr->copy = 1;                    /* indicate name/maps are copied */
   return newspr;
}

/*--------------------------------------------------------------------------*/
/* sprite_modify                                                            */
/*--------------------------------------------------------------------------*/

void sprite_modify(void *_sprite, canvas_modify_func func)
{
   SPRITE *sprite;
   MAP *map;

   sprite = (SPRITE *)_sprite;
   for (map = list_head(&sprite->maps); map != NULL; map = list_next(map))
      canvas_image_modify(map->image, func);
}

/*--------------------------------------------------------------------------*/
/* sprite_free                                                              */
/*--------------------------------------------------------------------------*/

void sprite_free(void *_sprite)
{
   SPRITE *sprite;
   MAP *map;
   STATE *state;

   sprite = (SPRITE *)_sprite;
   if (!sprite->copy)                   /* only if name was not copied */
      free(sprite->name);
   for (map = list_head(&sprite->maps); map != NULL; map = list_delete(&sprite->maps, map))
      if (!sprite->copy)                /* only if maps were not copied */
         for (state = list_head(states); state != NULL; state = list_next(state))
            if (state->id == map->state) {
               if (map->inode != -1) {  /* if "real" map */
                  canvas_image_free(map->image);
                  free(map->image);
               }
               break;
            }
   free(sprite);
}

/*--------------------------------------------------------------------------*/
/* sprite_set_state                                                         */
/*--------------------------------------------------------------------------*/

void sprite_set_state(void *_sprite, int state, int stepping)
{
   if (state != SPRITE_STOP)
      ((SPRITE *)_sprite)->state = state;
   ((SPRITE *)_sprite)->stepping = stepping;
}

/*--------------------------------------------------------------------------*/
/* sprite_get_state                                                         */
/*--------------------------------------------------------------------------*/

int sprite_get_state(void *_sprite)
{
   return ((SPRITE *)_sprite)->state;
}

/*--------------------------------------------------------------------------*/
/* sprite_get_stepping                                                      */
/*--------------------------------------------------------------------------*/

int sprite_get_stepping(void *_sprite)
{
   return ((SPRITE *)_sprite)->stepping;
}

/*--------------------------------------------------------------------------*/
/* sprite_find_image                                                        */
/*--------------------------------------------------------------------------*/

IMAGE *sprite_find_image(void *_sprite, int state)
{
   SPRITE *sprite;
   MAP *map, *paintmap = NULL;

   sprite = (SPRITE *)_sprite;
   if (state != sprite->state) {
      if (state != SPRITE_STOP)
         sprite->state = state;
      sprite->stepping = 0;
   }
   for(map = list_head(&sprite->maps); map != NULL; map = list_next(map))
      if (map->state == sprite->state) {
         if (map->stepping == sprite->stepping) {  /* found our map */
            paintmap = map;
            break;
         }
         if (map->stepping == 0)        /* first map for state */
            paintmap = map;
      }
   if (paintmap == NULL) {
      fprintf(stderr, "sprite.c/sprite_paint(): requested state %d not found for sprite %s\n", sprite->state, sprite->name);
      exit(EXIT_FAILURE);
   }
   sprite->stepping = paintmap->stepping + 1;
   return paintmap->image;
}

/*--------------------------------------------------------------------------*/
/* sprite_paint                                                             */
/*--------------------------------------------------------------------------*/

void sprite_paint(void *sprite, int state, int x, int y)
{
   IMAGE *image;

   image = sprite_find_image(sprite, state);
   canvas_image_paint(image, image, x, y, 0, 0, image->width, image->height);
}

/*--------------------------------------------------------------------------*/
/* sprite_paint_clipped                                                     */
/*--------------------------------------------------------------------------*/

void sprite_paint_clipped(void *sprite, int state, int x, int y, int sx, int sy, int sw, int sh)
{
   IMAGE *image;

   image = sprite_find_image(sprite, state);
   canvas_image_paint(image, image, x, y, sx, sy, sw, sh);
}

/*--------------------------------------------------------------------------*/
/* sprite_paint_clipped_mask                                                */
/*--------------------------------------------------------------------------*/

void sprite_paint_clipped_mask(void *sprite, int state,
      void *mask_sprite, int mask_state, int x, int y, int sx, int sy,
      int sw, int sh)
{
   IMAGE *image, *mask_image;

   image = sprite_find_image(sprite, state);
   mask_image = sprite_find_image(mask_sprite, mask_state);
   canvas_image_paint(image, mask_image, x, y, sx, sy, sw, sh);
}

/*--------------------------------------------------------------------------*/
/* sprite_get_image                                                         */
/*--------------------------------------------------------------------------*/

IMAGE *sprite_get_image(void *_sprite, int state, int stepping)
{
   SPRITE *sprite;
   MAP *map;

   sprite = (SPRITE *)_sprite;
   for(map = list_head(&sprite->maps); map != NULL; map = list_next(map))
      if (map->stepping == stepping && map->state == state)
            return map->image;
   fprintf(stderr, "sprite.c/sprite_pixmap(): requested state %d stepping %d not found for sprite %s\n", state, stepping, sprite->name);
   exit(EXIT_FAILURE);
}

/*--------------------------------------------------------------------------*/
/* sprite_intersect                                                         */
/*--------------------------------------------------------------------------*/

int sprite_intersect(void *sprite1, void *sprite2, int x_off, int y_off)
{
   IMAGE *image1, *image2;

   image1 = sprite_find_image(sprite1, sprite_get_state(sprite1));
   image2 = sprite_find_image(sprite2, sprite_get_state(sprite2));
   return canvas_image_intersect(image1, image2, x_off, y_off);
}


