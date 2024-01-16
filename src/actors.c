/*--------------------------------------------------------------------------*/
/* game actors                                                              */
/*--------------------------------------------------------------------------*/

#include <config.h>

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "actors.h"
#include "sprite.h"
#include "audio.h"
#include "main.h"

/*--------------------------------------------------------------------------*/
/* defines                                                                  */
/*--------------------------------------------------------------------------*/

#define DURATION(x) ((int)((x)*TIME_SCALE + 0.5))
#define SPEED(x) x
#define DISTANCE(x) x
#define HEALTH(x) ((x)*HEALTH_SCALE)
#define DAMAGE(x) ((x)*HEALTH_SCALE)

/*--------------------------------------------------------------------------*/
/* weapon defines                                                           */
/*--------------------------------------------------------------------------*/

#define STEER_SHOT_FLAGS(s,x) ACTOR_WEAPON_STEER| ACTOR_##s##_##x   | ACTOR_##s
#define SHOOT_FLAGS(s,x)     ACTOR_WEAPON_SHOOT | ACTOR_##s##_##x   | ACTOR_##s
#define CLOUD_FLAGS(s)       ACTOR_WEAPON_CLOUD | ACTOR_##s##_CLOUD | ACTOR_##s
#define CLONE_FLAGS          ACTOR_WEAPON_CLONE | ACTOR_DARK_CLONE  | ACTOR_DARK
#define HAND_FLAGS(s,x)      ACTOR_WEAPON_HAND  | ACTOR_##s##_##x   | ACTOR_##s | ACTOR_FREEZE_USER
#define AUTO_FLAGS(s,x)      ACTOR_WEAPON_AUTO  | ACTOR_##s##_##x   | ACTOR_##s | ACTOR_FREEZE_USER

#define RECHARGE_FASTEST     DURATION(10)
#define RECHARGE_VERY_FAST   DURATION(15)
#define RECHARGE_FAST        DURATION(18)
#define RECHARGE_MODERATE    DURATION(20)
#define RECHARGE_SLOW        DURATION(23)
#define RECHARGE_VERY_SLOW   DURATION(25)
#define RECHARGE_SLOWEST     DURATION(30)

#define SHOOT_DELAY          DURATION(1)

/*--------------------------------------------------------------------------*/
/* monster defines                                                          */
/*--------------------------------------------------------------------------*/

#define MONSTER(x,s,m)     ACTOR_##x        | ACTOR_##s       | ACTOR_##m
#define ELEMENTAL(x)       ACTOR_##x##_ELEM | ACTOR_ELEMENTAL

#define MONSTER2(x,s)      ACTOR_##x        | ACTOR_##s

/*--------------------------------------------------------------------------*/
/* structures                                                               */
/*--------------------------------------------------------------------------*/

typedef struct {
   char *name;
   char *descr;
   char *author;
} THEME;

/*--------------------------------------------------------------------------*/
/* functions                                                                */
/*--------------------------------------------------------------------------*/

static void actors_load_sprites(void);

/*--------------------------------------------------------------------------*/
/* variables                                                                */
/*--------------------------------------------------------------------------*/

static void (*progress_func)(char *msg, float progress);

