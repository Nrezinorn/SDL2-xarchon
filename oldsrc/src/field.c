/*--------------------------------------------------------------------------*/
/* battle field                                                             */
/*--------------------------------------------------------------------------*/

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "main.h"
#include "field.h"
#include "board.h"
#include "sprite.h"
#include "canvas.h"
#include "iface.h"
#include "audio.h"

/*--------------------------------------------------------------------------*/
/* defines                                                                  */
/*--------------------------------------------------------------------------*/

#define FONT_NAME      "-misc-fixed-medium-*-normal-*-40-0-*-*-*-*-iso8859-1"

#define NUM_ROCKS      12
#define ROCK_DELAY     2                /* rock slowdown factor */
#define ROCK_BUMP_MAX  6                /* max dist per frame when bounced */
#define ROCK_FRAMES    (FPS * 5)        /* frames between luminance change */

#define BAR_WIDTH      15               /* life and ammo bar sizes */
#define LIFE_BAR       0                /* to make field_paint_bar paint */
#define AMMO_BAR       1                /*    the life or ammo bars */

#define BLINK_FRAMES   2                /* number of frames to show/hide */
#define NUM_BLINKS     20               /* number of blinks to make */

#define CLOUD_FACTOR   ((int) (2*TIME_SCALE)) /* damage every x frames */

#define PROLOG_FRAMES  75               /* length in frames of prolog */
#define EPILOG_FRAMES  50               /* length in frames of epilog */

#define ROCK_OK         0               /* returns from field_rock_collision */
#define ROCK_SLOW       1
#define ROCK_BUMP       2

#define STRIPE_MAX      5               /* the dimensions of an array holding */
#define DIAGONAL_MAX    3               /* cloud sprite direction data */

#define SSP_DELAY       10              /* number of ticks between the */
                                        /* phases of the ssp (slow sprite */
                                        /* paint - shapeshifter effect) */

/*--------------------------------------------------------------------------*/
/* structures                                                               */
/*--------------------------------------------------------------------------*/

typedef struct {
   int x, y;                            /* rock position */
   int lumi, lumi_d;                    /* luminance value and direction */
} ROCK;

/* TODO: keep track of where each actor starts on the field                 */
/* (using field data, defines, config data, or whatever)                    */

typedef struct {
   FIELD_ACTOR light, dark;             /* the duelling actors */
   FIELD_ACTOR light_weapon, dark_weapon;   /* ... and their weapons */
   ROCK rocks[NUM_ROCKS];               /* rocks on the field */
   int cell_x, cell_y;                  /* (x,y) of disputed cell */
   int prolog_countdown;                /* if displaying prolog */
   int epilog_countdown;                /* if displaying epilog */
   int repaint_life;                    /* life probably needs repainting */
   int repaint_ammo;                    /* ammo probably needs repainting */
   int any_output;                      /* if any output emitted */
} FIELD_DATA;

/*--------------------------------------------------------------------------*/
/* variables                                                                */
/*--------------------------------------------------------------------------*/

unsigned char field_cells[FIELD_YCELLS][FIELD_XCELLS];
FIELD_ACTOR *field_me, *field_he;
int field_frame_time;

static ACTOR mutual_death = { };        /* used to indicate a double death */

static FIELD_DATA field = { };

static int back_color, light_color, dark_color, life_color, ammo_color;
static void *font = NULL;

static int *multiply_by_root2;

static int cloud_shape[3][STRIPE_MAX][DIAGONAL_MAX] = {
   {  { 0, 0, 0 },
      { 0, 0, 0 },
      { 0, STATE_FIRE_ANY, 0 },
      { 0, 0, 0 },
      { 0, 0, 0 } },
   {  { 0, 0, 0 },
      { STATE_FIRE_LEFT, STATE_FIRE_UP, 0 },
      { 0, STATE_FIRE_ANY, 0 },
      { STATE_FIRE_DOWN, STATE_FIRE_RIGHT, 0 },
      { 0, 0, 0 } },
   {  { STATE_FIRE_LEFT, STATE_FIRE_UP_LEFT, STATE_FIRE_UP },
      { STATE_FIRE_ANY, STATE_FIRE_ANY, 0 },
      { STATE_FIRE_DOWN_LEFT, STATE_FIRE_ANY, STATE_FIRE_UP_RIGHT },
      { STATE_FIRE_ANY, STATE_FIRE_ANY, 0 },
      { STATE_FIRE_DOWN, STATE_FIRE_DOWN_RIGHT, STATE_FIRE_RIGHT } } };

/*--------------------------------------------------------------------------*/
/* functions                                                                */
/*--------------------------------------------------------------------------*/

static void field_setup_rocks(CELL *cell);
static void field_paint_cell(int x, int y);
static void field_paint_bar(int ammo, FIELD_ACTOR *fa);
static void field_paint_actor(FIELD_ACTOR *fa);
static void field_paint_weapon(FIELD_ACTOR *fw);
static int field_clear_actor(FIELD_ACTOR *fa, FIELD_ACTOR *ofa);
static void field_ssp(FIELD_ACTOR *fa);
static void field_init(void);
static void field_image_dim_func(int width, int height, unsigned short *pixmap, char *mask);
static void field_setup_actor(FIELD_ACTOR *fa, ACTOR *actor, CELL *cell, ACTOR *enemy);
static int field_rock_collision(FIELD_ACTOR *fa, int x, int y, ROCK *rock);
static void field_animate(FIELD_ACTOR *fa);
static int field_x_step(int start_x, int direction, int speed);
static int field_y_step(int start_y, int direction, int speed);
static int field_obstacle_bump(FIELD_ACTOR *fa, int dx, int dy);
static int field_player_frozen(FIELD_ACTOR *fa);
static void field_player(FIELD_ACTOR *fa);
static void field_animate_weapon(FIELD_ACTOR *fa);
static int field_weapon_collision(FIELD_ACTOR *fw);
static void field_weapon(FIELD_ACTOR *fa);
static void field_lumi_cycle(void);
static void field_print(char *msg, int row, int color);
static void field_prolog_epilog_move(FIELD_ACTOR *fa);
static void field_prolog(void);
static ACTOR *field_epilog(void);
static void field_frame_paint(FIELD_DATA *old_field);

/*--------------------------------------------------------------------------*/
/* field_setup_rocks                                                        */
/*--------------------------------------------------------------------------*/

