/*--------------------------------------------------------------------------*/
/* computer                                                                 */
/*--------------------------------------------------------------------------*/

#include <config.h>

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "main.h"
#include "computer.h"
#include "actors.h"
#include "board.h"
#include "field.h"
#include "list.h"

/*--------------------------------------------------------------------------*/
/* defines                                                                  */
/*--------------------------------------------------------------------------*/

#define DATA(x)         (computer_data[configs[computer_side]->data_num].x)
#define FIELD_SKILL     DATA(skill_level)
#define TIME_TOLERANCE  DATA(time_tolerance)
#define DIST_TOLERANCE  DATA(dist_tolerance)
#define ACTOR_RECHARGE  DATA(actor_recharge)
#define NUM_GOOD_MOVES  DATA(num_good_moves)

/* these field_skill values are the number of frames to skip between actions */
#define FIELD_SKILL_HARD        4
#define FIELD_SKILL_MEDIUM      9
#define FIELD_SKILL_EASY        13

/* dist_tolerance is the range between max and min distances from enemy */
#define DIST_TOLERANCE_EASY     CELL_XSIZE * 3
#define DIST_TOLERANCE_MEDIUM   CELL_XSIZE * 2
#define DIST_TOLERANCE_HARD     CELL_XSIZE

/*--------------------------------------------------------------------------*/
/* enums                                                                    */
/*--------------------------------------------------------------------------*/

enum {
  ONE_CIRCLE_DIR,
  BOTH_CIRCLE_DIRS
};

enum {
  FLEE = -1,
  STAY = 0,
  CHASE = 1
};

enum {
  SHOOT = -1,
  DODGE = 1
};

/*--------------------------------------------------------------------------*/
/* structures                                                               */
/*--------------------------------------------------------------------------*/

typedef struct {
   int light_sum;                       /* damage light caused */
   int dark_sum;                        /* damage dark caused */
   int light_hits;                      /* number of times light hit */
   int dark_hits;                       /* number of times dark hit */
   int count;
} DAMAGE;

typedef struct {
   /* field */
   int skill_level;                     /* how many frames to skip on field */
   int time_tolerance;                  /* (unused) when to dodge missile */
   int dist_tolerance;                  /* range between min & max dist */
   int actor_recharge;                  /* (unused) min enemy recharge level */
   /* board */
   int num_good_moves;                  /* good moves to consider (max 32) */
} COMPUTER_DATA;

typedef struct {
   int spell;
   int x1, y1;
   int x2, y2;
   int score;
} MOVE;

typedef struct {
   int min_unsafe_dist_2;
   int max_unsafe_dist_2;
   int min_safe_dist_2;
   int max_safe_dist_2;
   int dodge_dir;
   int last_dir;
} FIELD_STATS;

/*--------------------------------------------------------------------------*/
/* functions                                                                */
/*--------------------------------------------------------------------------*/

static void computer_board(COMMAND *cmd);
static void computer_board_teleport_exchange(int spell, MOVE *move, int me, int him);
static void computer_board_teleport(MOVE *move);
static void computer_board_heal(MOVE *move);
static void computer_board_shift_time(MOVE *move);
static void computer_board_exchange(MOVE *move);
static void computer_board_summon_elemental(MOVE *move);
static void computer_board_revive(MOVE *move);
static void computer_board_imprison(MOVE *move);
static int computer_board_spell(void);
static void computer_board_battle_score(MOVE *move);
static void computer_board_move_score(MOVE *move);
static void computer_board_insert_move(int x1, int y1, int x2, int y2);
static int computer_board_good_moves(int pass);
static void computer_field_start(void);
static void computer_field_compute_stats(FIELD_STATS *stats, FIELD_ACTOR *fa,
      FIELD_ACTOR *enemy);
static void computer_field(COMMAND *cmd);
static int computer_field_defense(COMMAND *cmd, int dist, FIELD_STATS *my_stats,
      FIELD_STATS *their_stats);
static int computer_field_offense(COMMAND *cmd, int dist, FIELD_STATS *my_stats,
      FIELD_STATS *their_stats);
static void computer_field_move(COMMAND *cmd, int dist, FIELD_STATS *my_stats,
      FIELD_STATS *their_stats);
static int computer_field_hit_obstacle(FIELD_ACTOR *target,
      FIELD_ACTOR *source, int dir, int distance);
static int computer_field_choose_dir(FIELD_ACTOR *other, 
				     int chase_or_flee,
				     int shoot_or_dodge);
static int computer_field_score_dir(int test_dir,
				    int circle_dir, int radial_dir,
				    int which_circle_dirs);
static int computer_field_miss_dir(FIELD_ACTOR *target,
				   FIELD_ACTOR *source,
				   int shoot_dir);
static int computer_field_miss_dist(FIELD_ACTOR *target,
				    FIELD_ACTOR *source,
				    int miss_dir);
static int computer_field_uncorrected_dist(FIELD_ACTOR *towards);
static int computer_field_direction(FIELD_ACTOR *towards);
static void computer_field_stop(void);

extern void Xarchon_AI_Computer(int *x1, int *y1, int *x2, int *y2);

/*--------------------------------------------------------------------------*/
/* variables                                                                */
/*--------------------------------------------------------------------------*/

static MOVE good_moves[33];

static int computer_side;
static COMPUTER_CONFIG *configs[2];

static void (*computer_board_spell_funcs[SPELL_COUNT_2])(MOVE *move) = {
   NULL,
   computer_board_teleport,
   computer_board_heal,
   computer_board_shift_time,
   computer_board_exchange,
   computer_board_summon_elemental,
   computer_board_revive,
   computer_board_imprison,
   NULL,                                /* cease conjuring */
   NULL
};

static COMPUTER_DATA computer_data[3] = {
   { FIELD_SKILL_EASY, 2, DIST_TOLERANCE_EASY, FPS * 0.5, 6 },
   { FIELD_SKILL_MEDIUM, 2, DIST_TOLERANCE_MEDIUM, FPS * 0.5, 4 },
   { FIELD_SKILL_HARD, 5, DIST_TOLERANCE_HARD, FPS * 0.3, 1 }
};
   
