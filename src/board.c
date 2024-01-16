/*--------------------------------------------------------------------------*/
/* game board                                                               */
/*--------------------------------------------------------------------------*/

#include <config.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "board.h"
#include "field.h"
#include "sprite.h"
#include "canvas.h"
#include "iface.h"
#include "audio.h"

/*--------------------------------------------------------------------------*/
/* defines                                                                  */
/*--------------------------------------------------------------------------*/

#define BOARD_X(x)     (board.corner_x + CELL_X(x))
#define BOARD_Y(y)     (board.corner_y + CELL_Y(y))

#define FLOOR_DARK     0                /* steppings for light and dark */
#define FLOOR_LIGHT    7                /*   outside luminance cycle */

#define FONT_NAME      "-misc-fixed-medium-*-normal-*-15-0-*-*-*-*-iso8859-1"

#define V_CELL_YSIZE   40               /* revive cell height */

#define ANIMATE_FRAMES  4               /* animate the board once in this */
                                        /* many frames (a power of 2) */

/*--------------------------------------------------------------------------*/
/* selection states                                                         */
/*--------------------------------------------------------------------------*/

enum {
   SELECT_SOURCE = 1,                   /* non-spell */
   SELECT_TARGET,
   SELECT_FIELD,                        /* field mode */
   SELECT_GAME_OVER,                    /* the state to end all states! */
   SELECT_SPELL,
   SELECT_SPELL_DONE,                   /* spell completed or failed */
   T_SELECT_SOURCE,                     /* teleport */
   T_SELECT_TARGET,
   H_SELECT_TARGET,                     /* heal */
   X_SELECT_SOURCE,                     /* exchange */
   X_SELECT_TARGET,
   L_SELECT_TARGET,                     /* elemental */
   V_SELECT_SOURCE,                     /* revive */
   M_SELECT_TARGET,                     /* imprison */
};

/*--------------------------------------------------------------------------*/
/* structures                                                               */
/*--------------------------------------------------------------------------*/

typedef struct {                        /* Slow Sprite Paint */
   int count;
   int x1, y1;
   int both;                            /* double ssp -- for exchange spell */
   int x2, y2;
   int complete;                        /* ssp completed */
} SSP;

typedef struct {
   int game_status;                     /* 0=none,  1=active,  -1=paused */
   int turn;                            /* count of turns since game start */
   int side;                            /* 0=light,  1=dark */
   int state;                           /* state of the board */
   int lumi, lumi_d;                    /* luminance value and direction */
   int cx, cy;                          /* current cursor position */
   int cstate;                          /* state of cursor movement */
   int ax0, ay0;                        /* initial position of picked actor */
   int fire_down;                       /* fire key is held */
   int spell;                           /* spell selected */
   int prev_down, next_down;            /* up/down key is held */
   int teleport, exchange;              /* teleport/exchange taking place */
   int tx0, ty0;                        /* teleport source / exchange 1st */
   int tx1, ty1;                        /* teleport target / exchange 2nd */
   SSP ssp;                             /* Slow Sprite Paint data */
   CELL elem;                           /* cell for active elemental */
   char message[64];                    /* last board message displayed */
   int any_output;                      /* if any output emitted */
   int cell_w, cell_h;                  /* # of cells wide/high board is */
   int width, height;                   /* # of pixels wide/high board is */
   int corner_x, corner_y;              /* coordinates of top left corner */
   int game;                            /* 0 for Archon, 1 for Adept... */
   char *side_name[2];                  /* light/order and dark/chaos */
} BOARD_DATA;

typedef struct {
   void *ptr;
   long fg, bg;
} FONT;

/*--------------------------------------------------------------------------*/
/* functions                                                                */
/*--------------------------------------------------------------------------*/

static void board_paint_cell(int x, int y);
static void board_paint_cursor(void);
static void board_animate_board(void);
static void board_message(char *format, ...);
static void board_ssp_init(int x1, int y1, int x2, int y2, int both);
static void board_ssp_frame(void);
static void board_init_cell(CELL *cell, int actor_num);
static void board_clear_cell(CELL *cell);
static void board_reset_actor(ACTOR *actor);
static void board_iface_turn(void);
static int  board_verify_move(int state, int with_actor);
static int  board_check_win(void);
static void board_end_turn(int swap_side);
static void board_field(int xd, int yd, int xa, int ya);
static void board_move_done(void);
static void board_spell_done(void);
static void board_input(void);
static void board_cursor(void);
static void board_spell(void);
static void board_teleport(int x, int y, int side);
static void board_heal(int x, int y, int side);
static void board_shift_time(int x, int y, int side);
static void board_exchange(int x, int y, int side);
static void board_summon_elemental(int x, int y, int side);
static int board_revive_frame(int *i, int *actors);
static void board_revive(int x, int y, int side);
static void board_imprison(int x, int y, int side);
static void board_cease_conjuring(int x, int y, int side);
static int board_get_route_2(int x1, int y1, int x2, int y2, int *route,
                             int prev_state, int light, int ground, int distance);

/*--------------------------------------------------------------------------*/
/* variables                                                                */
/*--------------------------------------------------------------------------*/

static FONT font = { NULL };