void field_setup_rocks(CELL *cell)
{
   ROCK *rocks;
   int i, j;

   if (cell->rocks == NULL)
      cell->rocks = malloc(sizeof(field.rocks));
   rocks = cell->rocks;

   for (i = 0; i < NUM_ROCKS; i++) {
      rocks[i].x = main_random() % FIELD_XCELLS;
      rocks[i].y = main_random() % FIELD_YCELLS;
      if ((rocks[i].x == 0 && rocks[i].y == FIELD_YCELLS/2) ||
            (rocks[i].x == FIELD_XCELLS - 1 &&
            rocks[i].y == FIELD_YCELLS/2 + 1)) {
         /* don't have rocks appear in the two starting locations */
         i--;
         continue;
      }
      rocks[i].lumi = (i % 6 == 0) ? ROCK_WALKABLE :
                        (i % 2 == 0) ? ROCK_DARKEST : ROCK_LIGHTEST;
      rocks[i].lumi_d = (rocks[i].lumi == ROCK_DARKEST) ? 1 :
                        (rocks[i].lumi == ROCK_LIGHTEST) ? -1 :
                        (i % 2 == 0) ? 1 : -1;
      for (j = 0; j < i; j++)
         if (rocks[i].x >= rocks[j].x - 1 && rocks[i].x <= rocks[j].x + 1 &&
               rocks[i].y >= rocks[j].y - 1 && rocks[i].y <= rocks[j].y + 1) {
            i--;
            break;
         }
   }
}

/*--------------------------------------------------------------------------*/
/* field_paint_cell                                                         */
/*--------------------------------------------------------------------------*/

void field_paint_cell(int x, int y)
{
   int i;

   if (x < 0 || x >= FIELD_XCELLS || y < 0 || y >= FIELD_YCELLS) {
      canvas_rectangle(FIELD_X(x), FIELD_Y(y), CELL_XSIZE, CELL_YSIZE, back_color);
      field.repaint_life = 1;
      field.repaint_ammo = 1;
      return;
   }

   i = field_cells[y][x] >> ROCK_IX_SHIFT;
   if (i == ROCK_NONE)
      i = ROCK_LIGHTEST;
   else
      i = field.rocks[i].lumi;
   sprite_set_state(floor_sprite, STATE_FIELD, i);
   sprite_paint(floor_sprite, STATE_FIELD, FIELD_X(x), FIELD_Y(y));
   field.any_output = 1;
}

/*--------------------------------------------------------------------------*/
/* field_paint_bar                                                          */
/*--------------------------------------------------------------------------*/

void field_paint_bar(int ammo, FIELD_ACTOR *fa)
{
   int offset, height, color;

   if (ammo) {
      offset = BAR_WIDTH + 1;
      height = vmax(0, fa->weapon->actor->recharge - (field_frame_time - fa->weapon->fire_time));
      color = ammo_color;
   } else {
      offset = 0;
      height = fa->life;
      color = life_color;
   }
   if (fa == &field.dark)
      offset = CANVAS_WIDTH - BAR_WIDTH - offset;
   canvas_rectangle(offset, 0, BAR_WIDTH - 1, CANVAS_HEIGHT - height, back_color);
   canvas_rectangle(offset, CANVAS_HEIGHT - height, BAR_WIDTH - 1, height, color);
   field.any_output = 1;
}

/*--------------------------------------------------------------------------*/
/* field_paint_actor                                                        */
/*--------------------------------------------------------------------------*/

void field_paint_actor(FIELD_ACTOR *fa)
{
   int state;

   fa->redraw = 0;
   field.any_output = 1;                /* output will be made here */

   if (fa->orig_actor != fa->actor &&
         field.prolog_countdown + field.epilog_countdown != 0) {
      field_ssp(fa);
      return;
   }

   if ((!fa->blink || (field_frame_time / BLINK_FRAMES) % 2 == 0) &&
         (fa->weapon->state == 0 ||
         (fa->weapon->actor->type & ACTOR_IMMUNE_USER) != ACTOR_IMMUNE_USER)) {
      state = SPRITE_STOP; 
      if (fa->actor->type & ACTOR_ELEMENTAL) {
         if (fa->state != 0)
            state = sprite_get_state(fa->sprite);
      } else {
         if (fa->weapon->state != 0 &&
               !WAS_TIME(fa->weapon, fa->actor->duration))
            state = fa->fire_state - STATE_MOVE_FIRST + STATE_FIRE_FIRST;
         else
            state = fa->state;
      }
      if (fa->blink || fa->inanimate)
         sprite_set_state(fa->sprite, state, 0);
      sprite_paint(fa->sprite, state, fa->x + FIELD_X(0), fa->y + FIELD_Y(0));
   }
}

/*--------------------------------------------------------------------------*/
/* field_paint_weapon                                                       */
/*--------------------------------------------------------------------------*/

void field_paint_weapon(FIELD_ACTOR *fw)
{
   int cloud_grow, cloud_shrink;
   int stripe, diag;
   int cx, cy;

   fw->redraw = 0;

   if (fw->state == 0)
      return;

   if ((fw->actor->type & ACTOR_WEAPON_CLOUD) != ACTOR_WEAPON_CLOUD) {
      sprite_set_state(fw->sprite,
            fw->state - STATE_MOVE_FIRST + STATE_FIRE_FIRST, 0);
      sprite_paint(fw->sprite,
                   fw->state - STATE_MOVE_FIRST + STATE_FIRE_FIRST,
                   fw->x + FIELD_X(0), fw->y + FIELD_Y(0));
      return;
   }
   cloud_grow = fw->actor->cloud_grow;
   cloud_shrink = fw->actor->duration - fw->actor->cloud_grow;
   if (WAS_TIME(fw, 2*cloud_grow) && !WAS_TIME(fw, cloud_shrink - cloud_grow))
      fw->state = 3;
   else if (WAS_TIME(fw, cloud_grow) && !WAS_TIME(fw, cloud_shrink))
      fw->state = 2;
   else
      fw->state = 1;
   for (stripe = 0; stripe < STRIPE_MAX; stripe++) {
      cx = fw->x + FIELD_X((stripe - 4)/2);
      cy = fw->y + FIELD_Y(stripe/2);
      for (diag = 0; diag < DIAGONAL_MAX; diag++, cx += CELL_X(1),
            cy -= CELL_Y(1))
         if (cloud_shape[fw->state - 1][stripe][diag])
            sprite_paint(fw->sprite, cloud_shape[fw->state - 1][stripe][diag],
                  cx, cy);
   }
}

/*--------------------------------------------------------------------------*/
/* field_clear_actor                                                        */
/*--------------------------------------------------------------------------*/