static DAMAGE computer_damage[ACTOR_NUM_LIGHT + 1][ACTOR_NUM_DARK + 1];
static int computer_damage_ok = 0;

static FIELD_STATS light_stats, dark_stats;

/*--------------------------------------------------------------------------*/
/* computer_start                                                           */
/*--------------------------------------------------------------------------*/

void computer_start(int side, COMPUTER_CONFIG *config)
{
   configs[side] = config;
}

/*--------------------------------------------------------------------------*/
/* computer_turn                                                            */
/*--------------------------------------------------------------------------*/

void computer_turn(int side, int mode, COMMAND *cmd)
{
   computer_side = side;
   switch (mode) {
      case IFACE_BOARD:
         computer_board(cmd);
         break;
      case IFACE_FIELD_START:
         computer_field_start();
         break;
      case IFACE_FIELD:
         computer_field(cmd);
         break;
      case IFACE_FIELD_STOP:
         computer_field_stop();
         break;
   }
}

/*--------------------------------------------------------------------------*/
/* computer_board                                                           */
/*--------------------------------------------------------------------------*/

void computer_board(COMMAND *cmd)
{
   int n;
   MOVE *move;

   if (computer_board_spell())
      move = &good_moves[0];
   else {
      if (configs[computer_side]->old_board_mode) {
         n = computer_board_good_moves(1);
         move = &good_moves[random() % n];
      } else {
         move = &good_moves[0];
         Xarchon_AI_Computer(&move->x1, &move->y1, &move->x2, &move->y2);
      }
      move->spell = 0;
   }
   cmd->b.spell = move->spell;
   cmd->b.x1 = move->x1;
   cmd->b.y1 = move->y1;
   cmd->b.x2 = move->x2;
   cmd->b.y2 = move->y2;
}

/*--------------------------------------------------------------------------*/
/* computer_board_teleport_exchange                                         */
/*--------------------------------------------------------------------------*/

void computer_board_teleport_exchange(int spell, MOVE *move, int me, int him)
{
   int x1, y1;
   int x2, y2;
   int flags;

   board_find_actor(me, &x1, &y1);
   flags = CELL_POWER | CELL_IMPRISON;
   if (x1 != -1 && (board_cells[y1][x1].flags & flags) == 0) {
      board_find_actor(him, &x2, &y2);
      if (spell == SPELL_TELEPORT)      /* it is possible to teleport    */
         flags ^= CELL_IMPRISON;        /*   *into* an imprisoned square */
      if (x2 != -1 && (board_cells[y2][x2].flags & flags) == 0) {
         move->spell = spell;
         move->x1 = x1;
         move->y1 = y1;
         move->x2 = x2;
         move->y2 = y2;
      }
   }
}

/*--------------------------------------------------------------------------*/
/* computer_board_teleport                                                  */
/*--------------------------------------------------------------------------*/

void computer_board_teleport(MOVE *move)
{
   int me, him;
   int x, y;
   int lumi;

   if (random() % 5 == 0) {
      me = (computer_side == 0) ? ACTOR_UNICORN : ACTOR_BASILISK;
      him = (computer_side == 0) ? ACTOR_BASILISK : ACTOR_UNICORN;
      board_find_actor(him, &x, &y);
      if (x != -1) {
         lumi = board_cell_lumi(&board_cells[y][x]);
         if ((computer_side == 0 && (lumi & CELL_LIGHT)) ||
             (computer_side == 1 && (lumi & CELL_DARK)))
            computer_board_teleport_exchange(SPELL_TELEPORT, move, me, him);
      }
   }
}

/*--------------------------------------------------------------------------*/
/* computer_board_heal                                                      */
/*--------------------------------------------------------------------------*/

void computer_board_heal(MOVE *move)
{
   int x, y;
   int type;

   type = (computer_side == 0) ? ACTOR_UNICORN : ACTOR_BASILISK;
   board_find_actor(type, &x, &y);
   if (x != -1 && (board_cells[y][x].flags & CELL_POWER) == 0 &&
       board_cells[y][x].actor->strength <= actors_list[type].strength / 2) {
      move->spell = SPELL_HEAL;
      move->x1 = x;
      move->y1 = y;
   }
}

/*--------------------------------------------------------------------------*/
/* computer_board_shift_time                                                */
/*--------------------------------------------------------------------------*/

void computer_board_shift_time(MOVE *move)
{
   int x, y;

   if (!board_is_imprison_ok()) {
      for (y = 0; y < board_get_cell_height(); y++)
         for (x = 0; x < board_get_cell_width(); x++)
            if (board_cells[y][x].flags & CELL_IMPRISON) {
               move->spell = SPELL_SHIFT_TIME;
               return;
            }
   }

   if (!spell_avails[!computer_side][SPELL_SHIFT_TIME]) {
      move->spell = SPELL_SHIFT_TIME;
      return;
   }
}

/*--------------------------------------------------------------------------*/
/* computer_board_exchange                                                  */
/*--------------------------------------------------------------------------*/

void computer_board_exchange(MOVE *move)
{
   int me, him;

   if (board_turn % 40 == random() % 40) {
      me = (computer_side == 0) ? ACTOR_DJINNI : ACTOR_DRAGON;
      him = (computer_side == 0) ? ACTOR_GOBLIN : ACTOR_KNIGHT;
      computer_board_teleport_exchange(SPELL_EXCHANGE, move, me, him);
   }
}

/*--------------------------------------------------------------------------*/
/* computer_board_summon_elemental                                          */
/*--------------------------------------------------------------------------*/

void computer_board_summon_elemental(MOVE *move)
{
   int x, y;
   int type;

   if (board_turn % 20 == random() % 20) {
      type = (computer_side == 0) ? ACTOR_SHAPESHIFTER : ACTOR_PHOENIX;
      board_find_actor(type, &x, &y);
      if (x != -1 && (board_cells[y][x].flags & CELL_POWER) == 0) {
         move->spell = SPELL_SUMMON_ELEMENTAL;
         move->x1 = x;
         move->y1 = y;
      }
   }
}

/*--------------------------------------------------------------------------*/
/* computer_board_revive                                                    */
/*--------------------------------------------------------------------------*/