int init_board_cells[NUM_GAMES][BOARD_YCELLS][BOARD_XCELLS] = {
   /* the board for Archon */
   {
      { ( CELL_DARK  | ACTOR_VALKYRIE            ), ( CELL_LIGHT | ACTOR_ARCHER ), ( CELL_DARK  ), ( CELL_LUMI  ), ( CELL_LUMI | CELL_POWER ), ( CELL_LUMI  ), ( CELL_LIGHT ), ( CELL_DARK  | ACTOR_MANTICORE ), ( CELL_LIGHT | ACTOR_BANSHEE                ) },
      { ( CELL_LIGHT | ACTOR_GOLEM               ), ( CELL_DARK  | ACTOR_KNIGHT ), ( CELL_LUMI  ), ( CELL_LIGHT ), ( CELL_LUMI              ), ( CELL_DARK  ), ( CELL_LUMI  ), ( CELL_LIGHT | ACTOR_GOBLIN    ), ( CELL_DARK  | ACTOR_TROLL                  ) },
      { ( CELL_DARK  | ACTOR_UNICORN             ), ( CELL_LUMI  | ACTOR_KNIGHT ), ( CELL_LIGHT ), ( CELL_DARK  ), ( CELL_LUMI              ), ( CELL_LIGHT ), ( CELL_DARK  ), ( CELL_LUMI  | ACTOR_GOBLIN    ), ( CELL_LIGHT | ACTOR_BASILISK               ) },
      { ( CELL_LUMI  | ACTOR_DJINNI              ), ( CELL_LIGHT | ACTOR_KNIGHT ), ( CELL_DARK  ), ( CELL_LIGHT ), ( CELL_LUMI              ), ( CELL_DARK  ), ( CELL_LIGHT ), ( CELL_DARK  | ACTOR_GOBLIN    ), ( CELL_LUMI  | ACTOR_SHAPESHIFTER           ) },
      { ( CELL_LIGHT | ACTOR_WIZARD | CELL_POWER ), ( CELL_LUMI  | ACTOR_KNIGHT ), ( CELL_LUMI  ), ( CELL_LUMI  ), ( CELL_LUMI | CELL_POWER ), ( CELL_LUMI  ), ( CELL_LUMI  ), ( CELL_LUMI  | ACTOR_GOBLIN    ), ( CELL_DARK  | ACTOR_SORCERESS | CELL_POWER ) },
      { ( CELL_LUMI  | ACTOR_PHOENIX             ), ( CELL_LIGHT | ACTOR_KNIGHT ), ( CELL_DARK  ), ( CELL_LIGHT ), ( CELL_LUMI              ), ( CELL_DARK  ), ( CELL_LIGHT ), ( CELL_DARK  | ACTOR_GOBLIN    ), ( CELL_LUMI  | ACTOR_DRAGON                 ) },
      { ( CELL_DARK  | ACTOR_UNICORN             ), ( CELL_LUMI  | ACTOR_KNIGHT ), ( CELL_LIGHT ), ( CELL_DARK  ), ( CELL_LUMI              ), ( CELL_LIGHT ), ( CELL_DARK  ), ( CELL_LUMI  | ACTOR_GOBLIN    ), ( CELL_LIGHT | ACTOR_BASILISK               ) },
      { ( CELL_LIGHT | ACTOR_GOLEM               ), ( CELL_DARK  | ACTOR_KNIGHT ), ( CELL_LUMI  ), ( CELL_LIGHT ), ( CELL_LUMI              ), ( CELL_DARK  ), ( CELL_LUMI  ), ( CELL_LIGHT | ACTOR_GOBLIN    ), ( CELL_DARK  | ACTOR_TROLL                  ) },
      { ( CELL_DARK  | ACTOR_VALKYRIE            ), ( CELL_LIGHT | ACTOR_ARCHER ), ( CELL_DARK  ), ( CELL_LUMI  ), ( CELL_LUMI | CELL_POWER ), ( CELL_LUMI  ), ( CELL_LIGHT ), ( CELL_DARK  | ACTOR_MANTICORE ), ( CELL_LIGHT | ACTOR_BANSHEE                ) }
   },
   /* the board for Adept */
   {
      { (CELL_FIRE | CELL_POWER), (CELL_FIRE), (CELL_FIRE), (CELL_FIRE), (CELL_FIRE), (CELL_FIRE), (CELL_FIRE), (CELL_FIRE), (CELL_FIRE), (CELL_FIRE), (CELL_FIRE), (CELL_FIRE), (CELL_FIRE), (CELL_FIRE), (CELL_FIRE), (CELL_FIRE), (CELL_FIRE | CELL_POWER) },
      { (CELL_FIRE), (CELL_AIR), (CELL_AIR), (CELL_AIR), (CELL_AIR), (CELL_AIR), (CELL_AIR), (CELL_AIR), (CELL_VOID | CELL_POWER), (CELL_AIR), (CELL_AIR), (CELL_AIR), (CELL_AIR), (CELL_AIR), (CELL_AIR), (CELL_AIR), (CELL_FIRE) },
      { (CELL_FIRE), (CELL_AIR), (CELL_WATER), (CELL_WATER), (CELL_WATER), (CELL_WATER), (CELL_WATER), (CELL_AIR), (CELL_AIR), (CELL_AIR), (CELL_WATER), (CELL_WATER), (CELL_WATER), (CELL_WATER), (CELL_WATER), (CELL_AIR), (CELL_FIRE) },
      { (CELL_FIRE), (CELL_AIR), (CELL_WATER), (CELL_EARTH), (CELL_EARTH), (CELL_EARTH), (CELL_WATER), (CELL_WATER), (CELL_WATER), (CELL_WATER), (CELL_WATER), (CELL_EARTH), (CELL_EARTH), (CELL_EARTH), (CELL_WATER), (CELL_AIR), (CELL_FIRE) },
      { (CELL_FIRE /*| ACTOR_CHAOS_ADEPT*/), (CELL_AIR /*| ACTOR_CHAOS_ADEPT */), (CELL_WATER /*| ACTOR_CHAOS_ADEPT */), (CELL_EARTH /*| ACTOR_CHAOS_ADEPT */), (CELL_CHAOS_CITADEL), (CELL_EARTH), (CELL_EARTH), (CELL_EARTH), (CELL_EARTH), (CELL_EARTH), (CELL_EARTH), (CELL_EARTH), (CELL_ORDER_CITADEL), (CELL_EARTH /*| ACTOR_ORDER_ADEPT */), (CELL_WATER /*| ACTOR_ORDER_ADEPT */), (CELL_AIR /*| ACTOR_ORDER_ADEPT */), (CELL_FIRE /*| ACTOR_ORDER_ADEPT */) },
      { (CELL_FIRE), (CELL_AIR), (CELL_WATER), (CELL_EARTH), (CELL_EARTH), (CELL_EARTH), (CELL_WATER), (CELL_WATER), (CELL_WATER), (CELL_WATER), (CELL_WATER), (CELL_EARTH), (CELL_EARTH), (CELL_EARTH), (CELL_WATER), (CELL_AIR), (CELL_FIRE) },
      { (CELL_FIRE), (CELL_AIR), (CELL_WATER), (CELL_WATER), (CELL_WATER), (CELL_WATER), (CELL_WATER), (CELL_AIR), (CELL_AIR), (CELL_AIR), (CELL_WATER), (CELL_WATER), (CELL_WATER), (CELL_WATER), (CELL_WATER), (CELL_AIR), (CELL_FIRE) },
      { (CELL_FIRE), (CELL_AIR), (CELL_AIR), (CELL_AIR), (CELL_AIR), (CELL_AIR), (CELL_AIR), (CELL_AIR), (CELL_VOID | CELL_POWER), (CELL_AIR), (CELL_AIR), (CELL_AIR), (CELL_AIR), (CELL_AIR), (CELL_AIR), (CELL_AIR), (CELL_FIRE) },
      { (CELL_FIRE | CELL_POWER), (CELL_FIRE), (CELL_FIRE), (CELL_FIRE), (CELL_FIRE), (CELL_FIRE), (CELL_FIRE), (CELL_FIRE), (CELL_FIRE), (CELL_FIRE), (CELL_FIRE), (CELL_FIRE), (CELL_FIRE), (CELL_FIRE), (CELL_FIRE), (CELL_FIRE), (CELL_FIRE | CELL_POWER) }
   }
};

CELL board_cells[BOARD_YCELLS][BOARD_XCELLS];

static BOARD_DATA board = { 0 };               /* game status == none */

int board_turn;

static int board_frame_time = 0;

/*--------------------------------------------------------------------------*/
/* spells                                                                   */
/*--------------------------------------------------------------------------*/

static char *spell_names[SPELL_COUNT_2] = {
   NULL, "teleport", "heal", "shift time", "exchange", "summon elemental",
   "revive", "imprison", "cease conjuring", NULL
};
static void (*spell_funcs[SPELL_COUNT_2])(int x, int y, int side) = {
   NULL, board_teleport, board_heal, board_shift_time, board_exchange,
   board_summon_elemental, board_revive, board_imprison,
   board_cease_conjuring, NULL
};
int spell_avails[3][SPELL_COUNT_2] = {
   { },                                 /* for light */
   { },                                 /* for dark */
   { 0, 1, 1, 1, 1, 1, 1, 1, 1, 0 }     /* default--used to initialize */
};

/*--------------------------------------------------------------------------*/
/* board_paint_cell                                                         */
/*--------------------------------------------------------------------------*/

void board_paint_cell(int x, int y)
{
   int flags;
   int sx, sy;
   int anim_frame;
   int state, stepping;
   int ssp = 0;
   ACTOR *actor;
   ACTOR *other = NULL;

   sx = BOARD_X(x);
   sy = BOARD_Y(y);
   flags = board_cells[y][x].flags;
   anim_frame = board_frame_time/ANIMATE_FRAMES;
   if (flags & CELL_ANIM_FLOOR)
      stepping = anim_frame %
         ((flags & CELL_ANIM_FLOOR)>>CELL_ANIM_SHIFT_FLOOR);
   else
      stepping = 0;
   switch (flags & CELL_ELEMENT_MASK) {
      case CELL_EARTH:
         state = STATE_EARTH_ELEM;
         break;
      case CELL_WATER:
         state = STATE_WATER_ELEM;
         break;
      case CELL_AIR:
         state = STATE_AIR_ELEM;
         break;
      case CELL_FIRE:
         state = STATE_FIRE_ELEM;
         break;
      case CELL_VOID:
         state = STATE_EMPTY;
         break;
      default:
         /* Archon board */
         state = STATE_BOARD;
         stepping = (flags & CELL_LIGHT) ? FLOOR_LIGHT :
               (flags & CELL_DARK) ? FLOOR_DARK : board.lumi;
   }
   sprite_set_state(floor_sprite, state, stepping);
   sprite_paint(floor_sprite, state, sx, sy);
   if (flags & CELL_POWER) {
      sprite_set_state(floor_sprite, STATE_POWER, anim_frame %
	       ((/* flags & */ CELL_ANIM_POWER)>>CELL_ANIM_SHIFT_POWER));
      sprite_paint(floor_sprite, STATE_POWER, sx, sy);
   }
   if (board.ssp.count > 0) {
      if (x == board.ssp.x1 && y == board.ssp.y1)
         ssp = 1;       /* we're the cell being teleported from */
      else if (x == board.ssp.x2 && y == board.ssp.y2)
         ssp = 2;       /* we're the cell being teleported to */
   }
   /* draw any creature present in this cell... */
   actor = board_cells[y][x].actor;
   if (board.state == SELECT_TARGET && x == board.ax0 && y == board.ay0 &&
         (actor->type & ACTOR_MASTER) != ACTOR_MASTER)
      /* ... unless they're moving about the board */
      actor = NULL;
   if (actor != NULL) {
      if (ssp == 0 || (ssp == 2 && !board.ssp.both))
         /* if there's nothing fancy going on, or if this creature is being */
         /* teleported upon, just draw it normally */
         sprite_paint(board_cells[y][x].actor->sprite, SPRITE_STOP, sx, sy);
      else
         /* else this creature is teleporting out of the cell - draw only */
         /* the lower portion */
         sprite_paint_clipped(actor->sprite, SPRITE_STOP,
               sx, sy + board.ssp.count, 0, board.ssp.count,
               CELL_XSIZE, CELL_YSIZE - board.ssp.count);
   }
   if (ssp == 2)
      other = board_cells[board.ssp.y1][board.ssp.x1].actor;
   else if (ssp == 1 && board.ssp.both)
      other = board_cells[board.ssp.y2][board.ssp.x2].actor;
   if (other)
      /* something is teleporting in to this square - draw its upper portion */
      sprite_paint_clipped(other->sprite, SPRITE_STOP, sx, sy, 0, 0,
            CELL_XSIZE, board.ssp.count);
   board.any_output = 1;
}