ACTOR orig_actors_list[ACTOR_NUM_ACTORS] = {

   { },                                 /* the null actor, actors[0] */

/*--------------------------------------------------------------------------*/
/* Archon weapons                                                           */
/*--------------------------------------------------------------------------*/

   { "knight_weapon", NULL, HAND_FLAGS(LIGHT,SWORD), 0,
         DAMAGE(5), 0, 0, RECHARGE_FASTEST, DURATION(4), 0 },
   { "phoenix_weapon", NULL,
         CLOUD_FLAGS(LIGHT) | ACTOR_IMMUNE_USER | ACTOR_FREEZE_USER, 0,
         DAMAGE(2), 0, 0, RECHARGE_VERY_SLOW, DURATION(10), DURATION(2) },
   { "golem_weapon", NULL, SHOOT_FLAGS(LIGHT,BOULDER), SPEED(12),
         DAMAGE(10), 0, 0, RECHARGE_MODERATE, 0, 0 },
   { "archer_weapon", NULL, SHOOT_FLAGS(LIGHT,ARROW), SPEED(16),
         DAMAGE(5), 0, 0, RECHARGE_MODERATE, 0, 0 },
   { "valkyrie_weapon", NULL, SHOOT_FLAGS(LIGHT,MAGICSPEAR), SPEED(12),
         DAMAGE(7), 0, 0, RECHARGE_MODERATE, 0, 0 },
   { "unicorn_weapon", NULL, SHOOT_FLAGS(LIGHT,ENERGYBOLT), SPEED(28),
         DAMAGE(7), 0, 0, RECHARGE_VERY_FAST, 0, 0 },
   { "djinni_weapon", NULL, SHOOT_FLAGS(LIGHT,WHIRLWIND), SPEED(20),
         DAMAGE(6), 0, 0, RECHARGE_SLOW, 0, 0 },
   { "wizard_weapon", NULL, SHOOT_FLAGS(LIGHT,FIREBALL), SPEED(20),
         DAMAGE(10), 0, 0, RECHARGE_MODERATE, 0, 0 },

   { "air_elem_weapon", NULL, SHOOT_FLAGS(ELEMENTAL,TORNADO), SPEED(16),
         DAMAGE(5), 0, 0, RECHARGE_FAST, 0, 0 },
   { "earth_elem_weapon", NULL, SHOOT_FLAGS(ELEMENTAL,BOULDER), SPEED(12),
         DAMAGE(9), 0, 0, RECHARGE_VERY_SLOW, 0, 0 },
   { "fire_elem_weapon", NULL, SHOOT_FLAGS(ELEMENTAL,FIERYBOLT), SPEED(20),
         DAMAGE(15), 0, 0, RECHARGE_VERY_FAST, 0, 0 },
   { "water_elem_weapon", NULL, SHOOT_FLAGS(ELEMENTAL,ICEBOLT), SPEED(12),
         DAMAGE(6), 0, 0, RECHARGE_VERY_SLOW, 0, 0 },

   { "goblin_weapon", NULL, HAND_FLAGS(DARK,CLUB), 0,
         DAMAGE(5), 0, 0, RECHARGE_FASTEST, DURATION(4), 0 },
   { "banshee_weapon", NULL, CLOUD_FLAGS(DARK), 0,
         DAMAGE(1), 0, 0, RECHARGE_VERY_SLOW, DURATION(15), 0 },
   { "troll_weapon", NULL, SHOOT_FLAGS(DARK,BOULDER), SPEED(12),
         DAMAGE(10), 0, 0, RECHARGE_MODERATE, 0, 0 },
   { "manticore_weapon", NULL, SHOOT_FLAGS(DARK,TAILSPIKES), SPEED(12),
         DAMAGE(4), 0, 0, RECHARGE_MODERATE, 0, 0 },
   { "basilisk_weapon", NULL, SHOOT_FLAGS(DARK,EYEBEAM), SPEED(28),
         DAMAGE(9), 0, 0, RECHARGE_VERY_FAST, 0, 0 },
   { "dragon_weapon", NULL, SHOOT_FLAGS(DARK,FIERYBREATH), SPEED(16),
         DAMAGE(11), 0, 0, RECHARGE_SLOWEST, 0, 0 },
   { "shapeshifter_weapon", NULL, CLONE_FLAGS, 0,
         0, 0, 0, 0 },  /* not really a weapon */
   { "sorceress_weapon", NULL, SHOOT_FLAGS(DARK,LIGHTNINGBOLT), SPEED(24),
         DAMAGE(8), 0, 0, RECHARGE_MODERATE, 0, 0 },

/*--------------------------------------------------------------------------*/
/* Archon monsters                                                          */
/*--------------------------------------------------------------------------*/

   { "knight", NULL, MONSTER(KNIGHT,LIGHT,GROUND), SPEED(4), HEALTH(5),
         DISTANCE(3), ACTOR_LIGHT_SWORD, 0, 0, 0 },
   { "archer", NULL, MONSTER(ARCHER,LIGHT,GROUND), SPEED(4), HEALTH(5),
         DISTANCE(3), ACTOR_LIGHT_ARROW, 0, 0, 0 },
   { "valkyrie", NULL, MONSTER(VALKYRIE,LIGHT,FLY), SPEED(4), HEALTH(8),
         DISTANCE(3), ACTOR_LIGHT_MAGICSPEAR, 0, SHOOT_DELAY, 0 },
   { "golem", NULL, MONSTER(GOLEM,LIGHT,GROUND), SPEED(3), HEALTH(15),
         DISTANCE(3), ACTOR_LIGHT_BOULDER, 0, SHOOT_DELAY, 0 },
   { "unicorn", NULL, MONSTER(UNICORN,LIGHT,GROUND), SPEED(4), HEALTH(9),
         DISTANCE(4), ACTOR_LIGHT_ENERGYBOLT, 0, SHOOT_DELAY, 0 },
   { "djinni", NULL, MONSTER(DJINNI,LIGHT,FLY), SPEED(4), HEALTH(15),
         DISTANCE(4), ACTOR_LIGHT_WHIRLWIND, 0, SHOOT_DELAY, 0 },
   { "phoenix", NULL, MONSTER(PHOENIX,LIGHT,FLY), SPEED(4), HEALTH(12),
         DISTANCE(5), ACTOR_LIGHT_CLOUD, 0, 0, 0 },
   { "wizard", NULL, MONSTER(WIZARD,LIGHT,MASTER), SPEED(4), HEALTH(10),
         DISTANCE(3), ACTOR_LIGHT_FIREBALL, 0, SHOOT_DELAY, 0 },

   { "air_elem", NULL, ELEMENTAL(AIR), SPEED(4), HEALTH(12),
         0, ACTOR_ELEMENTAL_TORNADO, 0, SHOOT_DELAY, 0 },
   { "earth_elem", NULL, ELEMENTAL(EARTH), SPEED(3), HEALTH(17),
         0, ACTOR_ELEMENTAL_BOULDER, 0, SHOOT_DELAY, 0 },
   { "fire_elem", NULL, ELEMENTAL(FIRE), SPEED(4), HEALTH(10),
         0, ACTOR_ELEMENTAL_FIERYBOLT, 0, SHOOT_DELAY, 0 },
   { "water_elem", NULL, ELEMENTAL(WATER), SPEED(4), HEALTH(14),
         0, ACTOR_ELEMENTAL_ICEBOLT, 0, SHOOT_DELAY, 0 },

   { "goblin", NULL, MONSTER(GOBLIN,DARK,GROUND), SPEED(4), HEALTH(5),
         DISTANCE(3), ACTOR_DARK_CLUB, 0, 0, 0 },
   { "manticore", NULL, MONSTER(MANTICORE,DARK,GROUND), SPEED(4), HEALTH(8),
         DISTANCE(3), ACTOR_DARK_TAILSPIKES, 0, SHOOT_DELAY, 0 },
   { "banshee", NULL, MONSTER(BANSHEE,DARK,FLY), SPEED(4), HEALTH(8),
         DISTANCE(3), ACTOR_DARK_CLOUD, 0, 0, 0 },
   { "troll", NULL, MONSTER(TROLL,DARK,GROUND), SPEED(3), HEALTH(14),
         DISTANCE(3), ACTOR_DARK_BOULDER, 0, SHOOT_DELAY, 0 },
   { "basilisk", NULL, MONSTER(BASILISK,DARK,GROUND), SPEED(4), HEALTH(6),
         DISTANCE(3), ACTOR_DARK_EYEBEAM, 0, SHOOT_DELAY, 0 },
   { "dragon", NULL, MONSTER(DRAGON,DARK,FLY), SPEED(4), HEALTH(17),
         DISTANCE(4), ACTOR_DARK_FIERYBREATH, 0, SHOOT_DELAY, 0 },
   { "shapeshifter", NULL, MONSTER(SHAPESHIFTER,DARK,FLY), SPEED(4), 0,
         DISTANCE(5), ACTOR_DARK_CLONE, 0, 0, 0 },
   { "sorceress", NULL, MONSTER(SORCERESS,DARK,MASTER), SPEED(4), HEALTH(10),
         DISTANCE(3), ACTOR_DARK_LIGHTNINGBOLT, 0, SHOOT_DELAY, 0 },

/*--------------------------------------------------------------------------*/
/* Adept weapons                                                            */
/*--------------------------------------------------------------------------*/

   { "giant_weapon", NULL, SHOOT_FLAGS(ORDER,BOULDER), SPEED(16),
         DAMAGE(7), 0, 0, DURATION(21), 0, 0 },
   { "kraken_weapon", NULL, SHOOT_FLAGS(ORDER,TIDALWAVE), SPEED(16),
         DAMAGE(8), 0, 0, DURATION(21), DURATION(6), DURATION(2) },
   { "thunderbird_weapon", NULL, SHOOT_FLAGS(ORDER,THUNDERBOLT), SPEED(30),
         DAMAGE(7), 0, 0, DURATION(19), 0, 0 },
   { "salamander_weapon", NULL, SHOOT_FLAGS(ORDER,FIRE_PELLET), SPEED(24),
         DAMAGE(7), 0, 0, DURATION(17), 0, 0 },
   { "order_weapon", NULL, STEER_SHOT_FLAGS(ORDER,BOLT), SPEED(16),
         DAMAGE(8), 0, 0, DURATION(17), DURATION(17), 0 },
   { "order_apocalypse_weapon", NULL, STEER_SHOT_FLAGS(ORDER,APOCALYPSE_BOLT),
         0, 0, 0, 0, DURATION(17), DURATION(17), 0 },

   { "behemoth_weapon", NULL, HAND_FLAGS(CHAOS,FIST), 0,
         DAMAGE(10), 0, 0, DURATION(10), DURATION(3), 0 },
   { "siren_weapon", NULL, AUTO_FLAGS(CHAOS,SIREN_SONG), 0,
         DAMAGE(1), 0, 0, DURATION(5), 0, 0 },
   { "ifrit_weapon", NULL, SHOOT_FLAGS(CHAOS,IFRIT_LIGHTNING), SPEED(24),
         DAMAGE(6), 0, 0, DURATION(15), 0, 0 },
   { "firebird_weapon", NULL,
         CLOUD_FLAGS(CHAOS) | ACTOR_IMMUNE_USER | ACTOR_FREEZE_USER, 0,
         DAMAGE(2), 0, 0, DURATION(21), DURATION(10), DURATION(2) },
   { "chaos_weapon", NULL, STEER_SHOT_FLAGS(CHAOS,BOLT), SPEED(16),
         DAMAGE(8), 0, 0, DURATION(17), DURATION(17), 0 },
   { "chaos_apocalypse_weapon", NULL, STEER_SHOT_FLAGS(CHAOS,APOCALYPSE_BOLT),
         0, 0, 0, 0, DURATION(17), DURATION(17), 0 },

   { "juggernaut_weapon", NULL,
         SHOOT_FLAGS(DEMON,JUGGERNAUT_RAM) | ACTOR_IMMUNE_USER, SPEED(10),
         DAMAGE(9), 0, 0, DURATION(25), 0, 0 },
   { "wraith_weapon", NULL, CLOUD_FLAGS(DEMON) | ACTOR_WEAPON_VAMPIRIC, 0,
         DAMAGE(1), 0, 0, DURATION(21), DURATION(13), 0 },
   { "gorgon_weapon", NULL,
         SHOOT_FLAGS(DEMON,PARALYSIS) | ACTOR_WEAPON_PARALYSE, SPEED(36),
         0, 0, 0, DURATION(13), 0, 0 },
   { "chimera_weapon1", NULL, SHOOT_FLAGS(DEMON,CHIMERA_BREATH), SPEED(24),
         DAMAGE(7), 0, ACTOR_DEMON_CHIMERA_GAS, DURATION(17), 0, 0 },
   { "chimera_weapon2", NULL, SHOOT_FLAGS(DEMON,CHIMERA_GAS), SPEED(16),
         DAMAGE(8), 0, ACTOR_DEMON_CHIMERA_STING,
         DURATION(17), DURATION(6), DURATION(2) },
   { "chimera_weapon3", NULL, HAND_FLAGS(DEMON,CHIMERA_STING), 0,
         DAMAGE(9), 0, ACTOR_DEMON_CHIMERA_BREATH,
         DURATION(17), DURATION(3), 0 },

/*--------------------------------------------------------------------------*/
/* Adept monsters                                                           */
/*--------------------------------------------------------------------------*/

   { "giant", NULL, MONSTER2(GIANT,ORDER), SPEED(3.6), HEALTH(10),
         0, ACTOR_ORDER_BOULDER, 0, 0, 0 },
   { "kraken", NULL, MONSTER2(KRAKEN,ORDER), SPEED(3.6), HEALTH(11),
         0, ACTOR_ORDER_TIDALWAVE, 0, 0, 0 },
   { "thunderbird", NULL, MONSTER2(THUNDERBIRD,ORDER), SPEED(4.8), HEALTH(12),
         0, ACTOR_ORDER_THUNDERBOLT, 0, 0, 0 },
   { "salamander", NULL, MONSTER2(SALAMANDER,ORDER), SPEED(4.2), HEALTH(5),
         0, ACTOR_ORDER_FIRE_PELLET, 0, 0, 0 },
   { "adept", NULL, MONSTER2(ORDER_ADEPT,ORDER), SPEED(4.8), HEALTH(12),
         0, ACTOR_ORDER_BOLT, 0, 0, 0 },
   { "citadel", NULL, MONSTER2(ORDER_APOCALYPSE,ORDER), SPEED(4.8), 0,
         0, ACTOR_ORDER_APOCALYPSE_BOLT, 0, 0, 0 },

   { "behemoth", NULL, MONSTER2(BEHEMOTH,CHAOS), SPEED(4.2), HEALTH(12),
         0, ACTOR_CHAOS_FIST, 0, 0, 0 },
   { "siren", NULL, MONSTER2(SIREN,CHAOS), SPEED(3.6), HEALTH(7),
         0, ACTOR_CHAOS_SIREN_SONG, 0, 0, 0 },
   { "ifrit", NULL, MONSTER2(IFRIT,CHAOS), SPEED(4.8), HEALTH(11),
         0, ACTOR_CHAOS_IFRIT_LIGHTNING, 0, 0, 0 },
   { "firebird", NULL, MONSTER2(FIREBIRD,CHAOS), SPEED(4.8), HEALTH(11),
         0, ACTOR_CHAOS_CLOUD, 0, 1, 0 },
   { "adept", NULL, MONSTER2(CHAOS_ADEPT,CHAOS), SPEED(4.8), HEALTH(12),
         0, ACTOR_CHAOS_BOLT, 0, 0, 0 },
   { "citadel", NULL, MONSTER2(CHAOS_APOCALYPSE,CHAOS), SPEED(4.8), 0,
         0, ACTOR_CHAOS_APOCALYPSE_BOLT, 0, 0, 0 },

   { "juggernaut", NULL, MONSTER2(JUGGERNAUT,DEMON), SPEED(4.8), HEALTH(18),
         0, ACTOR_DEMON_JUGGERNAUT_RAM, 0, 1, 0 },
   { "wraith", NULL, MONSTER2(WRAITH,DEMON), SPEED(4.8), HEALTH(10),
         0, ACTOR_DEMON_CLOUD, 0, 0, 0 },
   { "gorgon", NULL, MONSTER2(GORGON,DEMON), SPEED(4.8), HEALTH(10),
         0, ACTOR_DEMON_PARALYSIS, 0, 0, 0 },
   { "chimera", NULL, MONSTER2(CHIMERA,DEMON), SPEED(4.8), HEALTH(20),
         0, ACTOR_DEMON_CHIMERA_BREATH, 0, 0, 0 },

/*--------------------------------------------------------------------------*/
/* end of table                                                             */
/*--------------------------------------------------------------------------*/

   { }                                  /* indicator */

};