void computer_board_revive(MOVE *move)
{
   int actors[10], i;

   if (random() % 2 == 0)
      if (board_revive_check(actors, NULL, NULL))
         for (i = 0; actors[i] != 0; i++)
            if (actors[i] == ACTOR_PHOENIX ||
                actors[i] == ACTOR_DJINNI ||
                actors[i] == ACTOR_UNICORN ||
                actors[i] == ACTOR_SHAPESHIFTER ||
                actors[i] == ACTOR_DRAGON ||
                actors[i] == ACTOR_BASILISK) {
               move->spell = SPELL_REVIVE;
               move->x1 = actors[i];
               return;
            }
}

/*--------------------------------------------------------------------------*/
/* computer_board_imprison                                                  */
/*--------------------------------------------------------------------------*/

void computer_board_imprison(MOVE *move)
{
   int type;
   int target_x, target_y;
   int count;
   int x, y;

   if (board_is_imprison_ok()) {
      type = (computer_side == 0) ? ACTOR_SORCERESS : ACTOR_WIZARD;
      board_find_actor(type, &target_x, &target_y);
      if (target_x == -1) {
         count = 0;
         for (y = 0; y < board_get_cell_height(); y++)
            for (x = 0; x < board_get_cell_width(); x++)
               if (board_cells[y][x].actor != NULL &&
                   actor_is_side(board_cells[y][x].actor, !computer_side)) {
                  target_x = x;
                  target_y = y;
                  count++;
               }
         if (count != 1)
            target_x = -1;
      }
      if (target_x != -1 &&
          (board_cells[target_y][target_x].flags & CELL_POWER) == 0) {
         move->spell = SPELL_IMPRISON;
         move->x1 = target_x;
         move->y1 = target_y;
      }
   }
}

/*--------------------------------------------------------------------------*/
/* computer_board_spell                                                     */
/*--------------------------------------------------------------------------*/

int computer_board_spell(void)
{
   int x, y;
   int type;
   MOVE move;
   int i = 0, i0;

   /* spells are only available if we have non-imprisoned master */
   type = (computer_side == 0) ? ACTOR_WIZARD : ACTOR_SORCERESS;
   if (!board_find_actor(type, &x, &y))
      return 0;
   if (board_cells[y][x].flags & CELL_IMPRISON)
      return 0;

   /* select a random spell that is worth casting */
   move.spell = 0;
   i0 = random() % (SPELL_COUNT - 2) + 1;  /* returns 1..7 */
   for (i = i0 + 1; i != i0; i++) {
      if (i < SPELL_FIRST)              /* no spell number 0 */
         continue;
      if (i >= SPELL_LAST) {
         i = SPELL_FIRST - 1;
         continue;
      }
      if (spell_avails[computer_side][i]) {
         computer_board_spell_funcs[i](&move);
         if (move.spell != 0) {
            good_moves[0].spell = i;
            good_moves[0].x1 = move.x1;
            good_moves[0].y1 = move.y1;
            good_moves[0].x2 = move.x2;
            good_moves[0].y2 = move.y2;
            return 1;
         }
      }
   }
   return 0;
}

/*--------------------------------------------------------------------------*/
/* computer_board_battle_score                                              */
/*--------------------------------------------------------------------------*/

void computer_board_battle_score(MOVE *move)
{
   ACTOR *me, *them;
   CELL *there;
   int health;
   int attacker_damage, defender_damage;
   int attacker_hits, defender_hits;

   me = board_cells[move->y1][move->x1].actor;
   there = &board_cells[move->y2][move->x2];
   them = there->actor;
   health = field_initial_life(me, there, them);
   computer_field_score(me, them, &attacker_damage, &defender_damage,
                        &attacker_hits, &defender_hits);
   if (health <= defender_damage)       /* our guy is going to die */
      move->score = 0;
   else {
      health = field_initial_life(them, there, me);
      move->score = attacker_damage * 500 / health;
   }
}

/*--------------------------------------------------------------------------*/
/* computer_board_move_score                                                */
/*--------------------------------------------------------------------------*/

void computer_board_move_score(MOVE *move)
{
   int x, y;
   int n_actors, n_power_points;

   /* score move regardless of power points */
   if (board_cells[move->y2][move->x2].actor == NULL)
      move->score = random() % 500;
   else
      computer_board_battle_score(move);

   /* if moving from a regular square into a power point, give bonus */
   if ((board_cells[move->y1][move->x1].flags & CELL_POWER) == 0 &&
       (board_cells[move->y2][move->x2].flags & CELL_POWER) != 0) {
      n_actors = 0;
      n_power_points = 1;
      for (y = 0; y < board_get_cell_height(); y++)
         for (x = 0; x < board_get_cell_width(); x++)
            if (board_cells[y][x].actor != NULL &&
                actor_is_side(board_cells[y][x].actor, computer_side)) {
               n_actors++;
               if (board_cells[y][x].flags & CELL_POWER)
                  n_power_points++;
            }
      if (n_actors >= 5)
         move->score += n_power_points * 500;
   }

   /* if moving from a power point to a regular square, reduce score */
   /* or:  if moving the master */
   if ((board_cells[move->y1][move->x1].flags & CELL_POWER) != 0 ||
        (board_cells[move->y1][move->x1].actor->type & ACTOR_MASTER) == ACTOR_MASTER)
      move->score /= 10;
}

/*--------------------------------------------------------------------------*/
/* computer_board_insert_move                                               */
/*--------------------------------------------------------------------------*/

void computer_board_insert_move(int x1, int y1, int x2, int y2)
{
   MOVE move;
   int i, min_i;

   move.x1 = x1;
   move.y1 = y1;
   move.x2 = x2;
   move.y2 = y2;
   computer_board_move_score(&move);

   for (i = 0; i < NUM_GOOD_MOVES; i++)
      if (good_moves[i].x1 == -1)
         break;
   if (i >= NUM_GOOD_MOVES) {
      min_i = 0;
      for (i = 1; i < NUM_GOOD_MOVES; i++)
         if (good_moves[i].score < good_moves[min_i].score)
            min_i = i;
      if (good_moves[min_i].score < move.score)
         i = min_i;
   }
   if (i < NUM_GOOD_MOVES)
      memcpy(&good_moves[i], &move, sizeof(MOVE));
}

