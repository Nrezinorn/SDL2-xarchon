/*--------------------------------------------------------------------------*/
/* interface                                                                */
/*--------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <guile/gh.h>

#include "iface.h"
#include "actors.h"
#include "canvas.h"
#include "misc.h"
#include "board.h"
#include "human.h"
#include "scheme.h"

/*--------------------------------------------------------------------------*/
/* defines                                                                  */
/*--------------------------------------------------------------------------*/

#define REPLY_MOVE_MASK 0x1F
#define REPLY_FIRE      0x20
#define REPLY_LAST      0x40

#define INSERT_REPLY(x) \
   side->replies[side->num_replies] = x; \
   side->num_replies++;

/*--------------------------------------------------------------------------*/
/* structures                                                               */
/*--------------------------------------------------------------------------*/

typedef struct {
   int which;                           /* 0=light, 1=dark */
   int mode;
   void *data;
   void (*turn_func)(void *data, int mode, COMMAND *cmd);
   void (*frame_func)(int *keys_down);
   void (*notify_func)(void *data, int mode, COMMAND *cmd);
   unsigned char replies[200];
   int num_replies;
   int next_reply;
} SIDE;

/*--------------------------------------------------------------------------*/
/* functions                                                                */
/*--------------------------------------------------------------------------*/

static void iface_board_reply(COMMAND *cmd);
static void iface_cursor_reply(int *cx, int *cy, int x, int y);

/*--------------------------------------------------------------------------*/
/* variables                                                                */
/*--------------------------------------------------------------------------*/

SIDE sides[2] = { {}, {} };
SIDE *side;

static int keys_down[STATE_MOVE_COUNT];

/*--------------------------------------------------------------------------*/
/* iface_start                                                              */
/*--------------------------------------------------------------------------*/

void iface_start(char *light, char *dark)
{
   int i;
   char *name;

   for (i = 0; i < 2; i++) {
      name = (i == 0) ? light : dark;
      side = &sides[i];
      side->which = i;
      if (side->data != NULL) {
         free(side->data);
         side->data = NULL;
      }
      if (strcasecmp(name, IFACE_HUMAN) == 0) {
         side->turn_func = human_turn;
         side->frame_func = human_frame;
         continue;
      }
      side->data = scheme_start(side->which, name);
      side->turn_func = scheme_turn;
      side->frame_func = NULL;
   }
}

/*--------------------------------------------------------------------------*/
/* iface_turn                                                               */
/*--------------------------------------------------------------------------*/

void iface_turn(int which, int mode)
{
   COMMAND cmd;

   side = &sides[which];
   side->mode = mode;
   cmd.b.spell = -12345;                /* magic number */
   side->turn_func(side->data, mode, &cmd);
   if (cmd.b.spell != -12345) {         /* if command was filled */
      side->num_replies = 0;
      side->next_reply = 0;
      if (mode == IFACE_BOARD)
         iface_board_reply(&cmd);
      else {                            /* IFACE_FIELD */
         INSERT_REPLY(cmd.f.dir | ((cmd.f.fire) ? REPLY_FIRE : 0));
         INSERT_REPLY(REPLY_LAST);
      }
   }
}

/*--------------------------------------------------------------------------*/
/* iface_board_reply                                                        */
/*--------------------------------------------------------------------------*/

void iface_board_reply(COMMAND *cmd)
{
   int cx, cy;

   cx = (side->which == 0) ? 0 : BOARD_XCELLS - 1;
   cy = BOARD_YCELLS / 2;

   if (cmd->b.spell == 0)               /* no spell */ {
      iface_cursor_reply(&cx, &cy, cmd->b.x1, cmd->b.y1);
      INSERT_REPLY(REPLY_FIRE);
      INSERT_REPLY(0);                  /* indicate release of fire key */
      iface_cursor_reply(&cx, &cy, cmd->b.x2, cmd->b.y2);
      INSERT_REPLY(REPLY_FIRE);
      INSERT_REPLY(0);                  /* indicate release of fire key */
      INSERT_REPLY(REPLY_LAST);
      return;
   }

   printf("NO SPELL SUPPORT YET\n");
}

/*--------------------------------------------------------------------------*/
/* iface_cursor_reply                                                       */
/*--------------------------------------------------------------------------*/

void iface_cursor_reply(int *cx, int *cy, int x, int y)
{
   while (*cy != y)
      if (*cy < y) {
         INSERT_REPLY(STATE_MOVE_DOWN);
         (*cy)++;
      } else {
         INSERT_REPLY(STATE_MOVE_UP);
         (*cy)--;
      }
   while (*cx != x)
      if (*cx < x) {
         INSERT_REPLY(STATE_MOVE_RIGHT);
         (*cx)++;
      } else {
         INSERT_REPLY(STATE_MOVE_LEFT);
         (*cx)--;
      }
}

/*--------------------------------------------------------------------------*/
/* iface_frame                                                              */
/*--------------------------------------------------------------------------*/

void iface_frame(void)
{
   int i;
   unsigned char r;

   if (side->frame_func != NULL) {
      side->frame_func(keys_down);
      return;
   }

   r = side->replies[side->next_reply];
   if (r == REPLY_LAST) {
      fprintf(stderr, "iface_frame():  reply is too short\n");
      exit(EXIT_FAILURE);
   }
   for (i = STATE_MOVE_FIRST; i <= STATE_MOVE_LAST; i++)
      keys_down[i] = (i == (r & REPLY_MOVE_MASK));
   keys_down[0] = ((r & REPLY_FIRE) == REPLY_FIRE);
   side->next_reply++;
}

/*--------------------------------------------------------------------------*/
/* iface_key_down                                                           */
/*--------------------------------------------------------------------------*/

int iface_key_down(int key)
{
   return keys_down[key];
}

/*--------------------------------------------------------------------------*/
/* iface_notify                                                             */
/*--------------------------------------------------------------------------*/

void iface_notify(COMMAND *cmd)
{
   if (side->notify_func != NULL)
      side->notify_func(side->data, side->mode, cmd);
}