ACTOR actors_list[ACTOR_NUM_ACTORS];

/*--------------------------------------------------------------------------*/
/* other sprites                                                            */
/*--------------------------------------------------------------------------*/

void *floor_sprite;
void *cursor_sprite;

/*--------------------------------------------------------------------------*/
/* state movement tables                                                    */
/*--------------------------------------------------------------------------*/

int state_move_x_step[STATE_MOVE_COUNT] = { 0, 0, 0, -1, 1, -1, -1, 1, 1 };
int state_move_y_step[STATE_MOVE_COUNT] = { 0, -1, 1, 0, 0, -1, 1, -1, 1 };

int state_opposite[STATE_MOVE_COUNT] = {
   0,
   STATE_MOVE_DOWN,                     /* for STATE_MOVE_UP */
   STATE_MOVE_UP,                       /* for STATE_MOVE_DOWN */
   STATE_MOVE_RIGHT,                    /* for STATE_MOVE_LEFT */
   STATE_MOVE_LEFT,                     /* for STATE_MOVE_RIGHT */
   STATE_MOVE_DOWN_RIGHT,               /* for STATE_MOVE_UP_LEFT */
   STATE_MOVE_UP_RIGHT,                 /* for STATE_MOVE_DOWN_LEFT */
   STATE_MOVE_DOWN_LEFT,                /* for STATE_MOVE_UP_RIGHT */
   STATE_MOVE_UP_LEFT                   /* for STATE_MOVE_DOWN_RIGHT */
};