int field_clear_actor(FIELD_ACTOR *fa, FIELD_ACTOR *ofa)
{
   FIELD_ACTOR *fw;
   int x, y;
   int cell_x, cell_y;
   int other_cleared;

   cell_x = fa->x / CELL_XSIZE;
   cell_y = fa->y / CELL_YSIZE;
   x = CELL_X(cell_x);
   y = CELL_Y(cell_y);
   field_paint_cell(cell_x, cell_y);
   other_cleared = (ofa != NULL && field_collision(x, y, ofa->x, ofa->y));
   if (CELL_XOFFSET(fa->x) != 0) {
      field_paint_cell(cell_x + 1, cell_y);
      other_cleared |= (ofa != NULL &&
            field_collision(x + CELL_XSIZE, y, ofa->x, ofa->y));
   }
   if (CELL_YOFFSET(fa->y) != 0) {
      field_paint_cell(cell_x, cell_y + 1);
      other_cleared |= (ofa != NULL &&
            field_collision(x, y + CELL_YSIZE, ofa->x, ofa->y));
      if (CELL_XOFFSET(fa->x) != 0) {
	 field_paint_cell(cell_x + 1, cell_y + 1);
	 other_cleared |= (ofa != NULL &&
	       field_collision(x + CELL_XSIZE, y + CELL_YSIZE, ofa->x, ofa->y));
      }
   }

   fw = fa->weapon;
   if (fw == NULL || fw->state == 0)
      return other_cleared;
   if ((fw->actor->type & ACTOR_WEAPON_CLOUD) == ACTOR_WEAPON_CLOUD) {
      for (y = cell_y - fw->state + 1; y <= cell_y + fw->state; y++)
         for (x = cell_x - fw->state + 1; x <= cell_x + fw->state; x++)
            if (x >= 0 && x < FIELD_XCELLS && y >= 0 && y < FIELD_YCELLS) {
               field_paint_cell(x, y);
               other_cleared |= (ofa != NULL &&
                     field_collision(CELL_X(x), CELL_Y(y), ofa->x, ofa->y));
            } else {
               canvas_rectangle(FIELD_X(x), FIELD_Y(y), CELL_X(1), CELL_Y(1),
                     back_color);
               if (x < 0 || x >= FIELD_XCELLS) {
                  field.repaint_life = 1;
                  field.repaint_ammo = 1;
               }
            }
   }
   else
      other_cleared |= field_clear_actor(fw, ofa);
   return other_cleared;
}

/*--------------------------------------------------------------------------*/
/* field_ssp                                                                */
/*--------------------------------------------------------------------------*/

void field_ssp(FIELD_ACTOR *fa)
{
   void *orig = NULL, *to = NULL;
   int tick = 0;
   int x, y;
   int to_h, from_h;

   orig = fa->orig_actor->sprite;
   to = fa->sprite;
   if (field.prolog_countdown != 0)
      tick = PROLOG_FRAMES - field.prolog_countdown;
   else /* field.epilog_countdown != 0 */
      tick = field.epilog_countdown;

   x = fa->x + FIELD_X(0);
   y = fa->y + FIELD_Y(0);
   if (tick <= SSP_DELAY) {
      sprite_paint(orig, SPRITE_STOP, x, y);
   } else if (tick < 3*SSP_DELAY) {
      to_h = tick - SSP_DELAY;
      from_h = (CELL_YSIZE - to_h)/2;
      sprite_paint_clipped(orig, SPRITE_STOP, x, y, 0, 0, CELL_XSIZE, from_h);
      sprite_paint_clipped_mask(orig, SPRITE_STOP, to, SPRITE_STOP,
            x, y + from_h, 0, from_h, CELL_XSIZE, to_h);
      sprite_paint_clipped(orig, SPRITE_STOP, x, y + from_h + to_h,
            0, from_h + to_h, CELL_XSIZE, from_h);
   } else if (tick < 3*SSP_DELAY + CELL_YSIZE) {
      to_h = tick - SSP_DELAY;
      from_h = (CELL_YSIZE - to_h)/2;
      sprite_paint_clipped(orig, SPRITE_STOP, x, y, 0, 0, CELL_XSIZE, from_h);
      sprite_paint_clipped_mask(orig, SPRITE_STOP, to, SPRITE_STOP,
            x, y + from_h, 0, from_h, CELL_XSIZE, to_h);
      sprite_paint_clipped(orig, SPRITE_STOP, x, y + from_h + to_h,
            0, from_h + to_h, CELL_XSIZE, from_h);
      to_h = tick - 3*SSP_DELAY;
      from_h = (CELL_YSIZE - to_h)/2;
      sprite_paint_clipped(to, SPRITE_STOP, x, y + from_h, 0, from_h,
         CELL_XSIZE, to_h);
   } else
      sprite_paint(to, SPRITE_STOP, x, y);
}

/*--------------------------------------------------------------------------*/
/* field_init                                                               */
/*--------------------------------------------------------------------------*/

void field_init(void)
{
   int value;
   double root2;

   if (font != NULL)
      return;
   back_color = canvas_alloc_color(0, 0, 0);
   light_color = canvas_alloc_color(255, 255, 0);
   dark_color = canvas_alloc_color(0, 0, 255);
   life_color = canvas_alloc_color(0, 255, 0);
   ammo_color = canvas_alloc_color(255, 0, 0);
   font = canvas_font_load(FONT_NAME);
   root2 = sqrt(2.0);
   multiply_by_root2 = malloc(sizeof(int)*CELL_XSIZE*2);
   for (value = 0; value < CELL_XSIZE*2; value++)
      multiply_by_root2[value] = (int) (root2*value);
}

/*--------------------------------------------------------------------------*/
/* field_image_dim_func                                                     */
/*--------------------------------------------------------------------------*/

void field_image_dim_func(int width, int height, unsigned short *pixmap, char *mask)
{
   int x, y;
   unsigned short *p;
   int r, g, b;

   for (y = 0; y < height; y++) {
      p = pixmap + y * width * 3;
      for (x = 0; x < width; x++) {
         r = *(p + 0);
         g = *(p + 1);
         b = *(p + 2);
         r = vmax(0, r - 20000);
         g = vmax(0, g - 20000);
         b = vmax(0, b - 20000);
         *(p + 0) = r;
         *(p + 1) = g;
         *(p + 2) = b;
         p += 3;
      }
   }
}

/*--------------------------------------------------------------------------*/
/* field_setup_actor                                                        */
/*--------------------------------------------------------------------------*/

