/*--------------------------------------------------------------------------*/
/* game actors                                                              */
/*--------------------------------------------------------------------------*/

#ifndef __MY_ACTORS_H
#define __MY_ACTORS_H

/* this file defines many constants using enums, many parts of the board */
/* and field depend on the order.  do not modify without good reason.    */

/*--------------------------------------------------------------------------*/
/* actor states                                                             */
/*--------------------------------------------------------------------------*/

enum {

   STATE_FIRE = 0,                      /* for querying about fire key */

   /* move states */
   STATE_MOVE_UP = 1,                   /* STATE_MOVE_* constants must */
   STATE_MOVE_DOWN,                     /*   have values 1 through 8:  */
   STATE_MOVE_LEFT,                     /*   don't move them!          */
   STATE_MOVE_RIGHT,
   STATE_MOVE_UP_LEFT,
   STATE_MOVE_DOWN_LEFT,
   STATE_MOVE_UP_RIGHT,
   STATE_MOVE_DOWN_RIGHT,

   /* fire states */
   STATE_FIRE_UP,                       /* must have same order as */
   STATE_FIRE_DOWN,                     /* STATE_MOVE_* constants */
   STATE_FIRE_LEFT,
   STATE_FIRE_RIGHT,
   STATE_FIRE_UP_LEFT,
   STATE_FIRE_DOWN_LEFT,
   STATE_FIRE_UP_RIGHT,
   STATE_FIRE_DOWN_RIGHT,
   STATE_FIRE_ANY,
   
   /* elemental states */
   STATE_AIR_ELEM,
   STATE_EARTH_ELEM,
   STATE_FIRE_ELEM,
   STATE_WATER_ELEM,

   /* other states */
   STATE_BOARD,                         /* floor/cursor on the board */
   STATE_POWER,                         /* power point */
   STATE_EMPTY,                         /* black cell */
   STATE_FIELD,                         /* field cell */
   STATE_ROCK,                          /* field rock */

   STATE_MOVE_FIRST = STATE_MOVE_UP,
   STATE_MOVE_LAST = STATE_MOVE_DOWN_RIGHT,
   STATE_MOVE_AXIAL_LAST = STATE_MOVE_RIGHT,
   STATE_MOVE_COUNT = (STATE_MOVE_LAST - STATE_MOVE_FIRST + 2),
   STATE_FIRE_FIRST = STATE_FIRE_UP
};

/*--------------------------------------------------------------------------*/
/* actor numbers                                                            */
/*--------------------------------------------------------------------------*/

enum {
   ACTOR_NULL,

   /* Archon actors */

   ACTOR_LIGHT_SWORD,                   /* weapons of light sides */
   ACTOR_LIGHT_CLOUD,
   ACTOR_LIGHT_BOULDER,
   ACTOR_LIGHT_ARROW,
   ACTOR_LIGHT_MAGICSPEAR,
   ACTOR_LIGHT_ENERGYBOLT,
   ACTOR_LIGHT_WHIRLWIND,
   ACTOR_LIGHT_FIREBALL,

   ACTOR_ELEMENTAL_TORNADO,             /* weapons of the mighty elementals */
   ACTOR_ELEMENTAL_BOULDER,             /*   must be placed immediately */
   ACTOR_ELEMENTAL_FIERYBOLT,           /*   after weapons of the light side */
   ACTOR_ELEMENTAL_ICEBOLT,
   
   ACTOR_DARK_CLUB,                     /* weapons of the dark side */
   ACTOR_DARK_CLOUD,
   ACTOR_DARK_BOULDER,
   ACTOR_DARK_TAILSPIKES,
   ACTOR_DARK_EYEBEAM,
   ACTOR_DARK_FIERYBREATH,
   ACTOR_DARK_CLONE,
   ACTOR_DARK_LIGHTNINGBOLT,