int state_perpendicular[STATE_MOVE_COUNT] = {
   0,
   STATE_MOVE_LEFT,                     /* for STATE_MOVE_UP */
   STATE_MOVE_RIGHT,                    /* for STATE_MOVE_DOWN */
   STATE_MOVE_DOWN,                     /* for STATE_MOVE_LEFT */
   STATE_MOVE_UP,                       /* for STATE_MOVE_RIGHT */
   STATE_MOVE_DOWN_LEFT,                /* for STATE_MOVE_UP_LEFT */
   STATE_MOVE_DOWN_RIGHT,               /* for STATE_MOVE_DOWN_LEFT */
   STATE_MOVE_UP_LEFT,                  /* for STATE_MOVE_UP_RIGHT */
   STATE_MOVE_UP_RIGHT                  /* for STATE_MOVE_DOWN_RIGHT */
};

int state_adjacent[STATE_MOVE_COUNT] = {
  0,
  STATE_MOVE_UP_RIGHT,                  /* for STATE_MOVE_UP */
  STATE_MOVE_DOWN_LEFT,                 /* for STATE_MOVE_DOWN */
  STATE_MOVE_UP_LEFT,                   /* for STATE_MOVE_LEFT */
  STATE_MOVE_DOWN_RIGHT,                /* for STATE_MOVE_RIGHT */
  STATE_MOVE_UP,                        /* for STATE_MOVE_UP_LEFT */
  STATE_MOVE_LEFT,                      /* for STATE_MOVE_DOWN_LEFT */
  STATE_MOVE_RIGHT,                     /* for STATE_MOVE_UP_RIGHT */
  STATE_MOVE_DOWN                       /* for STATE_MOVE_DOWN_RIGHT */
};

