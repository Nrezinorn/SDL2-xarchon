/*--------------------------------------------------------------------------*/
/* interface                                                                */
/*--------------------------------------------------------------------------*/

#include <config.h>

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "iface.h"
#include "actors.h"
#include "main.h"
#include "board.h"
#include "human.h"
#include "computer.h"
#include "network.h"

/*--------------------------------------------------------------------------*/
/* defines                                                                  */
/*--------------------------------------------------------------------------*/

#define REPLY_MOVE_MASK 0x1F
#define REPLY_FIRE      0x20
#define REPLY_LAST      0x40

#define INSERT_REPLY(x) { \
   side->replies[side->num_replies] = x; \
   side->num_replies++; \
}

/*--------------------------------------------------------------------------*/
/* structures                                                               */
/*--------------------------------------------------------------------------*/

typedef struct {
   int num;
   int iface;
   int mode;
   void (*turn_func)(int side_num, int mode, COMMAND *cmd);
   void (*frame_func)(int *keys_down);
   unsigned char replies[200];
   int num_replies;
   int next_reply;
} SIDE;

/*--------------------------------------------------------------------------*/
/* functions                                                                */
/*--------------------------------------------------------------------------*/

static void iface_board_reply(int side_num, COMMAND *cmd);
static void iface_cursor_reply(int *cx, int *cy, int x, int y);
static void iface_spell_reply(int side_num, int spell, int *cx, int *cy);
static void iface_teleport_exchange(COMMAND *cmd, int *cx, int *cy);
static void iface_heal_imprison_shift_summon(COMMAND *cmd, int *cx, int *cy);
static void iface_revive(COMMAND *cmd, int *cx, int *cy);

/*--------------------------------------------------------------------------*/
/* variables                                                                */
/*--------------------------------------------------------------------------*/

static IFACE_CONFIG iface_config;

SIDE sides[2] = { {}, {} };
SIDE *side;

static int keys_down[STATE_MOVE_COUNT];

static void (*iface_spell_funcs[SPELL_COUNT_2])(COMMAND *cmd, int *cx, int *cy) = {
   NULL,
   iface_teleport_exchange,             /* teleport */
   iface_heal_imprison_shift_summon,    /* heal */
   iface_heal_imprison_shift_summon,    /* shift time */
   iface_teleport_exchange,             /* exchange */
   iface_heal_imprison_shift_summon,    /* summon elemental */
   iface_revive,
   iface_heal_imprison_shift_summon,    /* imprison */
   NULL,
   NULL
};

static COMMAND computer_last_command;

/*--------------------------------------------------------------------------*/
/* iface_start                                                              */
/*--------------------------------------------------------------------------*/

char *iface_start(int *light_first)
{
   IFACE_PLAYER *player, *players[2];
   int i;

   players[0] = players[1] = NULL;
   for (player = list_head(&iface_config.players); player != NULL; player = list_next(player)) {
      if (player->name == NULL)
         continue;
      if (strcmp(player->name, iface_config.light_name) == 0)
         players[0] = player;
      if (strcmp(player->name, iface_config.dark_name) == 0)
         players[1] = player;
   }
   if (players[0] == NULL || players[1] == NULL)
      return "cannot start:  no players specified";

   if (players[0]->type == IFACE_NETWORK && players[1]->type == IFACE_NETWORK)
      return "cannot start:  two network players selected";

   for (i = 0; i < 2; i++) {
      side = &sides[i];
      side->num = i;
      side->iface = players[i]->type;
      if (side->iface == IFACE_HUMAN) {
         human_start(i, players[i]->human);
         side->turn_func = human_turn;
         side->frame_func = human_frame;
      } else if (side->iface == IFACE_COMPUTER) {
         computer_start(i, players[i]->computer);
         side->turn_func = computer_turn;
         side->frame_func = NULL;
      } else if (side->iface == IFACE_NETWORK) {
         iface_config.light_first = 1;  /* force light starts */
         if (!network_start(i, players[i]->network))
            return "cannot start:  network player not connected";
         side->turn_func = network_turn;
         side->frame_func = network_frame;
      }
   }

   *light_first = iface_config.light_first;
   return NULL;
}