/*--------------------------------------------------------------------------*/
/* board_paint_cursor                                                       */
/*--------------------------------------------------------------------------*/

void board_paint_cursor(void)
{
   int sx, sy;

   if (board.state == SELECT_GAME_OVER)
      return;

   sx = BOARD_X(0) + board.cx;
   sy = BOARD_Y(0) + board.cy;
   if (board.state == SELECT_TARGET || board.state == L_SELECT_TARGET) {
      if (board.state == L_SELECT_TARGET) {
         sprite_paint(board.elem.actor->sprite,
                      sprite_get_state(board.elem.actor->sprite), sx, sy);
         sx = BOARD_X(board.ax0);
         sy = BOARD_Y(board.ay0);
      } else if ((board_cells[board.ay0][board.ax0].actor->type &
            ACTOR_MASTER) != ACTOR_MASTER) {
         sprite_paint(board_cells[board.ay0][board.ax0].actor->sprite,
                      ( (CELL_XOFFSET(sx) == 0 && CELL_YOFFSET(sy) == 0)
                        ? SPRITE_STOP : board.cstate ), sx, sy);
         sx = BOARD_X(board.ax0);
         sy = BOARD_Y(board.ay0);
      }
   }
   sprite_set_state(cursor_sprite, STATE_BOARD, board.side);
   sprite_paint(cursor_sprite, STATE_BOARD, sx, sy);

   board.any_output = 1;
}

/*--------------------------------------------------------------------------*/
/* board_animate_board                                                      */
/*--------------------------------------------------------------------------*/

void board_animate_board(void)
{
   int x, y;

   /* only animate the board every n-th frame */
   if (board_frame_time & (ANIMATE_FRAMES - 1))
      return;
   for (y = 0; y < board.cell_h; y++)
      for (x = 0; x < board.cell_w; x++)
         if (board_cells[y][x].flags & CELL_ANIM_MASK) {
            board_paint_cell(x, y);
            if (board.cx/CELL_XSIZE == x && board.cy/CELL_YSIZE == y &&
                  board.cstate == 0)
               board_paint_cursor();
         }

}

/*--------------------------------------------------------------------------*/
/* board_message                                                            */
/*--------------------------------------------------------------------------*/

void board_message(char *format, ...)
{
   va_list ap;
   char msg[64];
   int width, height;

   if (format != NULL) {
      va_start(ap, format);
      vsprintf(msg, format, ap);
      va_end(ap);
      if (strcmp(msg, board.message) == 0)
         return;
   } else
      strcpy(msg, board.message);

   canvas_font_size(msg, font.ptr, &width, &height);
   canvas_rectangle(0, CANVAS_HEIGHT - CELL_YSIZE,
                    CANVAS_WIDTH, CELL_YSIZE, font.bg);
   canvas_font_print(msg, (CANVAS_WIDTH - width) / 2, CANVAS_HEIGHT - height,
                     font.ptr, font.fg);
   strcpy(board.message, msg);

   sprintf(msg, "(%c%c%d)", (board.lumi_d == -1 ? '-' : '+'),
                            (board.lumi <= 3 ? 'D' : 'L'),
                            board.lumi);
   canvas_font_size(msg, font.ptr, &width, &height);
   canvas_font_print(msg, CANVAS_WIDTH - width, CANVAS_HEIGHT - height,
                     font.ptr, font.fg);

   board.any_output = 1;

#ifdef AUTOPILOT
   printf("board:  %s\n", board.message);
#endif
}

/*--------------------------------------------------------------------------*/
/* board_ssp_init                                                           */
/*--------------------------------------------------------------------------*/

void board_ssp_init(int x1, int y1, int x2, int y2, int both)
{
   if (x1 != x2 || y1 != y2)
      board_paint_cell(x2, y2);         /* repaint cell to remove cursor */
   board.ssp.count = 1;
   board.ssp.x1 = x1;
   board.ssp.y1 = y1;
   board.ssp.x2 = x2;
   board.ssp.y2 = y2;
   board.ssp.both = both;
}

/*--------------------------------------------------------------------------*/
/* board_ssp_frame                                                          */
/*--------------------------------------------------------------------------*/

void board_ssp_frame(void)
{
   board_paint_cell(board.ssp.x1, board.ssp.y1);
   board_paint_cell(board.ssp.x2, board.ssp.y2);
   if (board.ssp.count == CELL_YSIZE) {
      board.ssp.count = 0;
         board.ssp.complete = 1;
         if (board.spell)
            /* restart the spell that requested ssp */
            board_spell_done();
         else
            /* return to the movement that requested ssp */
            board_move_done();
         board.ssp.complete = 0;
   } else
      board.ssp.count++;
}

/*--------------------------------------------------------------------------*/
/* board_init_cell                                                          */
/*--------------------------------------------------------------------------*/

void board_init_cell(CELL *cell, int actor_num)
{
   cell->actor = malloc(sizeof(ACTOR));
   memcpy(cell->actor, &actors_list[actor_num], sizeof(ACTOR));
   cell->actor->sprite = sprite_copy(actors_list[actor_num].sprite, 0);
   sprite_set_state(cell->actor->sprite, (cell->actor->type & ACTOR_RIGHT ? STATE_MOVE_RIGHT : STATE_MOVE_LEFT), 0);   
}

/*--------------------------------------------------------------------------*/
/* board_clear_cell                                                         */
/*--------------------------------------------------------------------------*/

void board_clear_cell(CELL *cell)
{
   sprite_free(cell->actor->sprite);
   free(cell->actor);
   cell->actor = NULL;
   cell->flags &= ~CELL_IMPRISON;
}

/*--------------------------------------------------------------------------*/
/* board_reset_actor                                                        */
/*--------------------------------------------------------------------------*/

void board_reset_actor(ACTOR *actor)
{
   if (!(actor->type & ACTOR_ELEMENTAL))
      sprite_set_state(actor->sprite,
                       actor->type & ACTOR_RIGHT ? STATE_MOVE_RIGHT : STATE_MOVE_LEFT, 0);
}

/*--------------------------------------------------------------------------*/
/* board_iface_turn                                                         */
/*--------------------------------------------------------------------------*/

void board_iface_turn(void)
{
   board_turn = board.turn;
   iface_turn(board.side, IFACE_BOARD);
}

/*--------------------------------------------------------------------------*/
/* board_verify_move                                                        */
/*--------------------------------------------------------------------------*/