void field_setup_actor(FIELD_ACTOR *fa, ACTOR *actor, CELL *cell, ACTOR *enemy)
{
   fa->orig_actor = actor;
   if (fa == &field.light)
      fa->weapon = &field.light_weapon;
   else
      fa->weapon = &field.dark_weapon;

   /* the shapeshifter needs to take its enemy's sprite during battle */
   if ((actors_list[actor->weapon].type & ACTOR_WEAPON_CLONE) == ACTOR_WEAPON_CLONE) {
      fa->actor = &actors_list[enemy->type & ACTOR_MASK];
#ifndef AUTOPILOT
      fa->sprite = sprite_copy(actors_list[(enemy->type & ACTOR_MASK)].sprite, 1);
      sprite_modify(fa->sprite, field_image_dim_func);
#else
      fa->sprite = fa->actor->sprite;
#endif
      if (fa->actor->type & ACTOR_ELEMENTAL)
         sprite_set_state(fa->sprite, sprite_get_state(field.light.sprite), 0);
      else
         sprite_set_state(fa->sprite, STATE_MOVE_LEFT, 0);
      fa->weapon->actor = &actors_list[enemy->weapon];
#ifndef AUTOPILOT
      fa->weapon->sprite = sprite_copy(actors_list[enemy->weapon].sprite, 1);
      sprite_modify(fa->weapon->sprite, field_image_dim_func);
#else
      fa->weapon->sprite = fa->weapon->actor->sprite;
#endif

   } else {
      /* non-shapeshifters have their own sprites */
      fa->actor = actor;
      fa->sprite = fa->actor->sprite;
      if (!(fa->actor->type & ACTOR_ELEMENTAL))
         sprite_set_state(fa->sprite, (fa->actor->type & ACTOR_RIGHT ? STATE_MOVE_RIGHT : STATE_MOVE_LEFT), 0);
      fa->weapon->actor = &actors_list[actor->weapon];
      fa->weapon->sprite = fa->weapon->actor->sprite;
   }

   fa->x = field.cell_x;
   fa->y = field.cell_y;
   fa->x1 = CELL_X(actor->type & ACTOR_LIGHT ? 0 : FIELD_XCELLS - 1);
   fa->y1 = CELL_Y(FIELD_YCELLS / 2 +
         ((actor->type & ACTOR_LIGHT) ? -0.5 : 0.5));
   fa->state = 0;
   fa->blink = 0;
   fa->fire_time = -10000;
   fa->fire_state = 0;
   fa->num_hits = 0;
   fa->life = field_initial_life(actor, cell, enemy);
   fa->orig_life = fa->life;
   /* fa->state = sprite_get_state(fa->sprite); */
   fa->inanimate = 1;

   fa->weapon->x = -1;
   fa->weapon->y = 0;
   fa->weapon->state = 0;
   fa->weapon->fire_time = fa->fire_time;
   fa->weapon->redraw = 0;
   fa->weapon->weapon = NULL;
}

/*--------------------------------------------------------------------------*/
/* field_start_game                                                         */
/*--------------------------------------------------------------------------*/

void field_start_game(ACTOR *_light, ACTOR *_dark, CELL *cell, int cx, int cy)
{
   int i;

   field_init();
   field_frame_time = 0;

   field.prolog_countdown = PROLOG_FRAMES;
   field.epilog_countdown = 0;
   field.repaint_life = 0;
   field.repaint_ammo = 0;
   field.cell_x = cx - FIELD_X(0);
   field.cell_y = cy - FIELD_Y(0);

   field_setup_actor(&field.light, _light, cell, _dark);
   field_setup_actor(&field.dark, _dark, cell, _light);

   if (cell->rocks == NULL)
      field_setup_rocks(cell);
   memcpy(&field.rocks, cell->rocks, sizeof(field.rocks));
   memset(field_cells, ROCK_NONE << ROCK_IX_SHIFT, sizeof(field_cells));
   for (i = 0; i < NUM_ROCKS; i++)
      field_cells[field.rocks[i].y][field.rocks[i].x] =
         (i << ROCK_IX_SHIFT) + field.rocks[i].lumi;

   canvas_clear();
   field_refresh();

   audio_start_battle(field.light.actor, field.dark.actor);
}

/*--------------------------------------------------------------------------*/
/* field_end_game                                                           */
/*--------------------------------------------------------------------------*/

void field_end_game(void)
{
#ifndef AUTOPILOT
   if (field.dark.actor != field.dark.orig_actor) {
      sprite_free(field.dark.sprite);
      sprite_free(field.dark.weapon->sprite);
   }
#endif
}

/*--------------------------------------------------------------------------*/
/* field_collision                                                          */
/*--------------------------------------------------------------------------*/

int field_collision(int x1, int y1, int x2, int y2)
{
   return (x2 > x1 - CELL_XSIZE && x2 < x1 + CELL_XSIZE &&
         y2 > y1 - CELL_YSIZE && y2 < y1 + CELL_YSIZE);
}

/*--------------------------------------------------------------------------*/
/* field_rock_collision                                                     */
/*--------------------------------------------------------------------------*/

int field_rock_collision(FIELD_ACTOR *fa, int x, int y, ROCK *rock)
{
   int rock_x, rock_y;

   if (!rock || rock->lumi == ROCK_LIGHTEST)
      /* walk freely */
      return ROCK_OK;
   rock_x = CELL_X(rock->x);
   rock_y = CELL_Y(rock->y);
   /* test if we're actually touching the rock */
   sprite_set_state(floor_sprite, STATE_FIELD, 0);
   if (sprite_get_state(fa->sprite) == -1) {
      if ((fa->actor->type & ACTOR_WEAPON) == ACTOR_WEAPON)
         sprite_set_state(fa->sprite,
               fa->state - STATE_MOVE_FIRST + STATE_FIRE_FIRST, 0);
      else
         sprite_set_state(fa->sprite, fa->state, 0);
   }
   if (rock_x < x) {
      if (!sprite_intersect(floor_sprite, fa->sprite, x - rock_x, y - rock_y))
         return ROCK_OK;
   } else {
      if (!sprite_intersect(fa->sprite, floor_sprite, rock_x - x, rock_y - y))
         return ROCK_OK;
   }
   if (rock->lumi >= ROCK_WALKABLE)
      return ROCK_SLOW;
   else
      return ROCK_BUMP;
}

/*--------------------------------------------------------------------------*/
/* field_direction                                                          */
/*--------------------------------------------------------------------------*/

int field_direction(int to_x, int to_y, int from_x, int from_y)
{
  int dx, dy;
  int grad;
  
  dx = to_x - from_x;
  dy = to_y - from_y;
  if (dx == 0) {
    if (dy > 0)
      return STATE_MOVE_DOWN;
    else if (dy < 0) 
      return STATE_MOVE_UP;
    else
      return 0;
  } 
  else {
    grad = 2 * dy / dx;
    if (grad <= -4 || 4 <= grad) {
      if (dy > 0)
	return STATE_MOVE_DOWN;
      else
	return STATE_MOVE_UP;
    } 
    else if (grad >= 1) {
      if (dx > 0)
	return STATE_MOVE_DOWN_RIGHT;
      else
	return STATE_MOVE_UP_LEFT;
    } 
    else if (grad >= -1) {
      if (dx > 0)
	return STATE_MOVE_RIGHT;
      else
	return STATE_MOVE_LEFT;
    } 
    else {
      if (dx > 0)
	return STATE_MOVE_UP_RIGHT;
      else
	return STATE_MOVE_DOWN_LEFT;
    }
  }
}

/*--------------------------------------------------------------------------*/
/* field_animate                                                            */
/*--------------------------------------------------------------------------*/

void field_animate(FIELD_ACTOR *fa)
{
  int dx = 0;
  int dy = 0;

  if (!fa->blink && fa->state != 0) {
    dx = field_x_step(fa->x, fa->state, fa->actor->speed);
    dy = field_y_step(fa->y, fa->state, fa->actor->speed);
    fa->x += dx;
    fa->y += dy;
  }

  /* move actor off of any obstacles it's on */
  if ( field_obstacle_bump(fa, dx, dy) && (fa->actor->type & ACTOR_WEAPON) ) {
    /* weapons stop if they hit anything */
    fa->state = 0;
  }
}