/*--------------------------------------------------------------------------*/
/* computer_board_good_moves                                                */
/*--------------------------------------------------------------------------*/

int computer_board_good_moves(int pass)
{
   int i;
   int x1, y1;
   int x2, y2;
   ACTOR *actor;

   for (i = 0; i < NUM_GOOD_MOVES + 1; i++)
      good_moves[i].x1 = -1;

   for (y1 = 0; y1 < board_get_cell_height(); y1++)
      for (x1 = 0; x1 < board_get_cell_width(); x1++) {
         if (!board_is_pickable(x1, y1, 0))
            continue;

         /* filtering (pass 1 only):  pick only ground actors on */
         /* even-numbered turns;  only fly ones on odd-numbered turns */
         actor = board_cells[y1][x1].actor;
         /*
         if ((actor->type & ACTOR_MASK) != ACTOR_MANTICORE)
            continue;
         */
         if (pass == 1 &&
             (((board_turn / 2) % 2 != 0 && (actor->type & ACTOR_FLY) == 0) ||
              ((board_turn / 2) % 2 == 0 && (actor->type & ACTOR_GROUND) == 0)))
            continue;
         /* filtering (pass 1,2 only):  don't pick the master */
         if (pass == 2 && (actor->type & ACTOR_MASTER) == 0)
            continue;

         for (y2 = y1 - actor->distance; y2 <= y1 + actor->distance; y2++)
            for (x2 = x1 - actor->distance; x2 <= x1 + actor->distance; x2++)
               if (y2 >= 0 && y2 < board_get_cell_height() &&
                   x2 >= 0 && x2 < board_get_cell_width() &&
                   (x2 != x1 || y2 != y1) &&
                   (board_cells[y2][x2].actor == NULL ||
                    actor_is_side(board_cells[y2][x2].actor, !computer_side)) &&
                   board_get_route(x1, y1, x2, y2) != NULL)
                  computer_board_insert_move(x1, y1, x2, y2);
      }

   for (i = 0; i < NUM_GOOD_MOVES && good_moves[i].x1 != -1; i++)
      ;
   if (i == 0 && pass < 3)              /* retry (the higher the pass num, */
      i = computer_board_good_moves(pass + 1); /* the less filters are done) */
   if (i == 0) {
      fprintf(stderr, "computer_board_good_moves():  cannot find any possible move to make\n");
      exit(EXIT_FAILURE);
   }
   return i;
}

/*--------------------------------------------------------------------------*/
/* computer_field_start                                                     */
/*--------------------------------------------------------------------------*/

void computer_field_start()
{
   /* unlike other computer...() functions, this one isn't side-dependant: */
   /* whenever it is invoked, field_me is light, and field_he is dark. */
   computer_field_compute_stats(&light_stats, field_me, field_he);
   computer_field_compute_stats(&dark_stats, field_he, field_me);
}

/*--------------------------------------------------------------------------*/
/* computer_field_compute_stats                                             */
/*--------------------------------------------------------------------------*/

void computer_field_compute_stats(FIELD_STATS *stats, FIELD_ACTOR *fa,
      FIELD_ACTOR *enemy)
{
  int safe_distance, max_safe_distance;
  int max_unsafe_distance, min_unsafe_distance;

  /* compute the minimum distance fa can be and still dodge a dead-on */
  /* shot from enemy */
  if ((enemy->weapon->actor->type & ACTOR_WEAPON_HAND) == ACTOR_WEAPON_HAND) {
    safe_distance = 3*CELL_XSIZE/2 + enemy->actor->speed;
    max_unsafe_distance = safe_distance;
    min_unsafe_distance = CELL_XSIZE/2;
  }
  else if ((enemy->weapon->actor->type & ACTOR_WEAPON_CLOUD) ==
	   ACTOR_WEAPON_CLOUD) {
    if ((enemy->weapon->actor->type & ACTOR_FREEZE_USER) == 
	ACTOR_FREEZE_USER) {
      safe_distance = 7*CELL_XSIZE/2 + enemy->actor->speed;
      max_unsafe_distance = 3*CELL_XSIZE/2;
      min_unsafe_distance = 0;
    }
    else {
      safe_distance = 7*CELL_XSIZE/2 + 2*enemy->actor->speed;
      max_unsafe_distance = 3*CELL_XSIZE/2;
      min_unsafe_distance = CELL_XSIZE/2;
    }
  } else {
    safe_distance = enemy->weapon->actor->speed *
      (1 + 2*CELL_XSIZE / (3*fa->actor->speed)) + CELL_XSIZE;
    max_unsafe_distance = safe_distance;
    min_unsafe_distance = safe_distance - CELL_XSIZE;
  }

  max_safe_distance = safe_distance + CELL_XSIZE;

  /* store the squares of the distances */
  stats->min_safe_dist_2 = safe_distance*safe_distance;
  stats->max_safe_dist_2 = max_safe_distance*max_safe_distance;
  stats->max_unsafe_dist_2 = max_unsafe_distance*max_unsafe_distance;
  stats->min_unsafe_dist_2 = min_unsafe_distance*min_unsafe_distance;
  stats->dodge_dir = 0;
  stats->last_dir = 0;
}

/*--------------------------------------------------------------------------*/
/* computer_field                                                           */
/*--------------------------------------------------------------------------*/

void computer_field(COMMAND *cmd)
{
   FIELD_STATS *my_stats, *their_stats;
   int dist;

   /* need to keep stats for light/dark and decide each time we're called */
   /* which are my_stats and which are their_stats because we could have */
   /* computer vs. computer */
   if ((field_me->actor->type & ACTOR_LIGHT) == ACTOR_LIGHT) {
      my_stats = &light_stats;
      their_stats = &dark_stats;
   } else {
      my_stats = &dark_stats;
      their_stats = &light_stats;
   }

   if (random() % (FIELD_SKILL + 1) != 0) {
     cmd->f.dir = my_stats->last_dir;
     cmd->f.fire = 0;
     return;
   }

   dist = (field_me->x - field_he->x)*(field_me->x - field_he->x) +
         (field_me->y - field_he->y)*(field_me->y - field_he->y);
   if (!computer_field_defense(cmd, dist, my_stats, their_stats))
      if (!computer_field_offense(cmd, dist, my_stats, their_stats))
         computer_field_move(cmd, dist, my_stats, their_stats);

   my_stats->last_dir = cmd->f.dir;
}

