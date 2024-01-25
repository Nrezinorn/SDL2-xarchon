/*--------------------------------------------------------------------------*/
/* battle field                                                             */
/*--------------------------------------------------------------------------*/

#ifndef __MY_FIELD_H
#define __MY_FIELD_H

#include "actors.h"
#include "board.h"

/*--------------------------------------------------------------------------*/
/* defines                                                                  */
/*--------------------------------------------------------------------------*/

#define FIELD_XCELLS    17
#define FIELD_YCELLS    13
#define FIELD_X(x) CELL_X((x) + (CANVAS_XCELLS - FIELD_XCELLS) / 2)
#define FIELD_Y(y) CELL_Y((y) + (CANVAS_YCELLS - FIELD_YCELLS) / 2)

#define ROCK_DARKEST    1               /* luminance steppings in sprite */
#define ROCK_LIGHTEST   6               /*   1=darkest  6=lightest */
#define ROCK_WALKABLE   4               /* upper 3 lumi's are walkable */
#define ROCK_LUMI_MASK  0xF             /* luminance mask in rock */
#define ROCK_IX_SHIFT   4               /* shift right to get rock index */
#define ROCK_NONE       0xF             /* indicates no rock in cell */

#define CHECK_TIME(fa,op,x) (field_frame_time - fa->fire_time op (x))
#define IS_TIME(fa,x)  CHECK_TIME(fa,==,x)
#define WAS_TIME(fa,x) CHECK_TIME(fa,>=,x)

/*--------------------------------------------------------------------------*/
/* structures                                                               */
/*--------------------------------------------------------------------------*/

typedef struct FIELD_ACTOR {
   ACTOR *actor;                        /* the actor */
   void *sprite;                        /* the sprite */
   ACTOR *orig_actor;                   /* original actor, before cloning */
   int x, y;                            /* position */
   int x1, y1;                          /* target position */
   int state;                           /* movement state */
   int blink;                           /* if blinking */
   int life;                            /* life span */
   int orig_life;                       /* life span at start of battle */
   int num_hits;                        /* how many successful attacks */
   int fire_time;                       /* frame time of fire */
   int fire_state;                      /* direction at which was firing */
   int redraw;                          /* if actor needs redraw */
   int inanimate;                       /* if actor does not animate */
   struct FIELD_ACTOR *weapon;
} FIELD_ACTOR;

/*--------------------------------------------------------------------------*/
/* functions                                                                */
/*--------------------------------------------------------------------------*/

void field_start_game(ACTOR *_light, ACTOR *_dark, CELL *cell, int cx, int cy);
void field_end_game(void);
void field_refresh(void);
ACTOR *field_frame(void);
int field_collision(int ax, int ay, int bx, int by);
int field_direction(int to_x, int to_y, int from_x, int from_y);
int field_initial_life(ACTOR *actor, CELL *cell, ACTOR *enemy);
void field_absolute_control_delta(int *dx, int *dy);

/*--------------------------------------------------------------------------*/
/* variables                                                                */
/*--------------------------------------------------------------------------*/

extern unsigned char field_cells[FIELD_YCELLS][FIELD_XCELLS];
extern FIELD_ACTOR *field_me, *field_he;
extern int field_frame_time;

#endif /* __MY_FIELD_H */