/*--------------------------------------------------------------------------*/
/* field_x_step                                                             */
/*--------------------------------------------------------------------------*/

int field_x_step(int start_x, int direction, int speed)
{
  int x, unscaled_x;

   if (state_move_x_step[direction] && state_move_y_step[direction]) {
     /* moving diagonally - correct by sqrt(2) */
     x = CELL_XOFFSET(start_x);
     /* get the position we'd be at if we didn't correct */
     unscaled_x = multiply_by_root2[x];
     /* add the actor's (horizontal) speed */
     unscaled_x += state_move_x_step[direction] * speed;
     if (unscaled_x < 0) {
       unscaled_x += CELL_XSIZE;
       x += multiply_by_root2[CELL_XSIZE + 1]>>1;
     } else if (unscaled_x > CELL_XSIZE) {
       unscaled_x -= CELL_XSIZE;
       x -= multiply_by_root2[CELL_XSIZE + 1]>>1;
     }
     /* correct the final position, and find the diff. from the original pos */
     return ( (multiply_by_root2[unscaled_x + 1]>>1) - x );
   } 
   else {
     return ( state_move_x_step[direction] * speed );
   }
}

/*--------------------------------------------------------------------------*/
/* field_y_step                                                             */
/*--------------------------------------------------------------------------*/

int field_y_step(int start_y, int direction, int speed)
{
  int y, unscaled_y;

  if (state_move_x_step[direction] && state_move_y_step[direction]) {
      y = CELL_YOFFSET(start_y);
      unscaled_y = multiply_by_root2[y] +
	    state_move_y_step[direction] * speed;
      if (unscaled_y < 0) {
	 unscaled_y += CELL_YSIZE;
	 y += multiply_by_root2[CELL_YSIZE + 1]>>1;
      } else if (unscaled_y > CELL_YSIZE) {
	 unscaled_y -= CELL_YSIZE;
	 y -= multiply_by_root2[CELL_YSIZE + 1]>>1;
      }
      return ( (multiply_by_root2[unscaled_y + 1]>>1) - y );
   } 
  else {
      return ( state_move_y_step[direction] * speed );
   }
}

/*--------------------------------------------------------------------------*/
/* field_obstacle_bump                                                      */
/*--------------------------------------------------------------------------*/

int field_obstacle_bump(FIELD_ACTOR *fa, int dx, int dy)
{
  int x,y;
  int cell_x, cell_y;
  int test_w, test_h;
  int rock, on_rock;
  int rockdir;
  int bumped = 0;
  int moved = 0;

   x = fa->x;
   y = fa->y;

   if (x < 0) {
     x = 0;
     bumped = 1;
   } else if (x > CELL_X(FIELD_XCELLS - 1)) {
     x = CELL_X(FIELD_XCELLS - 1);
     bumped = 1;
   }

   if (y < 0) {
     y = 0;
     bumped = 1;
   } else if (y > CELL_Y(FIELD_YCELLS - 1)) {
     y = CELL_Y(FIELD_YCELLS - 1);
     bumped = 1;
   }

   if (bumped) {
     fa->x = x;
     fa->y = y;
     return bumped;
   }

   test_h = (CELL_YOFFSET(y) > 0) ? 2 : 1;
   for (cell_y = y/CELL_YSIZE; test_h > 0; test_h--, cell_y++) {
      test_w = (CELL_XOFFSET(x) > 0) ? 2 : 1;
      for (cell_x = x/CELL_XSIZE; test_w > 0; test_w--, cell_x++) {
         rock = field_cells[cell_y][cell_x] >> ROCK_IX_SHIFT;
         if (rock != ROCK_NONE) {
            on_rock = field_rock_collision(fa, x, y, &field.rocks[rock]);
            if (on_rock == ROCK_SLOW && !(fa->actor->type & ACTOR_WEAPON)) {
               /* the rock slows fa down, unless it's a weapon */
	       x -= dx/ROCK_DELAY;
               y -= dy/ROCK_DELAY;
            } 
	    else {
	      rockdir = field_direction(x, y,
					CELL_X(field.rocks[rock].x),
					CELL_Y(field.rocks[rock].y));
	      while (on_rock == ROCK_BUMP && moved <= ROCK_BUMP_MAX) {
		/* the rock bounces you away from itself */
		moved++;
		fa->x = x + field_x_step(x, rockdir, moved);
		fa->y = y + field_y_step(y, rockdir, moved);
		on_rock = field_rock_collision(fa, fa->x, fa->y, 
					       &field.rocks[rock]);
	      }
	      if (moved)
		return 1;
	    }
	 }
      }
   }

   fa->x = x;
   fa->y = y;
   return bumped;
}

/*--------------------------------------------------------------------------*/
/* field_player_frozen                                                      */
/*--------------------------------------------------------------------------*/

int field_player_frozen(FIELD_ACTOR *fa)
{
   FIELD_ACTOR *fw;

   fw = fa->weapon;

   /* if blinking */
   if (fa->blink) {
      fa->blink++;
      if (fa->blink == NUM_BLINKS) {
         fa->blink = 0;
         if (fa->life <= 0)
            field.epilog_countdown = EPILOG_FRAMES;
         else
            fa->state = 0;
      }
      return 1;
   }

   if (fw->state != 0 &&
         (fw->actor->type & ACTOR_FREEZE_USER) == ACTOR_FREEZE_USER)
      /* certain weapons render their user immobile while they're going */
      /* e.g. hand weapons (light knight/dark goblin), phoenix cloud */
      return 1;

   /* if recently fired a shoot weapon */
   if ((fw->actor->type & ACTOR_WEAPON_SHOOT) == ACTOR_WEAPON_SHOOT &&
       fa->fire_state != 0) {
      if (!WAS_TIME(fa, fa->actor->duration))
         return 1;
      else {                            /* if the "shooting-freeze" ended */
         fa->fire_state = 0;
         fa->redraw = 1;
      }
   }

   return 0;
}

/*--------------------------------------------------------------------------*/
/* field_player                                                             */
/*--------------------------------------------------------------------------*/