/*--------------------------------------------------------------------------*/
/* actors_init                                                              */
/*--------------------------------------------------------------------------*/

void actors_init(void (*_progress_func)(char *msg, float progress))
{
   /* actors */
   sprite_add_state(STATE_MOVE_UP, "move.up");
   sprite_add_state(STATE_MOVE_DOWN, "move.down");
   sprite_add_state(STATE_MOVE_LEFT, "move.left");
   sprite_add_state(STATE_MOVE_RIGHT, "move.right");
   sprite_add_state(STATE_MOVE_UP_LEFT, "move.upleft");
   sprite_add_state(STATE_MOVE_DOWN_LEFT, "move.downleft");
   sprite_add_state(STATE_MOVE_UP_RIGHT, "move.upright");
   sprite_add_state(STATE_MOVE_DOWN_RIGHT, "move.downright");
   sprite_add_state(STATE_FIRE_ANY, "fire.any");
   sprite_add_state(STATE_FIRE_UP, "fire.up");
   sprite_add_state(STATE_FIRE_DOWN, "fire.down");
   sprite_add_state(STATE_FIRE_LEFT, "fire.left");
   sprite_add_state(STATE_FIRE_RIGHT, "fire.right");
   sprite_add_state(STATE_FIRE_UP_LEFT, "fire.upleft");
   sprite_add_state(STATE_FIRE_DOWN_LEFT, "fire.downleft");
   sprite_add_state(STATE_FIRE_UP_RIGHT, "fire.upright");
   sprite_add_state(STATE_FIRE_DOWN_RIGHT, "fire.downright");

   /* elementals */
   sprite_add_state(STATE_AIR_ELEM, "elem.air");
   sprite_add_state(STATE_EARTH_ELEM, "elem.earth");
   sprite_add_state(STATE_FIRE_ELEM, "elem.fire");
   sprite_add_state(STATE_WATER_ELEM, "elem.water");

   /* other states */
   sprite_add_state(STATE_BOARD, "board");
   sprite_add_state(STATE_POWER, "power");
   sprite_add_state(STATE_EMPTY, "empty");
   sprite_add_state(STATE_FIELD, "field");
   sprite_add_state(STATE_ROCK,  "rock");

   progress_func = _progress_func;
}