int board_verify_move(int state, int with_actor)
{
   static int opposite_state[] = { 0, STATE_MOVE_DOWN, STATE_MOVE_UP, STATE_MOVE_RIGHT, STATE_MOVE_LEFT };
   ACTOR *actor;
   int x0, y0, x1, y1;
   int distance;

   x0 = board.cx / CELL_XSIZE;
   y0 = board.cy / CELL_YSIZE;
   x1 = vmax(0, vmin(board.cell_w - 1, x0 + state_move_x_step[state]));
   y1 = vmax(0, vmin(board.cell_h - 1, y0 + state_move_y_step[state]));

   if (x1 == x0 && y1 == y0)
      return 0;

   if (!with_actor)
      return 1;

   actor = board_cells[board.ay0][board.ax0].actor;
   /* fly actor checks:  only requirement is that it is within the range */

   if ((actor->type & ACTOR_FLY) ||
         (actor->type & ACTOR_MASTER) == ACTOR_MASTER) {
      distance = vmax(vabs(x1 - board.ax0), vabs(y1 - board.ay0));
      if (distance > actor->distance) {
         sprite_set_state(board_cells[board.ay0][board.ax0].actor->sprite, state, 0);
         board_message("Alas, master, you have moved your limit.");
         return 0;
      }
      return 1;
   }

   /* ground checks:  first, cannot pass over opponent actors */

   if ((x0 != board.ax0 || y0 != board.ay0) &&
       board_cells[y0][x0].actor != NULL &&
       state != opposite_state[sprite_get_state(board_cells[board.ay0][board.ax0].actor->sprite)]) {
      board_message("Do you challenge this foe?");
      return 0;
   }

   /* check if it is possible to find a route on the board between */
   /* (x0,y0) and (x1,y1) for the creature standing at (x0,y0)     */

   if ((x1 != board.ax0 || y1 != board.ay0) &&
       board_get_route(board.ax0, board.ay0, x1, y1) == NULL) {
      sprite_set_state(board_cells[board.ay0][board.ax0].actor->sprite, state, 0);

      /* if it isn't possible, it might be because the creature is */
      /* blocked by an opponent, or because it's just too far      */
       
      if (actor->type & ACTOR_GROUND &&
          board_cells[y1][x1].actor != NULL &&
          (board_cells[y1][x1].actor->type & ACTOR_LIGHT) ==
             (board_cells[board.ay0][board.ax0].actor->type & ACTOR_LIGHT))
         board_message("The square ahead is occupied.");
      else
         board_message("Alas, master, you have moved your limit.");
      return 0;
   }

   return 1;
}

/*--------------------------------------------------------------------------*/
/* board_check_win                                                          */
/*--------------------------------------------------------------------------*/

int board_check_win(void)
{
   static char *msgs[] = {  NULL,  "The dark side wins!",
      "The light side wins!",  "The result is a tie!"  };
   int x, y;
   CELL *cell;
   int is_dark;
   int num_light = 0, num_dark = 0;
   int num_pp = 0, num_light_pp = 0, num_dark_pp = 0;
   int winner = 0;
   int old_side;

   for (y = 0; y < board.cell_h; y++)
      for (x = 0; x < board.cell_w; x++) {
         cell = &board_cells[y][x];
         if (cell->flags & CELL_POWER)
            num_pp++;
         if (cell->actor != NULL) {
            is_dark = ((cell->actor->type & ACTOR_LIGHT) == 0);
            /* count an actor for its side only if it isn't imprisoned; */
            /* or if it is imprisoned, but its side has just played.    */
            if ((cell->flags & CELL_IMPRISON) == 0 || board.side == is_dark) {
               /* count it only if it can be picked. */
               old_side = board.side;
               board.side = (cell->actor->type & ACTOR_LIGHT) ? 0 : 1;
               if (board_is_pickable(x, y, 0)) {
                  num_light += is_dark == 0;
                  num_dark  += is_dark == 1;
               }
               board.side = old_side;
            }
            /* count control of power points */
            if (cell->flags & CELL_POWER) {
               num_light_pp += is_dark == 0;
               num_dark_pp  += is_dark == 1;
            }
         }
      }

   if (num_light == 0 || num_dark_pp == num_pp)
      winner = 1;                         /* light loses */
   if (num_dark == 0 || num_light_pp == num_pp)
      winner += 2;                      /* dark loses, or it's a tie */
   if (winner != 0) {
      board_message("The game is over.  %s", msgs[winner]);
      board.state = SELECT_GAME_OVER;
      audio_end_game(2 - winner);
      return 1;
   }
   return 0;
}

/*--------------------------------------------------------------------------*/
/* board_end_turn                                                           */
/*--------------------------------------------------------------------------*/

void board_end_turn(int swap_side)
{
   int x, y;
   CELL *cell;
   ACTOR *actor;

   board.state = SELECT_SOURCE;
   board.ax0 = -1;

   if (swap_side) {                     /* otherwise just reset this turn */
      board.turn++;
      if (board.turn % 2 == 0) {
         board.lumi += board.lumi_d;
         if (board.lumi == LUMI_DARKEST || board.lumi == LUMI_LIGHTEST)
            board.lumi_d = -board.lumi_d;
         for (y = 0; y < board.cell_h; y++)
            for (x = 0; x < board.cell_w; x++)
               if (board_cells[y][x].actor != NULL) {
                  cell = &board_cells[y][x];
                  actor = cell->actor;
                  /* un-imprison light actor when luminance cycle is lightest; */
                  /* un-imprison dark actor when luminance cycle is darkest.   */
                  if (cell->flags & CELL_IMPRISON &&
                      ((board.lumi == LUMI_LIGHTEST && actor->type & ACTOR_LIGHT) ||
                       (board.lumi == LUMI_DARKEST && !(actor->type & ACTOR_LIGHT))))
                     cell->flags ^= CELL_IMPRISON;
                  /* heal actors, twice as fast if on a power point */
                  actor->strength =
                        vmin(actors_list[actor->type & ACTOR_MASK].strength,
                        actor->strength + HEALTH_SCALE*2*
                        (((cell->flags & CELL_POWER) == CELL_POWER) ? 2 : 1));
               }
      }

      if (board.elem.actor != NULL)
         board_clear_cell(&board.elem);
      if (board_check_win()) {          /* important that check_win() be */
         board_refresh();               /*    called only after un-imprison */
         return;                        /*    is done */
      }
      board.side = !board.side;

      board.cx = (board.side == 0) ? CELL_X(0) : board.width - CELL_X(1);
      board.cy = CELL_Y(board.cell_h / 2);
      board.cstate = 0;
      audio_start_turn(board.side);
   }

   sprintf(board.message, "%s plays", board.side_name[board.side]);
   board_refresh();
   board_iface_turn();
}

/*--------------------------------------------------------------------------*/
/* board_field                                                              */
/*--------------------------------------------------------------------------*/

void board_field(int xd, int yd, int xa, int ya)
{
   static CELL *defender, *attacker;
   static int light_attacks;
   ACTOR *light, *dark, *winner;

   if (board.state != SELECT_FIELD) {
      defender = &board_cells[yd][xd];
      attacker = &board_cells[ya][xa];
      light = attacker->actor;
      if (light->type & ACTOR_LIGHT) {
         dark = defender->actor;
         light_attacks = 1;
      } else {
         dark = light;
         light = defender->actor;
         light_attacks = 0;
      }
      defender->flags &= ~CELL_LUMI_MASK;
      if (defender->flags & CELL_LIGHT)
         defender->flags |= FLOOR_LIGHT;
      if (defender->flags & CELL_DARK)
         defender->flags |= FLOOR_DARK;
      if (defender->flags & CELL_LUMI)
         defender->flags |= board.lumi;
      board_reset_actor(defender->actor);
      board_reset_actor(attacker->actor);
      field_start_game(light, dark, defender, BOARD_X(xd), BOARD_Y(yd));
      board.state = SELECT_FIELD;

   } else {
      winner = field_frame();
      if (winner != NULL) {
         if (winner == attacker->actor) {
            board_clear_cell(defender);
            if ((attacker->actor->type & ACTOR_ELEMENTAL) != ACTOR_ELEMENTAL) {
               board_reset_actor(attacker->actor);
               defender->actor = attacker->actor;
            }
         } else if (winner == defender->actor) {
            board_reset_actor(defender->actor);
            if ((attacker->actor->type & ACTOR_ELEMENTAL) != ACTOR_ELEMENTAL)
               board_clear_cell(attacker);
         } else {
            board_clear_cell(defender);
            if ((attacker->actor->type & ACTOR_ELEMENTAL) != ACTOR_ELEMENTAL)
               board_clear_cell(attacker);
         }
         attacker->actor = NULL;
         board_end_turn(1);
      }
   }
}

/*--------------------------------------------------------------------------*/
/* board_move_done                                                          */
/*--------------------------------------------------------------------------*/