/*--------------------------------------------------------------------------*/
/* computer_field_defense                                                   */
/*--------------------------------------------------------------------------*/

int computer_field_defense(COMMAND *cmd, int dist, FIELD_STATS *my_stats,
      FIELD_STATS *their_stats)
{
   int dir, dodge_dist = 1;
   int calc_speed = 1;
   int time_hit_2, time_dodge_2;
   int hopeless = 0;

   /* test if we should dodge an incoming shot */
   if (field_he->weapon->state == 0) {
      my_stats->dodge_dir = 0;
      return 0;
   }
   /* what sort of attack are we avoiding? */
   switch (field_he->weapon->actor->type &
         (ACTOR_WEAPON_HAND | ACTOR_WEAPON_CLOUD | ACTOR_WEAPON_SHOOT)) {
      case ACTOR_WEAPON_HAND:
         dir = state_opposite[computer_field_direction(field_he)];
         if (dir != field_he->weapon->state)
            return 0;
         else
            calc_speed = 0;
         /* fall through to the SHOOT dodge code, but don't do speed calcs */

	 /* TODO: write fresh hand code. It should be like this:
	  * if we've already been hit by the hand weapon, it can't hurt us
	  * any more, so ignore it.
	  * if we haven't been hit, all we have to do is not run into it.
	  * maybe try to attack from a different side. */
      case ACTOR_WEAPON_SHOOT:
         /* If there's a shot in the air, dodge it! */
         if (my_stats->dodge_dir) {
            if (my_stats->dodge_dir < STATE_MOVE_FIRST)
               /* we're no longer dodging the shot- either we're already out */
               /* of the way, or we can't make it, so we're not even trying */
               return 0;
            dir = my_stats->dodge_dir;
         } else {
            /* first, test if the shot will reach us. */
            if (computer_field_hit_obstacle(field_me, field_he->weapon,
					    field_he->weapon->state, 0)) {
	       return 0;
	    }
	    /* the shot will hit us. find out which way to dodge */
	    dir = computer_field_choose_dir(field_he->weapon, STAY, DODGE);

	    if (calc_speed) {
	      dodge_dist = CELL_XSIZE - 
		computer_field_miss_dist(field_me, field_he->weapon, dir);
	      time_hit_2 = dist/(field_he->weapon->actor->speed*
			       field_he->weapon->actor->speed);
	      time_dodge_2 = dodge_dist*dodge_dist/(field_me->actor->speed*
						  field_me->actor->speed);
	      if (time_hit_2 < time_dodge_2) {
		hopeless = 1;
	      }
	      /* if there's a lot of time to spare, return 0
	       * in order to get off a shot or something... */
	      if (time_hit_2 > time_dodge_2 * 2)
		return 0;
	    }
         }
         break;
      case ACTOR_WEAPON_CLOUD:
         if (dist < my_stats->min_safe_dist_2) {
            /* run away! */
            dir = computer_field_choose_dir( field_he, FLEE, STAY);
         } else
            return 0;
         break;
      default:
         /* ??? */
         return 0;
   }
   if (hopeless) {
      my_stats->dodge_dir = -1;
      if ((field_me->weapon->actor->type & ACTOR_IMMUNE_USER) ==
            ACTOR_IMMUNE_USER) {
         /* if we're going to be hit, try to hit the fire button */
         cmd->f.fire = 1;
         /* may as well attack towards them */
         cmd->f.dir = computer_field_direction(field_he);
         return 1;
      }
   } else {
      cmd->f.fire = 0;
      cmd->f.dir = dir;
      my_stats->dodge_dir = dir;
      return 1;
   }
   return 0;
}

/*--------------------------------------------------------------------------*/
/* computer_field_offense                                                   */
/*--------------------------------------------------------------------------*/

int computer_field_offense(COMMAND *cmd, int dist, FIELD_STATS *my_stats,
      FIELD_STATS *their_stats)
{
   int dir_to_them;

   /* dark cloud attacker: follow enemy when cloud is active */
   if ((field_me->weapon->actor->type & ACTOR_MASK) == ACTOR_DARK_CLOUD &&
       field_me->weapon->state != 0) {
     /* unless enemy is firing a light cloud */
     if ((field_he->weapon->actor->type & ACTOR_MASK) != ACTOR_DARK_CLOUD ||
	 field_he->weapon->state == 0) {
       cmd->f.fire = 0;
       cmd->f.dir = computer_field_choose_dir(field_he, CHASE, DODGE);
       return 1;
     }
   }

   if (!WAS_TIME(field_me->weapon, field_me->weapon->actor->recharge)) {
     return 0;
   }

   if (dist > their_stats->max_unsafe_dist_2) {
     return 0;
   }
   /* TODO if they don't have perfect reflexes, we could try to shoot at
    * them anyway... */

   dir_to_them = computer_field_direction(field_he);
   switch (field_me->weapon->actor->type &
         (ACTOR_WEAPON_HAND | ACTOR_WEAPON_CLOUD | ACTOR_WEAPON_SHOOT)) {
      case ACTOR_WEAPON_HAND:
      case ACTOR_WEAPON_SHOOT:
         if (!computer_field_hit_obstacle(field_he, field_me,
					  dir_to_them, 0)) {
            /* we have a direct unobstructed line of fire */
            cmd->f.fire = 1;
            cmd->f.dir = dir_to_them;
            return 1;
         }
         break;
      case ACTOR_WEAPON_CLOUD:
         cmd->f.fire = 1;
         cmd->f.dir = dir_to_them;
         return 1;
   }

   return 0;
}

/*--------------------------------------------------------------------------*/
/* computer_field_move                                                      */
/*--------------------------------------------------------------------------*/