/*--------------------------------------------------------------------------*/
/* actors_load_theme                                                        */
/*--------------------------------------------------------------------------*/

void actors_load_theme(char *name)
{
   char *path, old_name[64], new_name[64];
   FILE *fp;
   ACTOR *actor;

   path = malloc(PATH_MAX);
   sprintf(path, "%s/%s/NAMES", DATADIR, name);
   fp = fopen(path, "r");
   if (fp == NULL) {
      fprintf(stderr, "Could not open `%s'.  Giving up.\n", path);
      exit(EXIT_FAILURE);
   }

   for (actor = &actors_list[1]; actor->name[0] != 0; actor++)
      if (actor->sprite != NULL)
         sprite_free(actor->sprite);

   memcpy(actors_list, orig_actors_list, sizeof(orig_actors_list));
   while (!feof(fp)) {
      fscanf(fp, "%s %s", old_name, new_name);
      if (strcmp(old_name, "end") == 0)
         break;
      for (actor = &actors_list[1]; actor->name[0] != 0; actor++)
         if (strcmp(actor->name, old_name) == 0)
            strcpy(actor->name, new_name);
   }
   fclose(fp);

   sprintf(path, "%s/%s", DATADIR, name);
   sprite_set_path(path);
   actors_load_sprites();
   sprintf(path, "%s/%s/wav", DATADIR, name);
   audio_load_theme(path);
   free(path);
}

