/*--------------------------------------------------------------------------*/
/* game board                                                               */
/*--------------------------------------------------------------------------*/

#ifndef __MY_BOARD_H
#define __MY_BOARD_H

#include "actors.h"

/*--------------------------------------------------------------------------*/
/* defines                                                                  */
/*--------------------------------------------------------------------------*/

#define BOARD_XCELLS    17
#define BOARD_YCELLS    9

#define CELL_ANIM_POWER 0x0A0000        /* power sprite has this many steps */
#define CELL_ANIM_FLOOR 0xF00000        /* floor sprite has this many steps */
#define CELL_ANIM_MASK  (CELL_ANIM_FLOOR | CELL_ANIM_POWER)
#define CELL_ANIM_SHIFT_POWER 16        /* bitshift needed to get the number */
#define CELL_ANIM_SHIFT_FLOOR 20        /* bitshift needed to get the number */

#define CELL_POWER      0x28000         /* cell is a power point */
#define CELL_IMPRISON   0x4000          /* actor is imprisoned */

/* board cells values for Archon */

#define CELL_LUMI       0x2000          /* participates in luminance cycle */
#define CELL_LIGHT      0x1000          /* cell is always light */
#define CELL_DARK       0x800           /* cell is always dark */
#define CELL_LUMI_MASK  0xF

#define LUMI_DARKEST    1               /* luminance steppings in sprite */
#define LUMI_LIGHTEST   6               /*   1=darkest  6=lightest */
#define LUMI_COUNT      8               /* number of luminance states (0..7) */

/* board cells values for Adept */

#define CELL_EARTH      0x1200000       /* cell in the realm of Earth */
#define CELL_WATER      0x2200000       /* cell in the realm of Water */
#define CELL_AIR        0x3000000       /* cell in the realm of Air */
#define CELL_FIRE       0x4200000       /* cell in the realm of Fire */
#define CELL_VOID       0x5000000       /* a void cell */

#define CELL_CHAOS_CITADEL      0x6000000 /* the Citadel of Chaos */
#define CELL_ORDER_CITADEL      0x7000000 /* the Citadel of Order */

#define CELL_ELEMENT_MASK       (0x7000000 | CELL_ANIM_FLOOR)

enum {
   SPELL_TELEPORT = 1,
   SPELL_HEAL,
   SPELL_SHIFT_TIME,
   SPELL_EXCHANGE,
   SPELL_SUMMON_ELEMENTAL,
   SPELL_REVIVE,
   SPELL_IMPRISON,
   SPELL_CEASE_CONJURING,

   SPELL_FIRST = SPELL_TELEPORT,
   SPELL_LAST  = SPELL_CEASE_CONJURING,
   SPELL_COUNT = (SPELL_LAST - SPELL_FIRST + 1),
   SPELL_COUNT_2 = SPELL_COUNT + 2
};

/*--------------------------------------------------------------------------*/
/* structures                                                               */
/*--------------------------------------------------------------------------*/

typedef struct {
   int flags;                           /* see CELL_* #define's above */
   ACTOR *actor;                        /* actor occupying cell */
   void *rocks;                         /* rocks placement for field mode */
} CELL;

/*--------------------------------------------------------------------------*/
/* function                                                                 */
/*--------------------------------------------------------------------------*/

void board_start_game(int game, int light_first);
int board_end_game(void);
int board_pause_game(int pause);
void board_refresh(void);
int board_frame(void);
int board_revive_check(int *actors, int *cell_x, int *cell_y);
int board_cell_lumi(CELL *cell);
int board_is_pickable(int cx, int cy, int msg);
int *board_get_route(int x1, int y1, int x2, int y2);
int board_find_actor(int type, int *ax, int *ay);
int board_is_imprison_ok(void);
void board_get_data(int *_side, int *_turn, int *_lumi, int *_lumi_d);
void board_absolute_control_delta(int *dx, int *dy);
int board_get_cell_width();
int board_get_cell_height();

/*--------------------------------------------------------------------------*/
/* variables                                                                */
/*--------------------------------------------------------------------------*/

extern int board_turn;
extern int board_frame_time;
extern CELL board_cells[BOARD_YCELLS][BOARD_XCELLS];
extern int spell_avails[3][SPELL_COUNT_2]; /* row 0 is light, row 1 is dark */
extern int init_board_cells[NUM_GAMES][BOARD_YCELLS][BOARD_XCELLS];

#endif /* __MY_BOARD_H */