void board_move_done(void)
{
   int x, y;
   ACTOR *source, *target;

   x = board.cx / CELL_XSIZE;
   y = board.cy / CELL_YSIZE;
   source = board_cells[board.ay0][board.ax0].actor;
   if (x == board.ax0 && y == board.ay0) {
      if ((source->type & ACTOR_MASTER) == ACTOR_MASTER) {
         board_reset_actor(source);
         board_paint_cell(board.ax0, board.ay0);
         board_paint_cursor();
         board_message("the %s conjures a spell", board_cells[board.ay0][board.ax0].actor->name);
         board.state = SELECT_SPELL;
         board.spell = 0;
      } else
         board_end_turn(0);
      return;
   }

   target = board_cells[y][x].actor;
   if (target != NULL &&
         (target->type & ACTOR_LIGHT) == (source->type & ACTOR_LIGHT)) {
      board_message("you cannot attack your own");
      return;
   }

   /* movement is going to be allowed - do teleport effect if appropriate */
   if ((source->type & ACTOR_MASTER) == ACTOR_MASTER && !board.ssp.complete) {
      board.spell = 0;
      board_ssp_init(board.ax0, board.ay0, x, y, 0);
      return;
   }

   if (target) {
      board_field(x, y, board.ax0, board.ay0);
      return;
   }

   board_cells[y][x].actor = source;
   board_cells[board.ay0][board.ax0].actor = NULL;
   board_reset_actor(source);
   board_end_turn(1);
}

/*--------------------------------------------------------------------------*/
/* board_spell_done                                                         */
/*--------------------------------------------------------------------------*/

void board_spell_done(void)
{
   int x, y;
   int side;

   x = board.cx / CELL_XSIZE;
   y = board.cy / CELL_YSIZE;
   if (board_cells[y][x].flags & CELL_POWER) {
      board_message("power points block all spells");
      board.state = SELECT_SPELL_DONE;
      board.spell = 0;
      return;
   }

   if (board_cells[y][x].flags & CELL_IMPRISON &&
       (board.state == T_SELECT_SOURCE ||
        board.state == X_SELECT_SOURCE ||
        board.state == X_SELECT_TARGET)) {
      board_message("alas, master, this %s is imprisoned",
                    board_cells[y][x].actor->name);
      board.state = SELECT_SPELL_DONE;
      board.spell = 0;
      return;
   }

   /* set side: -1 if empty cell, 0 if light, 1 if dark */
   if (board_cells[y][x].actor != NULL)
      side = (board_cells[y][x].actor->type & ACTOR_LIGHT) != ACTOR_LIGHT;
   else
      side = -1;

   spell_funcs[board.spell](x, y, side);
}

/*--------------------------------------------------------------------------*/
/* board_input                                                              */
/*--------------------------------------------------------------------------*/

void board_input(void)
{
   int state, use_state;
   int x0, y0;
   int x1, y1;

   iface_frame();
   if (board.game_status == 0)
      return;

   for (state = STATE_MOVE_LAST; state >= STATE_MOVE_FIRST; state--)
      if (iface_key_down(state)) {
         use_state = state;
         if (board.state == SELECT_TARGET &&
               board_cells[board.ay0][board.ax0].actor->type & ACTOR_GROUND &&
               state > STATE_MOVE_AXIAL_LAST) {
            /* transform diagonal moves when moving ground units into */
            /* axially-aligned moves */
            x0 = board.cx/CELL_XSIZE;
            y0 = board.cy/CELL_YSIZE;
            x1 = x0 + state_move_x_step[state];
            y1 = y0 + state_move_y_step[state];
            if (x1 >= 0 && x1 < board.cell_w && y1 >= 0 && y1 < board.cell_h) {
               /* walk around occupied squares, or minimise travel distance */
               if (board_cells[y1][x0].actor ||
                     (!board_cells[y0][x1].actor &&
                     (x0 - board.ax0 > 0) !=
                     (state_move_x_step[state] > 0)))
                  /* find the horizontal move in a similar direction */
                  for (use_state = STATE_MOVE_AXIAL_LAST;
                        state_move_x_step[use_state] != 
                        state_move_x_step[state] ||
                        state_move_y_step[use_state]; use_state--)
                     ;
               else
                  /* find the vertical move in a similar direction */
                  for (use_state = STATE_MOVE_AXIAL_LAST;
                        state_move_x_step[use_state] ||
                        state_move_y_step[use_state] !=
                        state_move_y_step[state]; use_state--)
                     ;
            }
         }
         if (board_verify_move(use_state, board.state == SELECT_TARGET)) {
            board.cstate = use_state;
            return;
         }
      }

   if (iface_key_down(STATE_FIRE))
      board.fire_down = 1;
   if (board.fire_down && !iface_key_down(STATE_FIRE)) {
      board.fire_down = 0;
      if (board.state == SELECT_SOURCE) {
         if (board_is_pickable(board.cx / CELL_XSIZE, board.cy / CELL_YSIZE, 1)) {
            board.ax0 = board.cx / CELL_XSIZE;
            board.ay0 = board.cy / CELL_YSIZE;
            board.state = SELECT_TARGET;
         }
      } else
         if (board.state == SELECT_TARGET)
            board_move_done();
         else                           /* all other states are spells */
            board_spell_done();         /* (except SELECT_FIELD, but we'll */
   }                                    /* never get here in that state) */
}

/*--------------------------------------------------------------------------*/
/* board_cursor                                                             */
/*--------------------------------------------------------------------------*/

void board_cursor(void)
{
   int old_state;
   int old_cx, old_cy;
   
   old_state = board.state;
   old_cx = board.cx;
   old_cy = board.cy;

   board.cx = vmax(0, vmin(board.width - CELL_X(1),
         board.cx + state_move_x_step[board.cstate] * CELL_XSIZE / 8));
   board.cy = vmax(0, vmin(board.height - CELL_Y(1),
         board.cy + state_move_y_step[board.cstate] * CELL_YSIZE / 8));

   if (board.cx != old_cx || board.cy != old_cy) {
      board_paint_cell(old_cx / CELL_XSIZE, old_cy / CELL_YSIZE);
      if (CELL_XOFFSET(old_cx) != 0)
         board_paint_cell(old_cx / CELL_XSIZE + 1, old_cy / CELL_YSIZE);
      if (CELL_YOFFSET(old_cy) != 0) {
         board_paint_cell(old_cx / CELL_XSIZE, old_cy / CELL_YSIZE + 1);
         if (CELL_XOFFSET(old_cx) != 0)
            board_paint_cell(old_cx / CELL_XSIZE + 1, old_cy / CELL_YSIZE + 1);
      }
      board_paint_cursor();
   }

   if (CELL_XOFFSET(board.cx) == 0 && CELL_YOFFSET(board.cy) == 0) {
      board.cstate = 0;
      board_input();
   }

   if (board.cstate != 0 || board.state != old_state) {
      if (board.state == SELECT_SOURCE)
         board_message("%s plays", board.side_name[board.side]);
      if (board.state == SELECT_TARGET)
         board_message("%s (%s %d)",
                       board_cells[board.ay0][board.ax0].actor->name,
                       board_cells[board.ay0][board.ax0].actor->type &
                       ACTOR_GROUND ? "ground" :
                       board_cells[board.ay0][board.ax0].actor->type &
                       ACTOR_FLY ? "fly" : "teleport",
                       board_cells[board.ay0][board.ax0].actor->distance);
   }
}

/*--------------------------------------------------------------------------*/
/* board_spell                                                              */
/*--------------------------------------------------------------------------*/

void board_spell(void)
{
   int fire_up = 0;
   int old_spell;

   iface_frame();
   if (board.game_status == 0)
      return;
   
   if (iface_key_down(STATE_FIRE))
      board.fire_down = 1;
   if (board.fire_down && !iface_key_down(STATE_FIRE)) {
      board.fire_down = 0;
      fire_up = 1;
   }
   if (board.spell == 0 || board.state == SELECT_SPELL_DONE) {
      if (fire_up) {
         if (board.state == SELECT_SPELL_DONE) {
            board.cx = CELL_X(board.ax0);
            board.cy = CELL_Y(board.ay0);
            board_end_turn(board.spell);
         } else
            for (board.spell = 1; !spell_avails[board.side][board.spell]; board.spell++)
               ;
      }
      return;
   }

   board_message("select spell: %s", spell_names[board.spell]);
   old_spell = board.spell;
   if (iface_key_down(STATE_MOVE_UP))
      board.prev_down = 1;
   if (board.prev_down && !iface_key_down(STATE_MOVE_UP)) {
      board.prev_down = 0;
      do {
         board.spell--;
      } while (board.spell > SPELL_TELEPORT && !spell_avails[board.side][board.spell]);
      if (!spell_avails[board.side][board.spell])
         board.spell = old_spell;
   }
   if (iface_key_down(STATE_MOVE_DOWN))
      board.next_down = 1;
   if (board.next_down && !iface_key_down(STATE_MOVE_DOWN)) {
      board.next_down = 0;
      do {
         board.spell++;
      } while (board.spell < SPELL_CEASE_CONJURING && !spell_avails[board.side][board.spell]);
      if (!spell_avails[board.side][board.spell])
         board.spell = old_spell;
   }

   if (fire_up)
      spell_funcs[board.spell](0, 0, 0);
}