/*--------------------------------------------------------------------------*/
/* actors_load_sprites                                                      */
/*--------------------------------------------------------------------------*/

void actors_load_sprites(void)
{
   ACTOR *actor;
   char name[64];
   char msg[128];
   int step;

   step = 0;
   for (actor = &actors_list[1]; actor->name[0] != 0; actor++) {
      if ((actor->type & (ACTOR_ELEMENTAL | ACTOR_WEAPON)) == ACTOR_ELEMENTAL)
         sprintf(name, "actors/elemental/%s", actor->name);
      else
         sprintf(name, "%s/%s/%s",
                  (actor->type & ACTOR_WEAPON) ? "weapons" : "actors",
                     (actor->type & ACTOR_ELEMENTAL) ? "elemental" :
                        (actor->type & ACTOR_LIGHT) ? "light" : "dark",
                  actor->name);
      if (progress_func != NULL) {
         sprintf(msg, "Loading graphics: `%s'", name);
         progress_func(msg, (float)step / ACTOR_NUM_ACTORS);
         step++;
      }
      actor->sprite = sprite_load(name);
   }

   /* other sprites */
   floor_sprite = sprite_load("floor");
   sprite_set_state(floor_sprite, STATE_BOARD, 0);
   cursor_sprite = sprite_load("cursor");
   sprite_set_state(cursor_sprite, STATE_BOARD, 0);

   progress_func("Theme has been successfully loaded", 1.0);
}

/*--------------------------------------------------------------------------*/
/* actor_is_side                                                            */
/*--------------------------------------------------------------------------*/

int actor_is_side(ACTOR *actor, int side)
{
   return ((actor->type & ACTOR_LIGHT) == ((side) ? 0 : ACTOR_LIGHT));
}

/*--------------------------------------------------------------------------*/
/* state_is_perpendicular                                                   */
/*--------------------------------------------------------------------------*/

int state_is_perpendicular(int state1, int state2)
{
  return ( state1 == state_perpendicular[state2] ||
	   state2 == state_perpendicular[state1] );
}

/*--------------------------------------------------------------------------*/
/* state_is_adjacent                                                        */
/*--------------------------------------------------------------------------*/

int state_is_adjacent(int state1, int state2)
{
  return ( state1 == state_adjacent[state2] ||
	   state2 == state_adjacent[state1] );
}