void field_player(FIELD_ACTOR *fa)
{
   int state;
   int fire_down;

   field_animate(fa);
   if (field_player_frozen(fa)) {
      fa->state = 0;
      return;
   }

   field_me = fa;
   field_he = (fa == &field.dark) ? &field.light : &field.dark;
   iface_turn((fa == &field.dark), IFACE_FIELD);
   iface_frame();
   if (!board_pause_game(-1))
      return;
   fire_down = iface_key_down(STATE_FIRE);
   fa->state = 0;

   if ((fa->weapon->actor->type & ACTOR_WEAPON_CLOUD) == ACTOR_WEAPON_CLOUD &&
       fire_down) {
      if (fa->weapon->state == 0 &&
          WAS_TIME(fa->weapon, fa->weapon->actor->recharge)) {
         fa->weapon->state = 1;
         fa->weapon->fire_time = field_frame_time;
         fa->weapon->x = fa->x;
         fa->weapon->y = fa->y;
      }
      fire_down = 0;
   }

   for (state = STATE_MOVE_LAST; state >= STATE_MOVE_FIRST; state--)
      if (iface_key_down(state)) {
         if (!fire_down)
            fa->state = state;
         else
            if (fa->weapon->state == 0 &&
                WAS_TIME(fa->weapon, fa->weapon->actor->recharge)) {
               fa->state = 0;
               fa->fire_state = state;
               fa->weapon->state = state;
               fa->weapon->fire_time = field_frame_time;
               fa->weapon->x  = fa->x;
               fa->weapon->x1 = fa->x;
               fa->weapon->y  = fa->y;
               fa->weapon->y1 = fa->y;
            }
         break;
      }
}

/*--------------------------------------------------------------------------*/
/* field_animate_weapon                                                     */
/*--------------------------------------------------------------------------*/

void field_animate_weapon(FIELD_ACTOR *fa)
{
   FIELD_ACTOR *fw;

   fw = fa->weapon;
   if ((fw->actor->type & ACTOR_WEAPON_SHOOT) == ACTOR_WEAPON_SHOOT) {
      field_animate(fw);
      return;
   }

   if ((fw->actor->type & ACTOR_WEAPON_HAND) == ACTOR_WEAPON_HAND) {
      fw->x = fa->x + CELL_X(state_move_x_step[fw->state]);
      fw->y = fa->y + CELL_Y(state_move_y_step[fw->state]);
   } else {
      fw->x = fa->x;
      fw->y = fa->y;
   }
   if (fw->x < 0 || fw->x > CELL_X(FIELD_XCELLS - 1) ||
         fw->y < 0 || fw->y > CELL_Y(FIELD_YCELLS - 1))
      /* make the attack expire immediately */
      fw->fire_time = field_frame_time - fw->actor->duration;
   if (WAS_TIME(fw, fw->actor->duration))
      fw->state = 0;
   else if ((fw->actor->type & ACTOR_WEAPON_CLOUD) == ACTOR_WEAPON_CLOUD)
      fw->redraw = 1;
}

/*--------------------------------------------------------------------------*/
/* field_weapon_collision                                                   */
/*--------------------------------------------------------------------------*/

int field_weapon_collision(FIELD_ACTOR *fw)
{
   FIELD_ACTOR *ofa, *me_fa;
   int cloud_w, cloud_h;
   int stripe, diag;
   int cx, cy;
   int hit = 0;
   int blink = 1;

   if (fw->state == 0)
      return 0;
   ofa = (field.light.weapon == fw ? &field.dark : &field.light);

   if ((fw->actor->type & ACTOR_WEAPON_CLOUD) == ACTOR_WEAPON_CLOUD) {
      cloud_w = CELL_X(fw->state);
      cloud_h = CELL_Y(fw->state);
      if (field_frame_time % CLOUD_FACTOR == 0 &&
            /* cloud only damages once per CLOUD_FACTOR frames */
            /* test 1, 2 or 3-wide by 1-high strip */
            ((ofa->x > fw->x - cloud_w && ofa->x < fw->x + cloud_w &&
            ofa->y > fw->y - CELL_Y(1) && ofa->y < fw->y + CELL_Y(1)) ||
            /* test 1-wide by 1, 2 or 3-high strip */
            (ofa->x > fw->x - CELL_X(1) && ofa->x < fw->x + CELL_X(1) &&
            ofa->y > fw->y - cloud_h && ofa->y < fw->y + cloud_h) ||
            /* on state == 3, test 2-wide by 2-high square */
            (fw->state == 3 &&
            ofa->x > fw->x - CELL_X(2) && ofa->x < fw->x + CELL_X(2) &&
            ofa->y > fw->y - CELL_Y(2) && ofa->y < fw->y + CELL_Y(2)))) {
         /* ok - they're in the bounding shape of the cloud.  Iterate over */
         /* it, checking collision with actual sprites */
         for (stripe = 0; !hit && stripe < STRIPE_MAX; stripe++) {
            cx = fw->x + CELL_X((stripe - 4)/2);
            cy = fw->y + CELL_Y(stripe/2);
            for (diag = 0; !hit && diag < DIAGONAL_MAX; diag++, cx += CELL_X(1),
                  cy -= CELL_Y(1))
               if (cloud_shape[fw->state - 1][stripe][diag] &&
                     field_collision(cx, cy, ofa->x, ofa->y)) {
                  sprite_set_state(fw->sprite, 
                        cloud_shape[fw->state - 1][stripe][diag], 0);
                  if (cx < ofa->x)
                     hit = sprite_intersect(fw->sprite, ofa->sprite,
                           ofa->x - cx, ofa->y - cy);
                  else
                     hit = sprite_intersect(ofa->sprite, fw->sprite,
                           cx - ofa->x, cy - ofa->y);
               }
         }
         blink = 0;
      }
   } else if (!ofa->blink && field_collision(fw->x, fw->y, ofa->x, ofa->y)) {
      if (sprite_get_state(fw->sprite) == -1)
         sprite_set_state(fw->sprite,
               fw->state - STATE_MOVE_FIRST + STATE_FIRE_FIRST, 0);
      if (fw->x < ofa->x)
         hit = sprite_intersect(fw->sprite, ofa->sprite,
               ofa->x - fw->x, ofa->y - fw->y);
      else
         hit = sprite_intersect(ofa->sprite, fw->sprite,
               fw->x - ofa->x, fw->y - ofa->y);
   }
   if (hit) {
      /* test that the opponant isn't using an attack that renders them */
      /* immune to damage */
      if (!(ofa->weapon->actor->type & ACTOR_IMMUNE_USER) ||
            ofa->weapon->state == 0) {
         me_fa = (ofa == &field.dark ? &field.light : &field.dark);
         ofa->life = vmax(ofa->life - fw->actor->strength, 0);
         me_fa->num_hits++;
         if (ofa->life <= 0) {
            ofa->blink = 1;
            if ((ofa->weapon->actor->type & ACTOR_WEAPON_CLOUD) ==
                  ACTOR_WEAPON_CLOUD && ofa->weapon->state != 0)
               /* immediately kill other's cloud weapon if they die */
               ofa->weapon->state = 0;
         }
         else
            ofa->blink = blink;
      }
      /* lose the bullet whether we damaged them or not */
      if ((fw->actor->type & ACTOR_WEAPON_SHOOT) == ACTOR_WEAPON_SHOOT)
         fw->state = 0;
      return 1;
   }
   return 0;
}

/*--------------------------------------------------------------------------*/
/* field_weapon                                                             */
/*--------------------------------------------------------------------------*/