void computer_field_move(COMMAND *cmd, int dist_2, FIELD_STATS *my_stats,
      FIELD_STATS *their_stats)
{
   /* it's this fn's responsibility to set up our position so */
   /* computer_field_defense and computer_field_offense can */
   /* jump in opportunistically */

  int chase_or_flee = STAY;
  int shoot_or_dodge = DODGE;
  int min_dist_2, max_dist_2;

  if (WAS_TIME(field_me->weapon, field_me->weapon->actor->recharge)) {
    /* my weapon's charged. Get in firing range */
    min_dist_2 = their_stats->min_unsafe_dist_2;
    max_dist_2 = their_stats->max_unsafe_dist_2;
    if (!WAS_TIME(field_he->weapon, field_he->weapon->actor->recharge)) {
      /* enemy is not charged. seek a clear line of fire */
      shoot_or_dodge = SHOOT;
    }
  } else {
    /* my weapon is not charged. Keep a safe distance away. */
    min_dist_2 = my_stats->min_safe_dist_2;
    max_dist_2 = my_stats->max_safe_dist_2;
  }

  if (dist_2 <= min_dist_2)
    chase_or_flee = FLEE;
  else if (dist_2 > max_dist_2)
    chase_or_flee = CHASE;

   cmd->f.fire = 0;
   cmd->f.dir = computer_field_choose_dir(field_he, 
					  chase_or_flee, 
					  shoot_or_dodge);

   /* TODO if our safe distance is inside theirs, try to feint, to tempt
    * them into firing fruitlessly */
   /* TODO avoid running in to shots/melee weapons/clouds we've already
    * dodged */
}

/*--------------------------------------------------------------------------*/
/* computer_field_hit_obstacle                                              */
/*--------------------------------------------------------------------------*/

int computer_field_hit_obstacle(FIELD_ACTOR *target, FIELD_ACTOR *source,
      int dir, int distance)
{
   /* return 1 if the line from source along dir hits a rock or */
   /* the edge of the screen. */
   /* return 0 if the line hits the target or reaches distance. */
   /* if distance is 0, don't count distance. */
   /* if target is NULL, don't look for a target. */

   int field_x, field_y;
   int cell_x, cell_y;
   int x_offset, y_offset;
   int dx, dy;
   unsigned char rock;
   int travelled = 0;

   if (target && distance == 0) {
     if ( CELL_XSIZE < computer_field_miss_dist(target, source,
			  computer_field_miss_dir(target, source, dir)) ) {
       /* the "shot" will miss the target. No point checking further */
       return 1;
     }
   }

   field_x = source->x;
   field_y = source->y;
   x_offset = CELL_XOFFSET(field_x);
   y_offset = CELL_YOFFSET(field_y);
   cell_x = field_x/CELL_XSIZE;
   cell_y = field_y/CELL_YSIZE;
   dx = CELL_X(state_move_x_step[dir]);
   dy = CELL_Y(state_move_y_step[dir]);
   while (field_x >= 0 && field_x <= CELL_X(FIELD_XCELLS - 1) &&
	  field_y >= 0 && field_y <= CELL_Y(FIELD_YCELLS - 1)) {

     /* I changed the order of these tests, so the computer won't */
     /* check the source's position for collisions. It's sort of */
     /* a hack, since the field range checking is screwed up now. */
     /* If it gets to be a problem, this whole function could use */
     /* a rewrite. - mhc */

     if (distance == 0 || travelled < distance - CELL_XSIZE) {
       cell_x += state_move_x_step[dir];
       cell_y += state_move_y_step[dir];
       field_x += dx;
       field_y += dy;
       travelled += CELL_XSIZE;
     } else {
       if (travelled == distance) {
	 return 0;
       }
       field_x += state_move_x_step[dir]*(distance - travelled);
       field_y += state_move_y_step[dir]*(distance - travelled);
       x_offset = CELL_XOFFSET(field_x);
       y_offset = CELL_YOFFSET(field_y);
       cell_x = field_x/CELL_XSIZE;
       cell_y = field_y/CELL_YSIZE;
       travelled = distance;
     }

     if (target && field_collision(field_x, field_y, target->x, target->y))
       return 0;

     if (x_offset < 3*CELL_XSIZE/4 && y_offset < 3*CELL_YSIZE/4) {
       rock = field_cells[cell_y][cell_x];
       if ((rock >> ROCK_IX_SHIFT) != ROCK_NONE &&
	   (rock & ROCK_LUMI_MASK) < ROCK_WALKABLE) {
	 return 1;
       }
     }
     if (x_offset > CELL_XSIZE/4 && cell_x < FIELD_XCELLS - 1) {
       rock = field_cells[cell_y][cell_x + 1];
       if ((rock >> ROCK_IX_SHIFT) != ROCK_NONE &&
	   (rock & ROCK_LUMI_MASK) < ROCK_WALKABLE) {
	 return 1;
       }
     }
     if (y_offset > CELL_YSIZE/4 && cell_y < FIELD_YCELLS - 1) {
        rock = field_cells[cell_y + 1][cell_x];
        if ((rock >> ROCK_IX_SHIFT) != ROCK_NONE &&
	   (rock & ROCK_LUMI_MASK) < ROCK_WALKABLE) {
	  return 1;
	}
	if (x_offset > CELL_XSIZE/4 && cell_x < FIELD_XCELLS - 1) {
	  rock = field_cells[cell_y + 1][cell_x + 1];
	  if ((rock >> ROCK_IX_SHIFT) != ROCK_NONE &&
	      (rock & ROCK_LUMI_MASK) < ROCK_WALKABLE) {
	    return 1;
	  }
	}
     }
   }
   return 1;
}

/*--------------------------------------------------------------------------*/
/* computer_field_choose_dir                                                */
/*--------------------------------------------------------------------------*/