/*--------------------------------------------------------------------------*/
/* board_teleport                                                           */
/*--------------------------------------------------------------------------*/

void board_teleport(int x, int y, int side)
{
   static int x0, y0;

   switch (board.state) {
      case SELECT_SPELL:
         board_message("who would you like to teleport?");
         board.state = T_SELECT_SOURCE;
         break;
      case T_SELECT_SOURCE:
         if (side != board.side || board_cells[y][x].flags & CELL_IMPRISON)
            return;
         x0 = x;
         y0 = y;
         board_message("where would you like to teleport it?");
         board.state = T_SELECT_TARGET;
         break;
      case T_SELECT_TARGET:
         if (side == board.side)
            return;
         if (!board.ssp.complete) {
            board_ssp_init(x0, y0, x, y, 0);
            return;
         }
         spell_avails[board.side][SPELL_TELEPORT] = 0;
         if (side == -1) {
            board_cells[y][x].actor = board_cells[y0][x0].actor;
            board_cells[y0][x0].actor = NULL;
            board_reset_actor(board_cells[y][x].actor);
            board_end_turn(1);
         } else
            board_field(x, y, x0, y0);
         break;
   }
}

/*--------------------------------------------------------------------------*/
/* board_heal                                                               */
/*--------------------------------------------------------------------------*/

void board_heal(int x, int y, int side)
{
   if (board.state == SELECT_SPELL) {
      board_message("who would you like to heal?");
      board.state = H_SELECT_TARGET;
   } else {                             /* H_SELECT_TARGET */
      if (side != board.side)
         return;
      spell_avails[board.side][SPELL_HEAL] = 0;
      board_cells[y][x].actor->strength =
         actors_list[board_cells[y][x].actor->type & ACTOR_MASK].strength;
      board_message("this %s is now healed", board_cells[y][x].actor->name);
      board.state = SELECT_SPELL_DONE;
      board.spell = 1;
   }
}

/*--------------------------------------------------------------------------*/
/* board_shift_time                                                         */
/*--------------------------------------------------------------------------*/

void board_shift_time(int x, int y, int side)
{
   ACTOR *actor;

   spell_avails[board.side][SPELL_SHIFT_TIME] = 0;
   board.lumi = (board.lumi == LUMI_DARKEST) ? LUMI_LIGHTEST :
                (board.lumi == LUMI_LIGHTEST) ? LUMI_DARKEST :
                board.lumi;
   board.lumi_d = -board.lumi_d;

   for (y = 0; y < board.cell_h; y++)
      for (x = 0; x < board.cell_w; x++)
         if (board_cells[y][x].flags & CELL_IMPRISON) {
            actor = board_cells[y][x].actor;
            /* un-imprison light actor if luminance cycle is lightest; */
            /* un-imprison dark actor if luminance cycle is darkest.   */
            if ((board.lumi == LUMI_LIGHTEST && actor->type & ACTOR_LIGHT) ||
                (board.lumi == LUMI_DARKEST && !(actor->type & ACTOR_LIGHT)))
               board_cells[y][x].flags ^= CELL_IMPRISON;
         }

   board_message("the flow of time is reversed");
   board.state = SELECT_SPELL_DONE;
   board.spell = 1;
}

/*--------------------------------------------------------------------------*/
/* board_exchange                                                           */
/*--------------------------------------------------------------------------*/

void board_exchange(int x, int y, int side)
{
   static int x0, y0;
   ACTOR *actor;

   switch (board.state) {
      case SELECT_SPELL:
         board_message("who would you like to exchange?");
         board.state = X_SELECT_SOURCE;
         break;
      case X_SELECT_SOURCE:
         x0 = x;
         y0 = y;
         board_message("whom to exchange with?");
         board.state = X_SELECT_TARGET;
         break;
      case X_SELECT_TARGET:
         if (!board.ssp.complete) {
            board_ssp_init(x0, y0, x, y, 1);
            return;
         }
         spell_avails[board.side][SPELL_EXCHANGE] = 0;
         actor = board_cells[y0][x0].actor;
         board_cells[y0][x0].actor = board_cells[y][x].actor;
         board_cells[y][x].actor = actor;
         board_reset_actor(board_cells[y0][x0].actor);
         board_reset_actor(board_cells[y][x].actor);
         board_end_turn(1);
         break;
   }
}

/*--------------------------------------------------------------------------*/
/* board_summon_elemental                                                   */
/*--------------------------------------------------------------------------*/

void board_summon_elemental(int x, int y, int side)
{
   static char *elem_names[] = { "an air", "an earth", "a fire", "a water" };
   int y0, x0;

   if (board.state == SELECT_SPELL) {
      if (board.elem.actor == NULL) {
         board.elem.flags = board_frame_time % 4;
         board_init_cell(&board.elem, ACTOR_AIR_ELEM + board.elem.flags);
      }
      board_message("%s elemental appears", elem_names[board.elem.flags]);
      sprite_set_state(board.elem.actor->sprite, STATE_AIR_ELEM + board.elem.flags, 0);
      board.state = L_SELECT_TARGET;
      board_paint_cursor();

   } else {                             /* L_SELECT_TARGET */
      if (side == board.side || side == -1)
         return;
      spell_avails[board.side][SPELL_SUMMON_ELEMENTAL] = 0;
      for (y0 = 0; y0 < board.cell_h; y++)
         for (x0 = 0; x0 < board.cell_w; x0++)
            if (board_cells[y0][x0].actor == NULL) {
               board.elem.actor->type |= (!board.side) * ACTOR_LIGHT;
               board_cells[y0][x0].actor = board.elem.actor;
               board_field(x, y, x0, y0);
               return;                  /* there should be lots of empty */
            }                           /* cells, so we will always get here */
   }
}

/*--------------------------------------------------------------------------*/
/* board_revive_check                                                       */
/*--------------------------------------------------------------------------*/

int board_revive_check(int *actors, int *cell_x, int *cell_y)
{
   int num_actors;
   int first, last, i;
   int y, x, dx;
   int init_count, real_count;
   int master_x, master_y;

   num_actors = 0;
   first = (board.side == 0) ? ACTOR_LIGHT_FIRST : ACTOR_DARK_FIRST;
   last = (board.side == 0) ? ACTOR_LIGHT_LAST : ACTOR_DARK_LAST;
   for (i = first; i <= last; i++) {
      init_count = real_count = 0;
      for (y = 0; y < board.cell_h; y++)
         for (x = 0; x < board.cell_w; x++) {
            if ((init_board_cells[board.game][y][x] & ACTOR_MASK) == i)
               init_count++;
            if (board_cells[y][x].actor != NULL && (board_cells[y][x].actor->type & ACTOR_MASK) == i)
               real_count++;
         }
      if (init_count != real_count)
         actors[num_actors++] = i;
   }
   if (num_actors == 0) {
      if (cell_x != NULL)               /* only if interactive mode */
         board_message("happily, master, all are alive");
      return 0;
   }
   actors[num_actors] = 0;

   board_find_actor((board.side == 0 ? ACTOR_WIZARD : ACTOR_SORCERESS),
                    &master_x, &master_y);
   dx = (board.side == 0) ? 1 : -1;
   for (y = master_y - 1; y <= master_y + 1; y++)
      for (x = master_x - dx; x != master_x + (dx * 2); x += dx)
         if (y >= 0 && y < board.cell_h && x >= 0 && x < board.cell_w &&
             board_cells[y][x].actor == NULL) {
            if (cell_x != NULL) {       /* only if interactive mode */
               *cell_x = x;
               *cell_y = y;
            }
            return 1;
         }
   if (cell_x != NULL)                  /* only if interactive mode */
      board_message("There is no open square near your %s.",
            board_cells[master_y][master_x].actor->name);
   return 0;
}

/*--------------------------------------------------------------------------*/
/* board_revive_frame                                                       */
/*--------------------------------------------------------------------------*/