   ACTOR_KNIGHT,                        /* actors of the light side */
   ACTOR_ARCHER,
   ACTOR_VALKYRIE,
   ACTOR_GOLEM,
   ACTOR_UNICORN,
   ACTOR_DJINNI,
   ACTOR_PHOENIX,
   ACTOR_WIZARD,

   ACTOR_AIR_ELEM,                      /* the four mighty elementals */
   ACTOR_EARTH_ELEM,                    /*   must be placed immediately */
   ACTOR_FIRE_ELEM,                     /*   following the light side */
   ACTOR_WATER_ELEM,

   ACTOR_GOBLIN,                        /* actors of the dark side */
   ACTOR_MANTICORE,
   ACTOR_BANSHEE,
   ACTOR_TROLL,
   ACTOR_BASILISK,
   ACTOR_DRAGON,
   ACTOR_SHAPESHIFTER,
   ACTOR_SORCERESS,

   /* Adept actors */

   ACTOR_ORDER_BOULDER,                 /* Weapons of the Adept creatures */
   ACTOR_ORDER_TIDALWAVE,
   ACTOR_ORDER_THUNDERBOLT,
   ACTOR_ORDER_FIRE_PELLET,
   ACTOR_ORDER_BOLT,
   ACTOR_ORDER_APOCALYPSE_BOLT,

   ACTOR_CHAOS_FIST,
   ACTOR_CHAOS_SIREN_SONG,
   ACTOR_CHAOS_IFRIT_LIGHTNING,
   ACTOR_CHAOS_CLOUD,
   ACTOR_CHAOS_BOLT,
   ACTOR_CHAOS_APOCALYPSE_BOLT,

   ACTOR_DEMON_JUGGERNAUT_RAM,
   ACTOR_DEMON_CLOUD,
   ACTOR_DEMON_PARALYSIS,
   ACTOR_DEMON_CHIMERA_BREATH,
   ACTOR_DEMON_CHIMERA_GAS,
   ACTOR_DEMON_CHIMERA_STING,

   ACTOR_GIANT,                         /* Elementals and Adepts of Order */
   ACTOR_KRAKEN,
   ACTOR_THUNDERBIRD,
   ACTOR_SALAMANDER,
   ACTOR_ORDER_ADEPT,
   ACTOR_ORDER_APOCALYPSE,

   ACTOR_BEHEMOTH,                      /* Elementals and Adepts of Chaos */
   ACTOR_SIREN,
   ACTOR_IFRIT,
   ACTOR_FIREBIRD,
   ACTOR_CHAOS_ADEPT,
   ACTOR_CHAOS_APOCALYPSE,

   ACTOR_JUGGERNAUT,                    /* Demons */
   ACTOR_WRAITH,
   ACTOR_GORGON,
   ACTOR_CHIMERA,

   ACTOR_DUMMY_ACTOR,                   /* indicates end of actor list */
   ACTOR_NUM_ACTORS,

   ACTOR_LIGHT_FIRST = ACTOR_KNIGHT,
   ACTOR_LIGHT_LAST = ACTOR_WIZARD,

   ACTOR_DARK_FIRST = ACTOR_GOBLIN,
   ACTOR_DARK_LAST = ACTOR_SORCERESS,

   ACTOR_ORDER_FIRST = ACTOR_GIANT,
   ACTOR_ORDER_LAST = ACTOR_ORDER_APOCALYPSE,

   ACTOR_CHAOS_FIRST = ACTOR_BEHEMOTH,
   ACTOR_CHAOS_LAST = ACTOR_CHAOS_APOCALYPSE,

   ACTOR_DEMON_FIRST = ACTOR_JUGGERNAUT,
   ACTOR_DEMON_LAST = ACTOR_CHIMERA,

   ACTOR_NUM_LIGHT = ACTOR_LIGHT_LAST - ACTOR_LIGHT_FIRST + 1,
   ACTOR_NUM_DARK = ACTOR_DARK_LAST - ACTOR_DARK_FIRST + 1,

   ACTOR_MASK = 0x7F                    /* at most 128 actors allowed */
};