int computer_field_choose_dir(FIELD_ACTOR *other, 
  int chase_or_flee,
  int shoot_or_dodge)
{
  /* this fn returns the best available direction to move in. */
  /* The direction is determined relative to other- */
  /* chase_or_flee tells whether to go towards or away from other. */
  /* shoot_or_dodge tells what direction to circle other: */
  /* SHOOT seeks a clear line of fire, DODGE avoids it. */

  int test_dir, test_score; 
  int best_dir = 0;
  int best_score = 0;
  int towards_dir, around_dir;
  int which_circle_dirs = BOTH_CIRCLE_DIRS;

  towards_dir = computer_field_direction(other);
  around_dir = computer_field_miss_dir(field_me, other, 
				       state_opposite[towards_dir]);

  if (shoot_or_dodge == DODGE) {
    if (!computer_field_hit_obstacle(field_me, other, 
				     state_opposite[towards_dir],
				     computer_field_uncorrected_dist(other)))
      /* when dodging, don't run into the line of fire from a safe place */
      /* warning: this test assumes that other is shooting towards us, */
      /* even if other is a weapon going in some random direction */
      which_circle_dirs = ONE_CIRCLE_DIR;
    else if (CELL_X(2) < computer_field_miss_dist(field_me, other, around_dir))
      /* if we're well out of the way, quit circling */
      around_dir = 0;
  }

  if (shoot_or_dodge == SHOOT) {
    if (computer_field_hit_obstacle(other, field_me, towards_dir, 0))
      /* can't shoot from here. Get in line of fire. */
      around_dir = state_opposite[around_dir];
    else
      /* we're in the line of fire. stay there */
      around_dir = 0;
  }

  if (shoot_or_dodge == STAY)
    around_dir = 0;

  if (chase_or_flee == FLEE)
    towards_dir = state_opposite[towards_dir];

  if (chase_or_flee == STAY)
    towards_dir = 0;

  if (!around_dir && !towards_dir)
    return 0;
  
  /* try all directions: pick the highest scoring unobstructed move */
  for (test_dir = STATE_MOVE_FIRST; test_dir <= STATE_MOVE_LAST; test_dir++) {
    test_score = computer_field_score_dir(test_dir, around_dir, towards_dir, 
					  which_circle_dirs);
    if ( test_score >= best_score ) {
      if(!computer_field_hit_obstacle(NULL, field_me, 
				      test_dir, CELL_XSIZE)) {
	best_score = test_score;
	best_dir = test_dir;
      }
    }
  }

  return best_dir;
}

/*--------------------------------------------------------------------------*/
/* computer_field_score_dir                                                 */
/*--------------------------------------------------------------------------*/

int computer_field_score_dir(int test_dir, 
			     int circle_dir, 
			     int radial_dir,
			     int which_circle_dirs)
{
  int test_score = 0;
  
  if (radial_dir) {
    if ( test_dir == state_opposite[radial_dir] ||
	state_is_adjacent(test_dir, state_opposite[radial_dir]) ) {
      /* going in the wrong direction */
      if (which_circle_dirs == BOTH_CIRCLE_DIRS)
	/* but if there's only one circle direction, we'll be flexible */
	return 0;
    } else if ( test_dir == radial_dir ) {
      test_score += 20;
    } else if ( state_is_adjacent(test_dir, radial_dir) ) {
      test_score += 14;
    } else if ( state_is_perpendicular(test_dir, radial_dir) ) {
      test_score += 1;
    }
  }

  if (circle_dir) {
    if ( test_dir == state_opposite[circle_dir] ) {
      if (which_circle_dirs == ONE_CIRCLE_DIR)
	return 0;
      else
	test_score += 10;
    } else if ( state_is_adjacent(test_dir, state_opposite[circle_dir]) ) {
      if (which_circle_dirs == ONE_CIRCLE_DIR)
	return 0;
      else
	test_score += 8;
    } else if ( test_dir == circle_dir ) {
      test_score += 18;
    } else if ( state_is_adjacent(test_dir, circle_dir) ) {
      test_score += 12;
    }
  }

  return test_score;
}

/*--------------------------------------------------------------------------*/
/* computer_field_miss_dir                                                  */
/*--------------------------------------------------------------------------*/

int computer_field_miss_dir(FIELD_ACTOR *target,
			    FIELD_ACTOR *source,
			    int shoot_dir)
{
  /* return the direction perpendicular to a shot */

  int dir;

  dir = state_perpendicular[shoot_dir];
  if ( computer_field_miss_dist(target, source, dir) < 0 )
    dir = state_opposite[dir];
  return dir;

}

/*--------------------------------------------------------------------------*/
/* computer_field_miss_dist                                                 */
/*--------------------------------------------------------------------------*/

int computer_field_miss_dist(FIELD_ACTOR *target,
			     FIELD_ACTOR *source,
			     int miss_dir)
{
  /* return the distance source will miss by (in miss_dir) if */
  /* it shoots directly at target */

  int dist;

  dist = state_move_x_step[miss_dir] * target->x - source->x +
	   state_move_y_step[miss_dir] * target->y - source->y;

  if ( state_move_x_step[miss_dir] && state_move_y_step[miss_dir] )
    /* miss_dir was not a unit vector! Its length was sqrt(2) */
    dist = 7 * dist / 10;

  return dist;
}

/*--------------------------------------------------------------------------*/
/* computer_field_uncorrected_dist                                          */
/*--------------------------------------------------------------------------*/

int computer_field_uncorrected_dist(FIELD_ACTOR *towards)
{
  /* returns a distance that can be passed to computer_field_hit_obstacle */
  /* the distance is always measured from field_me to towards */

  int dx, dy;

  dx = field_me->x - towards->x;
  dy = field_me->y - towards->y;

  if (dx < 0)
    dx = -dx;

  if (dy < 0)
    dy = -dy;

  if (dx > dy)
    return dx;
  else
    return dy;

}

/*--------------------------------------------------------------------------*/
/* computer_field_direction                                                 */
/*--------------------------------------------------------------------------*/

int computer_field_direction(FIELD_ACTOR *towards)
{
   int dx, dy;
   int grad;

   dx = towards->x - field_me->x;
   dy = towards->y - field_me->y;
   if (dx == 0) {
      if (dy > 0)
         return STATE_MOVE_DOWN;
      else if (dy < 0)
         return STATE_MOVE_UP;
      else
         return 0;
   } else {
      grad = 2 * dy / dx;
      if (grad <= -4 || 4 <= grad) {
         if (dy > 0)
            return STATE_MOVE_DOWN;
         else
            return STATE_MOVE_UP;
      } else if (grad >= 1) {
         if (dx > 0)
            return STATE_MOVE_DOWN_RIGHT;
         else
            return STATE_MOVE_UP_LEFT;
      } else if (grad >= -1) {
         if (dx > 0)
            return STATE_MOVE_RIGHT;
         else
            return STATE_MOVE_LEFT;
      } else {
         if (dx > 0)
            return STATE_MOVE_UP_RIGHT;
         else
            return STATE_MOVE_DOWN_LEFT;
      }
   }
}