int board_revive_frame(int *i, int *actors)
{
   int old_i = *i;

   if (*i < 0)
      for (*i = 0; actors[*i]; (*i)++)
         ;
   if (iface_key_down(STATE_FIRE))
      board.fire_down = 1;
   if (board.fire_down && !iface_key_down(STATE_FIRE)) {
      board.fire_down = 0;
      return 1;
   }

   if (iface_key_down(STATE_MOVE_UP))
      board.prev_down = 1;
   if (board.prev_down && !iface_key_down(STATE_MOVE_UP)) {
      board.prev_down = 0;
      if ((*i) > 0)
         (*i)--;
   }
   if (iface_key_down(STATE_MOVE_DOWN))
      board.next_down = 1;
   if (board.next_down && !iface_key_down(STATE_MOVE_DOWN)) {
      board.next_down = 0;
      if (actors[(*i)] != 0)
         (*i)++;
   }

   if (*i != old_i) {
      sprite_set_state(cursor_sprite, STATE_BOARD, 1);  /* erase cursor */
      sprite_paint(cursor_sprite, STATE_BOARD,
                   (board.side == 0 ? CELL_XSIZE : CANVAS_WIDTH - CELL_XSIZE * 2),
                   old_i * V_CELL_YSIZE + CELL_YSIZE);
      sprite_set_state(cursor_sprite, STATE_BOARD, 2);  /* draw cursor */
      sprite_paint(cursor_sprite, STATE_BOARD,
                   (board.side == 0 ? CELL_XSIZE : CANVAS_WIDTH - CELL_XSIZE * 2),
                   (*i) * V_CELL_YSIZE + CELL_YSIZE);
      board.any_output = 1;
   }

   return 0;
}

/*--------------------------------------------------------------------------*/
/* board_revive                                                             */
/*--------------------------------------------------------------------------*/

void board_revive(int x, int y, int side)
{
   static int actors[10];
   static int cell_x, cell_y;
   int facing, x_pos;
   int i;

   if (board.ssp.complete) {
      board.cx = CELL_X(cell_x);        /* once ssp is complete, place */
      board.cy = CELL_Y(cell_y);        /*    cursor on revived cell */
      board_end_turn(1);
      return;
   }

   if (x >= 0) {
      if (!board_revive_check(actors, &cell_x, &cell_y)) {
         board.state = SELECT_SPELL_DONE;
         board.spell = 0;
         return;
      }
      board_message("select who to revive");
      if (board_cells[board.ay0][board.ax0].actor->type & ACTOR_RIGHT) {
         facing = STATE_MOVE_RIGHT;
         x_pos = CELL_XSIZE;
      } else {
         facing = STATE_MOVE_LEFT;
         x_pos = CANVAS_WIDTH - CELL_XSIZE*2;
      }
      for (i = 0; actors[i] != 0; i++)
         sprite_paint(actors_list[actors[i]].sprite, facing, x_pos,
                      i*V_CELL_YSIZE + CELL_YSIZE);
      board.cy = -1;                        /* force redraw */
      board.state = V_SELECT_SOURCE;
   }

   if (board_revive_frame(&board.cy, actors)) {
      if (actors[board.cy] != 0) {
         spell_avails[board.side][SPELL_REVIVE] = 0;
         board_init_cell(&board_cells[cell_y][cell_x], actors[board.cy]);
         board_ssp_init(cell_x, cell_y, cell_x, cell_y, 0);
         board.cx = CELL_X(1);          /* make sure cursor is not on a */
         board.cy = CELL_Y(1);          /*    power cell (temporarily) */
      } else {
         board.cx = CELL_X(board.ax0);  /* put the cursor on the master */
         board.cy = CELL_Y(board.ay0);
         board_end_turn(0);
      }
   }
}

/*--------------------------------------------------------------------------*/
/* board_imprison                                                           */
/*--------------------------------------------------------------------------*/

void board_imprison(int x, int y, int side)
{
   if (board.state == SELECT_SPELL) {
      if (!board_is_imprison_ok()) {
         board_message("%s cannot be imprisoned at this time",
                       board.side_name[board.side]);
         board.state = SELECT_SPELL_DONE;
         board.spell = 0;
      } else {
         board_message("who would you like to imprison?");
         board.state = M_SELECT_TARGET;
      }

   } else
      if (side != board.side) {
         spell_avails[board.side][SPELL_IMPRISON] = 0;
         board_cells[y][x].flags |= CELL_IMPRISON;
         board_message("this %s is now imprisoned",
                       board_cells[y][x].actor->name);
         board.state = SELECT_SPELL_DONE;
         board.spell = 1;
      }
}

/*--------------------------------------------------------------------------*/
/* board_cease_conjuring                                                    */
/*--------------------------------------------------------------------------*/

void board_cease_conjuring(int x, int y, int side)
{
   board_end_turn(0);                   /* reset this turn */
}

/*--------------------------------------------------------------------------*/
/* board_start_game                                                         */
/*--------------------------------------------------------------------------*/

void board_start_game(int game, int light_first)
{
   int x, y;
   CELL *cell;

   /* setup font stuff */
   if (font.ptr == NULL) {
      font.ptr = canvas_font_load(FONT_NAME);
      font.fg = canvas_alloc_color(255, 255, 0);
      font.bg = canvas_alloc_color(255, 0, 0);
   }

   /* setup initial board[] data */
   memset(&board, 0, sizeof(board));
   board.game_status = 1;               /* game on! */
   board.side = !light_first;
   board.state = SELECT_SOURCE;

   board.game = game;
   switch (board.game) {
      case GAME_ARCHON:
         board.cell_w = 9;
         board.cell_h = 9;
         board.side_name[0] = "the light side";
         board.side_name[1] = "the dark side";
         break;
      case GAME_ADEPT:
         board.cell_w = 17;
         board.cell_h = 9;
         board.side_name[0] = "chaos";
         board.side_name[1] = "order";
         break;
   }
   board.width = CELL_X(board.cell_w);
   board.height = CELL_Y(board.cell_h);
   board.corner_x = CELL_X((CANVAS_XCELLS - board.cell_w)/2.0);
   board.corner_y = CELL_Y((CANVAS_YCELLS - board.cell_h)/2.0);
   board.lumi   = light_first ? LUMI_LIGHTEST : LUMI_DARKEST;
   board.lumi_d = light_first ? -1            : 1;
   board.cx     = light_first ? CELL_X(0)     : board.width - CELL_X(1);
   board.cy     = CELL_Y(board.cell_h / 2);
   board.ax0 = -1;
   board.elem.actor = NULL;
   sprintf(board.message, "%s goes first", board.side_name[board.side]);
   board.any_output = 0;

   /* setup actors on the board */
   for (y = 0; y < board.cell_h; y++)
      for (x = 0; x < board.cell_w; x++) {
         cell = &board_cells[y][x];
         cell->flags = init_board_cells[board.game][y][x];
         if ((cell->flags & ACTOR_MASK) != 0)
            board_init_cell(cell, cell->flags & ACTOR_MASK);
         else
            cell->actor = NULL;

      }

   /* setup global variables */
   memcpy(spell_avails[0], spell_avails[2], sizeof(spell_avails[2]));
   memcpy(spell_avails[1], spell_avails[2], sizeof(spell_avails[2]));
   board_frame_time = 0;
   board_refresh();
   board_iface_turn();
   audio_start_game();
   /*
   sleep(2);
   audio_start_turn(board.side);
   */
}

/*--------------------------------------------------------------------------*/
/* board_end_game                                                           */
/*--------------------------------------------------------------------------*/

int board_end_game(void)
{
   int x, y;

   if (board.game_status == 0)
      return 0;
   if (board.state == SELECT_FIELD)
      field_end_game();
   for (y = 0; y < board.cell_h; y++)
      for (x = 0; x < board.cell_w; x++)
         if (board_cells[y][x].actor != NULL) {
            if (board_cells[y][x].actor == board.elem.actor)
               board.elem.actor = NULL;
            board_clear_cell(&board_cells[y][x]);
         }
   if (board.elem.actor != NULL)
      board_clear_cell(&board.elem);
   board.game_status = 0;               /* game ends */
   board.state = SELECT_GAME_OVER;
   audio_terminate();
   return 1;
}

/*--------------------------------------------------------------------------*/
/* board_pause_game                                                         */
/*--------------------------------------------------------------------------*/

int board_pause_game(int pause)
{
   if (board.game_status == 0)
      return 0;
   if (pause != -1)
      board.game_status = pause ? -1 : 1;
   return 1;
}

/*--------------------------------------------------------------------------*/
/* board_refresh                                                            */
/*--------------------------------------------------------------------------*/

void board_refresh(void)
{
   int x, y;

   if (board.state == SELECT_FIELD) {
      field_refresh();
      return;
   }

   canvas_clear();
   for (y = 0; y < board.cell_h; y++)
      for (x = 0; x < board.cell_w; x++)
         board_paint_cell(x, y);
   board_message(NULL);
   board_paint_cursor();
   canvas_refresh();
}