void field_weapon(FIELD_ACTOR *fa)
{
   FIELD_ACTOR *fw;

   fw = fa->weapon;
   if (IS_TIME(fw, fw->actor->recharge))
      audio_player_reload(fa == &field.dark);
   if (fw->state == 0)
      return;
   field_animate_weapon(fa);
   if (fw->state != 0 && field_weapon_collision(fw))
      audio_damage(fa == &field.light);
}

/*--------------------------------------------------------------------------*/
/* field_lumi_cycle                                                         */
/*--------------------------------------------------------------------------*/

void field_lumi_cycle(void)
{
   int i;
   ROCK *rock;
   int rock_x, rock_y;
   FIELD_ACTOR *fa, *ofa;

   if (field_frame_time % ROCK_FRAMES != 0)
      return;
   for (i = 0; i < NUM_ROCKS; i++) {
      rock = &field.rocks[i];
      rock->lumi += rock->lumi_d;
      if (rock->lumi == ROCK_DARKEST || rock->lumi == ROCK_LIGHTEST)
         rock->lumi_d = -rock->lumi_d;
      field_cells[rock->y][rock->x] = (i << ROCK_IX_SHIFT) + rock->lumi;
      field_paint_cell(rock->x, rock->y);
      rock_x = CELL_X(rock->x);
      rock_y = CELL_Y(rock->y);
      for (fa = &field.light, ofa = &field.dark; fa;
            fa = (fa == &field.light) ? &field.dark : NULL, ofa = &field.light) {
         if (field_collision(rock_x, rock_y, fa->x, fa->y)) {
	   field_paint_actor(fa);
         }
         if (field_collision(rock_x, rock_y, fa->weapon->x, fa->weapon->y) ||
               ((fa->weapon->actor->type & ACTOR_WEAPON_CLOUD) ==
               ACTOR_WEAPON_CLOUD &&
               rock_x > fa->weapon->x - fa->weapon->state*CELL_XSIZE &&
               rock_x < fa->weapon->x + fa->weapon->state*CELL_XSIZE &&
               rock_y > fa->weapon->y - fa->weapon->state*CELL_YSIZE &&
               rock_y < fa->weapon->y + fa->weapon->state*CELL_YSIZE))
         field_paint_weapon(fa->weapon);
      }
   }
}

/*--------------------------------------------------------------------------*/
/* field_print                                                              */
/*--------------------------------------------------------------------------*/

void field_print(char *msg, int row, int color)
{
   int w, h;

   canvas_font_size(msg, font, &w, &h);
   canvas_font_print(msg, (CANVAS_WIDTH - w) / 2, row * h, font, color);
   field.any_output = 1;

#ifdef AUTOPILOT
   printf("field:  %s\n", msg);
#endif
}

/*--------------------------------------------------------------------------*/
/* field_prolog_epilog_move                                                 */
/*--------------------------------------------------------------------------*/

void field_prolog_epilog_move(FIELD_ACTOR *fa)
{
   if (fa->x < fa->x1)
      fa->x = vmin(fa->x1, fa->x + CELL_XSIZE / 4);
   else if (fa->x > fa->x1)
      fa->x = vmax(fa->x1, fa->x - CELL_XSIZE / 4);

   if (fa->y < fa->y1)
      fa->y = vmin(fa->y1, fa->y + CELL_YSIZE / 4);
   else if (fa->y > fa->y1)
      fa->y = vmax(fa->y1, fa->y - CELL_YSIZE / 4);
}

/*--------------------------------------------------------------------------*/
/* field_prolog                                                             */
/*--------------------------------------------------------------------------*/

void field_prolog(void)
{
   char msg[64];

   field_clear_actor(&field.light, &field.dark);
   field_clear_actor(&field.dark, &field.light);
   field_prolog_epilog_move(&field.light);
   field_prolog_epilog_move(&field.dark);
   field_paint_actor(&field.light);
   field_paint_actor(&field.dark);

#ifndef AUTOPILOT
   if (field.prolog_countdown <= PROLOG_FRAMES * 5 / 6) {
#endif
      field_print("fighting for the light side", 1, light_color);
      sprintf(msg, "the light %s", field.light.actor->name);
      field_print(msg, 2, light_color);
#ifndef AUTOPILOT
   }

   if (field.prolog_countdown <= PROLOG_FRAMES * 4 / 6) {
#endif
      field_print("fighting for the dark side", 5, dark_color);
      sprintf(msg, "the dark %s", field.dark.orig_actor->name);
      field_print(msg, 6, dark_color);
#ifndef AUTOPILOT
   }

   if (field.prolog_countdown <= PROLOG_FRAMES * 2 / 6)
#endif
      field_print("fight!", 10, back_color);

#ifdef AUTOPILOT
   field.prolog_countdown = 0;
#else
   field.prolog_countdown--;
#endif
   if (field.prolog_countdown == 0) {
      field_refresh();
      field.light.inanimate = 0;
      field.dark.inanimate = 0;
      field_me = &field.light;
      field_he = &field.dark;
      /* allow the computer player(s) (if any) to compute initial stats */
      iface_notify_computer(IFACE_FIELD_START);
   }
}

/*--------------------------------------------------------------------------*/
/* field_epilog                                                             */
/*--------------------------------------------------------------------------*/

ACTOR *field_epilog(void)
{
   ACTOR *winner;
   FIELD_ACTOR *fa_winner, *fa_loser;
   int life;
   char msg[64];
   int color;

   if (field.light.life <= 0) {
      if (field.dark.life <= 0) {
         winner = &mutual_death;
         fa_winner = fa_loser = NULL;
         life = color = 0;
      } else {
         winner = field.dark.orig_actor;
         color = dark_color;
         /* the vmax(1,...) is there to keep the winning actor at health >= 1 */
         /* even after factoring *out* the lumi-bonus */
         life = vmax(1, winner->strength - (field.dark.orig_life - field.dark.life));
         fa_winner = &field.dark;
         fa_loser = &field.light;
      }
   } else {
      winner = field.light.actor;
      color = light_color;
      life = vmax(1, winner->strength - (field.light.orig_life - field.light.life));
      fa_winner = &field.light;
      fa_loser = &field.dark;
   }

   if (field.epilog_countdown == EPILOG_FRAMES * 6 / 6) {
      if (fa_winner) {
         field_clear_actor(fa_loser, fa_winner);
         audio_end_battle(color == dark_color);
         fa_winner->x1 = field.cell_x;
         fa_winner->y1 = field.cell_y;
         fa_winner->state = (fa_winner->actor->type & ACTOR_RIGHT) ?
               STATE_MOVE_RIGHT : STATE_MOVE_LEFT;
         fa_winner->inanimate = 1;
      } else {
         field_clear_actor(&field.light, NULL);
         field_clear_actor(&field.dark, NULL);
         audio_end_battle(-1);
      }
   }

   if (fa_winner) {
      field_clear_actor(fa_winner, fa_loser);
      field_prolog_epilog_move(fa_winner);
      field_paint_actor(fa_winner);

      sprintf(msg, "the %s %s wins", color == light_color ? "light" : "dark",
            winner->name);
   } else
      sprintf(msg, "Neither side wins");

   field_print(msg, 5, color);

#ifdef AUTOPILOT
   field.epilog_countdown = 0;
#else
   field.epilog_countdown--;
#endif
   if (field.epilog_countdown > 0)
      winner = NULL;
   else {
      winner->strength = life;
      field_me = &field.light;          /* notify the computer player */
      field_he = &field.dark;           /*    (if any) of a victory */
      iface_notify_computer(IFACE_FIELD_STOP);
      field_end_game();
   }
   return winner;
}