/*--------------------------------------------------------------------------*/
/* iface_turn                                                               */
/*--------------------------------------------------------------------------*/

void iface_turn(int side_num, int mode)
{
   COMMAND cmd;

   side = &sides[side_num];
   side->mode = mode;
   cmd.b.spell = -12345;                /* magic number */
   side->turn_func(side_num, mode, &cmd);
   if (cmd.b.spell != -12345) {         /* only if a reply was specified */
      side->num_replies = 0;
      side->next_reply = 0;
      if (mode == IFACE_BOARD) {
         memcpy(&computer_last_command, &cmd, sizeof(COMMAND));
         iface_board_reply(side_num, &cmd);
      } else {                          /* IFACE_FIELD */
         INSERT_REPLY(cmd.f.dir | ((cmd.f.fire) ? REPLY_FIRE : 0));
         INSERT_REPLY(REPLY_LAST);
      }
   }
}

/*--------------------------------------------------------------------------*/
/* iface_board_reply                                                        */
/*--------------------------------------------------------------------------*/

void iface_board_reply(int side_num, COMMAND *cmd)
{
   int cx, cy;
   int *route;

   cx = (side_num == 0) ? 0 : board_get_cell_width() - 1;
   cy = board_get_cell_height() / 2;

   if (cmd->b.spell == 0) {             /* no spell */
      iface_cursor_reply(&cx, &cy, cmd->b.x1, cmd->b.y1);
      INSERT_REPLY(REPLY_FIRE);
      INSERT_REPLY(0);                  /* indicate release of fire key */
      route = board_get_route(cmd->b.x1, cmd->b.y1, cmd->b.x2, cmd->b.y2);
      while (route != NULL && *route != 0) {
         INSERT_REPLY(*route);
         route++;
      }
      INSERT_REPLY(REPLY_FIRE);
      INSERT_REPLY(0);                  /* indicate release of fire key */

   } else {                             /* spell casting */
      iface_spell_reply(side_num, cmd->b.spell, &cx, &cy);
      iface_spell_funcs[cmd->b.spell](cmd, &cx, &cy);
   }

   INSERT_REPLY(REPLY_LAST);            /* end the reply */
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
/* iface_spell_reply                                                        */
/*--------------------------------------------------------------------------*/

void iface_spell_reply(int side_num, int spell, int *cx, int *cy)
{
   int x, y;
   int type;
   int i;

   /* find the master and move the cursor to it */
   type = (side_num == 0) ? ACTOR_WIZARD : ACTOR_SORCERESS;
   board_find_actor(type, &x, &y);
   iface_cursor_reply(cx, cy, x, y);

   /* invoke spell mode */
   INSERT_REPLY(REPLY_FIRE);            /* select master */
   INSERT_REPLY(0);                     /* indicate release of fire key */
   INSERT_REPLY(REPLY_FIRE);            /* re-select master for spell menu */
   for (i = 0; i < (FPS / 2); i++)      /* indicate release of fire key */
      INSERT_REPLY(0);                  /*   and pause for a while */
   INSERT_REPLY(REPLY_FIRE);            /* ok the "conjures a spell" msg */
   INSERT_REPLY(0);                     /* indicate release of fire key */

   /* scan spells, pressing DOWN key for spells that are available and */
   /* not the one we want, pressing FIRE when finding our spell */
   for (x = SPELL_FIRST; x <= SPELL_LAST; x++) {
      if (x == spell) {                 /* found our spell? */
         for (i = 0; i < (FPS / 2); i++)   /* pause for a while */
            INSERT_REPLY(0);
         INSERT_REPLY(REPLY_FIRE);
         INSERT_REPLY(0);               /* indicate release of fire key */
         break;
      }
      if (spell_avails[side_num][x]) {
         INSERT_REPLY(STATE_MOVE_DOWN);
         INSERT_REPLY(0);               /* indicate release of down key */
      }
   }
}

/*--------------------------------------------------------------------------*/
/* iface_teleport_exchange                                                  */
/*--------------------------------------------------------------------------*/

void iface_teleport_exchange(COMMAND *cmd, int *cx, int *cy)
{
   iface_cursor_reply(cx, cy, cmd->b.x1, cmd->b.y1);
   INSERT_REPLY(REPLY_FIRE);            /* select creature */
   INSERT_REPLY(0);                     /* indicate release of fire key */
   iface_cursor_reply(cx, cy, cmd->b.x2, cmd->b.y2);
   INSERT_REPLY(REPLY_FIRE);            /* select creature */
   INSERT_REPLY(0);                     /* indicate release of fire key */
}

/*--------------------------------------------------------------------------*/
/* iface_heal_imprison_shift_summon                                         */
/*--------------------------------------------------------------------------*/

void iface_heal_imprison_shift_summon(COMMAND *cmd, int *cx, int *cy)
{
   int i;

   if (cmd->b.spell != SPELL_SHIFT_TIME) {
      iface_cursor_reply(cx, cy, cmd->b.x1, cmd->b.y1);
      INSERT_REPLY(REPLY_FIRE);         /* select creature */
      INSERT_REPLY(0);                  /* indicate release of fire key */
   }
   if (cmd->b.spell != SPELL_SUMMON_ELEMENTAL) {
      for (i = 0; i < (FPS / 2); i++)   /* pause for a while */
         INSERT_REPLY(0);
      INSERT_REPLY(REPLY_FIRE);         /* acknowledge displayed message */
      INSERT_REPLY(0);                  /* indicate release of fire key */
   }
}

/*--------------------------------------------------------------------------*/
/* iface_revive                                                             */
/*--------------------------------------------------------------------------*/

void iface_revive(COMMAND *cmd, int *cx, int *cy)
{
   int actors[10], i;

   for (i = 0; i < (FPS / 2); i++)      /* pause for a while */
      INSERT_REPLY(0);
   board_revive_check(actors, NULL, NULL);
   for (i = 0; actors[i] != 0; i++)
      ;
   for (; i >= 0 && actors[i] != cmd->b.x1; i--) {
      INSERT_REPLY(STATE_MOVE_UP);
      INSERT_REPLY(0);                  /* indicate release of up key */
   }
   INSERT_REPLY(REPLY_FIRE);
   INSERT_REPLY(0);               /* indicate release of fire key */
}

/*--------------------------------------------------------------------------*/
/* iface_frame                                                              */
/*--------------------------------------------------------------------------*/

void iface_frame(void)
{
   int i;
   unsigned char r;
   int other;

   if (side->frame_func != NULL)
      side->frame_func(keys_down);
   else {
      r = side->replies[side->next_reply];
      if (r == REPLY_LAST) {
         fprintf(stderr, "iface_frame():  reply is too short.\n");
         fprintf(stderr, "last command was b=(spell=%d, x1=%d, y1=%d, x2=%d, y2=%d)\n",
                 computer_last_command.b.spell,
                 computer_last_command.b.x1, computer_last_command.b.y1,
                 computer_last_command.b.x2, computer_last_command.b.y2);
         exit(EXIT_FAILURE);
      }
      for (i = STATE_MOVE_FIRST; i <= STATE_MOVE_LAST; i++)
         keys_down[i] = (i == (r & REPLY_MOVE_MASK));
      keys_down[0] = ((r & REPLY_FIRE) == REPLY_FIRE);
      side->next_reply++;
   }

   other = !side->num;
   if (sides[other].iface == IFACE_NETWORK) {
      sides[other].turn_func(side->num, 0, NULL);
      sides[other].frame_func(keys_down);
   }
}

/*--------------------------------------------------------------------------*/
/* iface_key_down                                                           */
/*--------------------------------------------------------------------------*/

int iface_key_down(int key)
{
   return keys_down[key];
}

/*--------------------------------------------------------------------------*/
/* iface_notify_computer                                                    */
/*--------------------------------------------------------------------------*/

void iface_notify_computer(int mode)
{
   if (sides[0].iface == IFACE_COMPUTER)
      iface_turn(0, mode);
   if (sides[1].iface == IFACE_COMPUTER)
      iface_turn(1, mode);
}

/*--------------------------------------------------------------------------*/
/* iface_is_pausable                                                        */
/*--------------------------------------------------------------------------*/

int iface_is_pausable(void)
{
   return (sides[0].iface != IFACE_NETWORK &&
           sides[1].iface != IFACE_NETWORK);
}

/*--------------------------------------------------------------------------*/
/* iface_get_config                                                         */
/*--------------------------------------------------------------------------*/

IFACE_CONFIG *iface_get_config(void)
{
   return &iface_config;
}

/*--------------------------------------------------------------------------*/
/* iface_new_player                                                         */
/*--------------------------------------------------------------------------*/

IFACE_PLAYER *iface_new_player(void)
{
   IFACE_PLAYER *player;
   
   player = list_insert_after(
      &iface_config.players, NULL, sizeof(IFACE_PLAYER));
   player->human = calloc(1, sizeof(HUMAN_CONFIG));
   player->computer = calloc(1, sizeof(COMPUTER_CONFIG));
   player->network = calloc(1, sizeof(NETWORK_CONFIG));

   return player;
}

/*--------------------------------------------------------------------------*/
/* iface_delete_player                                                      */
/*--------------------------------------------------------------------------*/

void iface_delete_player(IFACE_PLAYER *player)
{
   if (!strcmp(iface_config.light_name, player->name))
      strcpy(iface_config.light_name, "light?");
   if (!strcmp(iface_config.dark_name, player->name))
      strcpy(iface_config.dark_name, "dark?");
   
   list_delete(&iface_config.players, player);
}

/*--------------------------------------------------------------------------*/
/* iface_config_read                                                        */
/*--------------------------------------------------------------------------*/

void iface_config_read(FILE *fp)
{
   int i, num;
   IFACE_PLAYER *player;

   fscanf(fp, "%32s %32s %d %d",
          iface_config.light_name, iface_config.dark_name,
          &iface_config.light_first, &num);
   list_create(&iface_config.players);
   for (i = 0; i < num; i++) {
      player = iface_new_player();
      fscanf(fp, "%32s %d", player->name, &player->type);
      human_config_read(fp, (HUMAN_CONFIG *)player->human);
      computer_config_read(fp, (COMPUTER_CONFIG *)player->computer);
      network_config_read(fp, (NETWORK_CONFIG *)player->network);

#ifdef AUTOPILOT
      if (i == 2) {
         printf("iface:  autopilot mode:  use only 2 players\n");
         exit(EXIT_FAILURE);
      }
      if (player->type != IFACE_COMPUTER) {
         printf("iface:  autopilot mode:  use only computer players\n");
         exit(EXIT_FAILURE);
      }
      if (strcmp(iface_config.light_name, iface_config.dark_name) == 0) {
         printf("iface:  autopilot mode:  use different players for light and dark\n");
         exit(EXIT_FAILURE);
      }
#endif
   }
}

/*--------------------------------------------------------------------------*/
/* iface_config_write                                                       */
/*--------------------------------------------------------------------------*/

void iface_config_write(FILE *fp)
{
   IFACE_PLAYER *player;

   fprintf(fp, "%-32s %-32s\n%d\n%d\n\n",
           iface_config.light_name, iface_config.dark_name,
           iface_config.light_first,
           list_count(&iface_config.players));

   for (player = list_head(&iface_config.players); player != NULL; player = list_next(player)) {
      fprintf(fp, "\n%-32s\n%d\n", player->name, player->type);
      human_config_write(fp, (HUMAN_CONFIG *)player->human);
      fprintf(fp, "\n");
      computer_config_write(fp, (COMPUTER_CONFIG *)player->computer);
      fprintf(fp, "\n");
      network_config_write(fp, (NETWORK_CONFIG *)player->network);
      fprintf(fp, "\n");
   }
}