/*--------------------------------------------------------------------------*/
/* board_frame                                                              */
/*--------------------------------------------------------------------------*/

int board_frame(void)
{
#ifdef AUTOPILOT
   if (board.turn == 1000) {
      printf("board:  game is taking too long, ending it.\n");
      board_end_game();
   }
#endif

   if (board.game_status != 1)
      return (board.state != SELECT_GAME_OVER);

   if (board.state != SELECT_FIELD)
      board_animate_board();

   if (board.ssp.count != 0)
      board_ssp_frame();
   else {

      switch (board.state) {
         case SELECT_SPELL:
         case SELECT_SPELL_DONE:
            board_spell();
            break;
         case SELECT_FIELD:
            board_field(0, 0, 0, 0);
            break;
         case V_SELECT_SOURCE:
            iface_frame();
            if (board.game_status != 0)
               board_revive(-99, 0, 0);
            break;
         default:
            board_cursor();
            break;
      }
      if (board.state == SELECT_GAME_OVER)
         board_end_game();
   }

   if (board.any_output) {
      canvas_refresh();
      board.any_output = 0;
   }
   board_frame_time++;

   return (board.state != SELECT_GAME_OVER);
}

/*--------------------------------------------------------------------------*/
/* board_cell_lumi                                                          */
/*--------------------------------------------------------------------------*/

int board_cell_lumi(CELL *cell)
{
   if (cell->flags & CELL_DARK)
      return CELL_DARK | LUMI_DARKEST;
   if (cell->flags & CELL_LIGHT)
      return CELL_LIGHT | LUMI_LIGHTEST;
   if (board.lumi < LUMI_COUNT / 2)
      return CELL_DARK | board.lumi;
   else
      return CELL_LIGHT | board.lumi;
}

/*--------------------------------------------------------------------------*/
/* board_is_pickable                                                        */
/*--------------------------------------------------------------------------*/

int board_is_pickable(int cx, int cy, int msg)
{
   int x0, y0;
   int x, y;
   int distance;

   x0 = cx;
   y0 = cy;
   if (board_cells[y0][x0].actor == NULL)
      return 0;
   if (!actor_is_side(board_cells[y0][x0].actor, board.side))
      return 0;
   if (board_cells[y0][x0].flags & CELL_IMPRISON) {
      if (msg)
         board_message("alas, master, this %s is imprisoned",
                       board_cells[y0][x0].actor->name);
      return 0;
   }
   if ((board_cells[y0][x0].actor->type & ACTOR_MASTER) == ACTOR_MASTER)
      return 1;
   distance = (board_cells[y0][x0].actor->type & ACTOR_GROUND) ? 1 : (board_cells[y0][x0].actor->distance);
   for (y = y0 - distance; y <= y0 + distance; y++)
      if (y >= 0 && y <= board.cell_h - 1)
         for (x = x0 - distance; x <= x0 + distance; x++) {
            if (x < 0 || x > board.cell_w - 1)
               continue;
            if (board_cells[y0][x0].actor->type & ACTOR_GROUND &&
                vabs(x - x0) == 1 && vabs(y - y0) == 1)
               continue;
            if (board_cells[y][x].actor == NULL)
               return 1;
            if (actor_is_side(board_cells[y][x].actor, !board.side))
               return 1;
         }
   if (msg)
      board_message("alas, master, this %s cannot move", board_cells[y0][x0].actor->name);
   return 0;
}

/*--------------------------------------------------------------------------*/
/* board_get_route_2                                                        */
/*--------------------------------------------------------------------------*/

int board_get_route_2(int x1, int y1, int x2, int y2, int *route,
                      int prev_state, int light, int ground, int distance)
{
   int state, state_last;
   int x, y;
   int count;

   if (distance == 0)
      return 0;
   state_last = ground ? STATE_MOVE_AXIAL_LAST : STATE_MOVE_LAST;
   for (state = STATE_MOVE_FIRST; state <= state_last; state++) {
      if (state == state_opposite[prev_state])
         continue;
      x = vmax(0, vmin(board.cell_w - 1, x1 + state_move_x_step[state]));
      y = vmax(0, vmin(board.cell_h - 1, y1 + state_move_y_step[state]));
      if (x == x1 && y == y1)
         continue;
      if (ground && board_cells[y][x].actor != NULL &&
          (board_cells[y][x].actor->type & ACTOR_LIGHT) == light)
         continue;
      *route = state;
      if (x == x2 && y == y2)
         return 1;
      if (ground && board_cells[y][x].actor != NULL)
         continue;
      count = board_get_route_2(x, y, x2, y2, route + 1, state, light, ground, distance - 1);
      if (count != 0)
         return (count + 1);
   }
   return 0;
}

/*--------------------------------------------------------------------------*/
/* board_get_route                                                          */
/*--------------------------------------------------------------------------*/

int *board_get_route(int x1, int y1, int x2, int y2)
{
   static int route[16];
   ACTOR *actor;
   int count;

   route[0] = 0;
   if (x1 == x2 && y1 == y2)
      return NULL;
   actor = board_cells[y1][x1].actor;
   if (board_cells[y2][x2].actor != NULL &&
       (board_cells[y2][x2].actor->type & ACTOR_LIGHT) == (actor->type & ACTOR_LIGHT))
      return NULL;
   count = board_get_route_2(x1, y1, x2, y2, route,
                             0,
                             (actor->type & ACTOR_LIGHT),
                             (actor->type & ACTOR_GROUND),
                             actor->distance);
   route[count] = 0;
   return (count == 0) ? NULL : route;
}

/*--------------------------------------------------------------------------*/
/* board_find_actor                                                         */
/*--------------------------------------------------------------------------*/

int board_find_actor(int type, int *ax, int *ay)
{
   int x, y;

   for (y = 0; y < board.cell_h; y++)
      for (x = 0; x < board.cell_w; x++)
         if (board_cells[y][x].actor != NULL &&
             (board_cells[y][x].actor->type & ACTOR_MASK) == type) {
            *ax = x;
            *ay = y;
            return 1;
         }
   *ax = -1;
   *ay = -1;
   return 0;
}

/*--------------------------------------------------------------------------*/
/* board_is_imprison_ok                                                     */
/*--------------------------------------------------------------------------*/

int board_is_imprison_ok(void)
{
   int next_lumi;

   /* don't allow if luminance on next turn is max of enemy's side */
   next_lumi = board.lumi + board.lumi_d * ((board.turn + 1) % 2 == 0);
   if ((board.side == 0 && next_lumi == LUMI_DARKEST) ||
       (board.side == 1 && next_lumi == LUMI_LIGHTEST))
      return 0;
   return 1;
}

/*--------------------------------------------------------------------------*/
/* board_get_data                                                           */
/*--------------------------------------------------------------------------*/

void board_get_data(int *_side, int *_turn, int *_lumi, int *_lumi_d)
{
   *_side = board.side;
   *_turn = board.turn;
   *_lumi = board.lumi;
   *_lumi_d = board.lumi_d;
}

/*--------------------------------------------------------------------------*/
/* board_absolute_control_delta                                             */
/*--------------------------------------------------------------------------*/

void board_absolute_control_delta(int *dx, int *dy)
{
   int spell;

   switch (board.state) {
      case SELECT_SPELL:
         *dx = 0;
         /* don't just hold the direction down, or the selection will get
          * stuck on the first option */
         if (board.prev_down || board.next_down)
            *dy = 0;
         else {
            *dy = (*dy/CELL_YSIZE) - 2;
            for (spell = 0; spell < board.spell; spell++)
               if (spell_avails[board.side][spell])
                  (*dy)--;
         }
         break;
      case V_SELECT_SOURCE:
         *dx = 0;
         /* don't just hold the direction down, or the selection will get
          * stuck on the first option */
         if (board.prev_down || board.next_down)
            *dy = 0;
         else
            *dy = ((*dy - CELL_YSIZE)/V_CELL_YSIZE) - board.cy;
         break;
      default:
         *dx -= CELL_XOFFSET(*dx) + BOARD_X(0) + board.cx;
         *dy -= CELL_YOFFSET(*dy) + BOARD_Y(0) + board.cy;
         break;
   }
}

/*--------------------------------------------------------------------------*/
/* board_get_cell_width                                                     */
/*--------------------------------------------------------------------------*/

int board_get_cell_width()
{
   return board.cell_w;
}

/*--------------------------------------------------------------------------*/
/* board_get_cell_height                                                    */
/*--------------------------------------------------------------------------*/

int board_get_cell_height()
{
   return board.cell_h;
}