/*--------------------------------------------------------------------------*/
/* actor types                                                              */
/*--------------------------------------------------------------------------*/

enum {
   /* actor types */
   ACTOR_WEAPON = 0x8000,               /* if weapon, otherwise monster */
   ACTOR_MONSTER = 0x0,

   ACTOR_RIGHT = 0x4000,                /* if we face right on the board */
   ACTOR_LEFT = 0x0,                    /* if we face left on the board */
   
   ACTOR_LIGHT = ACTOR_RIGHT,
   ACTOR_DARK  = ACTOR_LEFT,

   ACTOR_CHAOS = ACTOR_RIGHT,
   ACTOR_ORDER = ACTOR_LEFT,

   ACTOR_DEMON = 0x2000,                /* demons can belong to either side */

   /* movement */
   ACTOR_FLY = 0x10000,                 /* flying monster */
   ACTOR_GROUND = 0x20000,              /* ground-walking monster */
   ACTOR_MASTER = 0x40000,              /* wizard/sorceress teleport */ 
   ACTOR_ELEMENTAL = 0x80000,           /* elemental teleport */

   /* weapon types */
   ACTOR_WEAPON_HAND = (ACTOR_WEAPON | 0x1000),   /* melee weapons */
   ACTOR_WEAPON_CLOUD = (ACTOR_WEAPON | 0x800),   /* clouds/explosions */
   ACTOR_WEAPON_SHOOT = (ACTOR_WEAPON | 0x400),   /* bullets */
   ACTOR_WEAPON_CLONE = (ACTOR_WEAPON | 0x200),   /* shapeshifter */
   ACTOR_WEAPON_AUTO = (ACTOR_WEAPON | 0x100),    /* hit enemy automatically */

   /* additional weapon abilities (independant of or in addition to type) */
   ACTOR_FREEZE_USER = 0x200000,        /* freezes the user */
   ACTOR_IMMUNE_USER = 0x400000,        /* renders the user immune to damage */
   ACTOR_WEAPON_VAMPIRIC = 0x800000,    /* gives health from target to user */
   ACTOR_WEAPON_PARALYSE = 0x1000000,   /* slows the target */
   ACTOR_WEAPON_STEER = (ACTOR_WEAPON_SHOOT | 0x2000000),
                                        /* steerable bullets */

};

/*--------------------------------------------------------------------------*/
/* structures                                                               */
/*--------------------------------------------------------------------------*/

typedef struct {                        /* actor (monster/weapon) */

   /* general */
   char name[64];
   void *sprite;                        /* sprite for actor */
   int type;
   short int speed;                     /* field speed, in pixels/frame */
   short int strength;                  /* monster health or weapon damage */

   /* monster only */
   short int distance;                  /* max board steps per turn */
   short int weapon;                    /* index to weapon */

   /* weapon only */
   short int recharge;                  /* recharge time */
   short int duration;                  /* duration for melee/cloud attacks */
   short int cloud_grow;                /* number of frames a cloud weapon */
                                        /* grows and shrinks */

} ACTOR;

/*--------------------------------------------------------------------------*/
/* functions                                                                */
/*--------------------------------------------------------------------------*/

void actors_init(void (*_progress_func)(char *msg, float progress));
void actors_load_theme(char *name);
int actor_is_side(ACTOR *actor, int side);

int state_is_perpendicular(int state1, int state2);
int state_is_adjacent(int state1, int state2);

/*--------------------------------------------------------------------------*/
/* variables                                                                */
/*--------------------------------------------------------------------------*/

extern ACTOR actors_list[];
extern ACTOR orig_actors_list[];

extern void *floor_sprite;
extern void *cursor_sprite;

extern int state_move_x_step[STATE_MOVE_COUNT],
           state_move_y_step[STATE_MOVE_COUNT];
extern int state_opposite[STATE_MOVE_COUNT];
extern int state_perpendicular[STATE_MOVE_COUNT];
extern int state_adjacent[STATE_MOVE_COUNT];

#endif /* __MY_ACTORS_H */