/*--------------------------------------------------------------------------*/
/* field_refresh                                                            */
/*--------------------------------------------------------------------------*/

void field_refresh(void)
{
   int x, y;

   for (y = 0; y < FIELD_YCELLS; y++)
      for (x = 0; x < FIELD_XCELLS; x++)
         field_paint_cell(x, y);
   field_paint_bar(LIFE_BAR, &field.light);
   field_paint_bar(LIFE_BAR, &field.dark);
   field_paint_actor(&field.light);
   field_paint_actor(&field.dark);
   field_paint_weapon(field.light.weapon);
   field_paint_weapon(field.light.weapon);
   canvas_refresh();
   field.any_output = 0;
}

/*--------------------------------------------------------------------------*/
/* field_frame_paint                                                        */
/*--------------------------------------------------------------------------*/

void field_frame_paint(FIELD_DATA *old_field)
{
   int light_cleared = 0, dark_cleared = 0;

   if (old_field->light.life != field.light.life ||
       old_field->dark.life != field.dark.life)
      field.repaint_life = 1;

   if (old_field->light.x != field.light.x ||
       old_field->light.y != field.light.y ||
       old_field->light.state != field.light.state ||
       old_field->light.blink != field.light.blink ||
       old_field->light.redraw != field.light.redraw ||
       old_field->light.weapon->state != field.light.weapon->state ||
       old_field->light.weapon->x != field.light.weapon->x ||
       old_field->light.weapon->y != field.light.weapon->y ||
       old_field->light.weapon->redraw != field.light.weapon->redraw) {
      if (field_clear_actor(&old_field->light, &old_field->dark))
         dark_cleared = 1;
      light_cleared = 1;
   }
   if (old_field->dark.x != field.dark.x ||
       old_field->dark.y != field.dark.y ||
       old_field->dark.state != field.dark.state ||
       old_field->dark.blink != field.dark.blink ||
       old_field->dark.redraw != field.dark.redraw ||
       old_field->dark.weapon->state != field.dark.weapon->state ||
       old_field->dark.weapon->x != field.dark.weapon->x ||
       old_field->dark.weapon->y != field.dark.weapon->y ||
       old_field->dark.weapon->redraw != field.dark.weapon->redraw) {
      if (field_clear_actor(&old_field->dark, &old_field->light))
         light_cleared = 1;
      dark_cleared = 1;
   }

   if (field.repaint_life) {
       field_paint_bar(LIFE_BAR, &field.light);
       field_paint_bar(LIFE_BAR, &field.dark);
       field.repaint_life = 0;
   }
   if (field.repaint_ammo ||
       field_frame_time - field.light.weapon->fire_time <= field.light.weapon->actor->recharge)
      field_paint_bar(AMMO_BAR, &field.light);
   if (field.repaint_ammo ||
       field_frame_time - field.dark.weapon->fire_time <= field.dark.weapon->actor->recharge)
      field_paint_bar(AMMO_BAR, &field.dark);
   field.repaint_ammo = 0;

   if (light_cleared)
      field_paint_actor(&field.light);
   if (dark_cleared)
      field_paint_actor(&field.dark);
   if (light_cleared && field.light.weapon->state != 0)
      field_paint_weapon(field.light.weapon);
   if (dark_cleared && field.dark.weapon->state != 0)
      field_paint_weapon(field.dark.weapon);
}

/*--------------------------------------------------------------------------*/
/* field_frame                                                              */
/*--------------------------------------------------------------------------*/

ACTOR *field_frame(void)
{
   FIELD_DATA old_field;
   ACTOR *winner = NULL;

   field_frame_time++;

   if (field.prolog_countdown > 0)
      field_prolog();

   if (field.prolog_countdown + field.epilog_countdown == 0) {
      memcpy(&old_field, &field, sizeof(FIELD_DATA));
      old_field.light.weapon = &old_field.light_weapon;
      old_field.dark.weapon = &old_field.dark_weapon;
      field_player(&field.light);
      if (!board_pause_game(-1))
         return NULL;
      field_player(&field.dark);
      if (!board_pause_game(-1))
         return NULL;
      field_weapon(&field.light);
      field_weapon(&field.dark);
      field_frame_paint(&old_field);
      field_lumi_cycle();
   }

   if (field.epilog_countdown > 0)
      winner = field_epilog();

   if (field.any_output) {
      canvas_refresh();
      field.any_output = 0;
   }

   return winner;
}

/*--------------------------------------------------------------------------*/
/* field_initial_life                                                       */
/*--------------------------------------------------------------------------*/

int field_initial_life(ACTOR *actor, CELL *cell, ACTOR *enemy)
{
   int light_side;
   int lumi;

   light_side = ((actor->type & ACTOR_LIGHT) == ACTOR_LIGHT);
   if (enemy && (actors_list[actor->weapon].type & ACTOR_WEAPON_CLONE) ==
         ACTOR_WEAPON_CLONE)
      /* act as an uninjured copy of our enemy (but with our own colour) */
      actor = &actors_list[enemy->type & ACTOR_MASK];
#ifndef AUTOPILOT
   lumi = board_cell_lumi(cell);
   if (actor->type & ACTOR_ELEMENTAL)
      lumi = 0;         /* elementals are unaffected by the luminance cycle */
   else if (light_side)
      /* LUMI_DARKEST is least helpful, LUMI_LIGHTEST is most */
      lumi = (lumi & CELL_LUMI_MASK) - LUMI_DARKEST;
   else
      /* LUMI_LIGHTEST is least helpful, LUMI_DARKEST is most */
      lumi = LUMI_LIGHTEST - (lumi & CELL_LUMI_MASK);
   /* luminance bonus goes +0, +1, +3, +4, +6, +7 as the cell goes from the */
   /* opposite colour to our own. */
   return actor->strength + HEALTH_SCALE*(lumi + lumi/2);
#else
   return actor->strength;
#endif
}

/*--------------------------------------------------------------------------*/
/* field_absolute_control_delta                                             */
/*--------------------------------------------------------------------------*/

void field_absolute_control_delta(int *dx, int *dy)
{
   *dx = (*dx - FIELD_X(0) - field_me->x - CELL_XSIZE/2)/field_me->actor->speed;
   *dy = (*dy - FIELD_Y(0) - field_me->y - CELL_YSIZE/2)/field_me->actor->speed;
}