/*--------------------------------------------------------------------------*/
/* computer_field_stop                                                      */
/*--------------------------------------------------------------------------*/

void computer_field_stop(void)
{
   int light, dark;
   DAMAGE *damage;

   /* unlike other computer...() functions, this one isn't side-dependant: */
   /* whenever it is invoked, field_me is light, and field_he is dark. */

   light = field_me->orig_actor->type & ACTOR_MASK;
   if (light >= ACTOR_AIR_ELEM && light <= ACTOR_WATER_ELEM)
      light = ACTOR_NUM_LIGHT;          /* use last entry in stats array */
   else
      light -= ACTOR_LIGHT_FIRST;       /* otherwise get index for entry */

   dark = (field_he->orig_actor->type & ACTOR_MASK);
   if (dark >= ACTOR_AIR_ELEM && dark <= ACTOR_WATER_ELEM)
      dark = ACTOR_NUM_DARK;            /* use last entry in stats array */
   else
      dark -= ACTOR_DARK_FIRST;         /* otherwise get index for entry */

   damage = &computer_damage[light][dark];
   damage->light_sum += field_he->orig_life - field_he->life;
   damage->dark_sum += field_me->orig_life - field_me->life;
   damage->light_hits += field_me->num_hits;
   damage->dark_hits += field_he->num_hits;
   damage->count++;
}

/*--------------------------------------------------------------------------*/
/* computer_field_score                                                     */
/*--------------------------------------------------------------------------*/

int computer_field_score(ACTOR *attacker, ACTOR *defender,
                         int *attacker_damage, int *defender_damage,
                         int *attacker_hits, int *defender_hits)
{
   int light, dark;
   DAMAGE *damage;

   if (attacker->type & ACTOR_LIGHT) {
      light = attacker->type & ACTOR_MASK;
      dark = defender->type & ACTOR_MASK;
   } else {
      light = defender->type & ACTOR_MASK;
      dark = attacker->type & ACTOR_MASK;
   }

   if (light >= ACTOR_AIR_ELEM && light <= ACTOR_WATER_ELEM)
      light = ACTOR_NUM_LIGHT;          /* use last entry in stats array */
   else
      light -= ACTOR_LIGHT_FIRST;       /* otherwise get index for entry */

   if (dark >= ACTOR_AIR_ELEM && dark <= ACTOR_WATER_ELEM)
      dark = ACTOR_NUM_DARK;            /* use last entry in stats array */
   else
      dark -= ACTOR_DARK_FIRST;         /* otherwise get index for entry */

   damage = &computer_damage[light][dark];
   if (damage->count == 0) {
      *attacker_damage = 0;
      *defender_damage = 0;
      *attacker_hits = 1;
      *defender_hits = 1;
   } else if (attacker->type & ACTOR_LIGHT) {
      *attacker_damage = damage->light_sum / damage->count;
      *defender_damage = damage->dark_sum / damage->count;
      *attacker_hits = vmax(1, damage->light_hits);
      *defender_hits = vmax(1, damage->dark_hits);
   } else {
      *attacker_damage = damage->dark_sum / damage->count;
      *defender_damage = damage->light_sum / damage->count;
      *attacker_hits = vmax(1, damage->dark_hits);
      *defender_hits = vmax(1, damage->light_hits);
   }
   return damage->count;
}

/*--------------------------------------------------------------------------*/
/* computer_config_read                                                     */
/*--------------------------------------------------------------------------*/

void computer_config_read(FILE *fp, COMPUTER_CONFIG *config)
{
   char *path;
   FILE *fp1;
   int i, j;
   DAMAGE *damage;

   if (fp != NULL) {
      fscanf(fp, "%32s %d", config->rules, &config->old_board_mode);
      if (strcmp(config->rules, "easy") == 0)
         config->data_num = 0;
      else if (strcmp(config->rules, "medium") == 0)
         config->data_num = 1;
      else if (strcmp(config->rules, "hard") == 0)
         config->data_num = 2;
      else {
         fprintf(stderr, "computer_config_read():  unknown rules `%s'\n", config->rules);
         exit(EXIT_FAILURE);
      }
   }

   if (!computer_damage_ok) {
      path = malloc(PATH_MAX);
      sprintf(path, "%s/statistics", DATADIR);
      fp1 = fopen(path, "r");
      if (fp1 == NULL) {
         fprintf(stderr, "statistics file `%s' not found\n", path);
         exit(EXIT_FAILURE);
      }
      for (i = 0; i < ACTOR_NUM_LIGHT + 1; i++)
         for (j = 0; j < ACTOR_NUM_DARK + 1; j++) {
            damage = &computer_damage[i][j];
            if (fscanf(fp1, "%d %d %d %d %d",
                       &damage->light_sum, &damage->dark_sum,
                       &damage->light_hits, &damage->dark_hits,
                       &damage->count) == EOF) {
               fprintf(stderr, "computer_config_read():  not enough statistics\n");
               exit(EXIT_FAILURE);
            }
         }
      fclose(fp1);
      free(path);
      computer_damage_ok = 1;
   }
}

/*--------------------------------------------------------------------------*/
/* computer_config_write                                                    */
/*--------------------------------------------------------------------------*/

void computer_config_write(FILE *fp, COMPUTER_CONFIG *config)
{
   fprintf(fp, "%-32s %d\n",
           (config->rules[0] == 0) ? "easy" : config->rules,
           config->old_board_mode);

#ifdef AUTOPILOT
   { int i, j;
   for (i = 0; i < ACTOR_NUM_LIGHT + 1; i++) {
      for (j = 0; j < ACTOR_NUM_DARK + 1; j++)
         printf("%5d %5d %5d %5d %5d\n",
                 computer_damage[i][j].light_sum,
                 computer_damage[i][j].dark_sum,
                 computer_damage[i][j].light_hits,
                 computer_damage[i][j].dark_hits,
                 computer_damage[i][j].count);
      printf("\n");
   }
   }
#endif
}
