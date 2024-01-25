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
#include "misc.h"

/*--------------------------------------------------------------------------*/
/* defines                                                                  */
/*--------------------------------------------------------------------------*/

#define SECONDS(x) ((int)(x * FPS))

#define DISTANCE(x) x
#define HEALTH(x) x
#define DAMAGE(x) x
#define WEAPON(x) x

#define _CM(name) { name, NULL, ACTOR_DARK | ACTOR_CLONE, 0, 0, 0, 0 }
#define _CW(name) { name, NULL, ACTOR_DARK | ACTOR_CLONE | ACTOR_WEAPON, 0, 0, 0, 0 }

/*--------------------------------------------------------------------------*/
/* weapon rules -- common                                                   */
/*--------------------------------------------------------------------------*/

#define SHOOT_FLAGS(s,x)     ACTOR_WEAPON_SHOOT | ACTOR_##s##_##x | ACTOR_##s

#define RECHARGE_TIME_A      SECONDS(0.9)
#define RECHARGE_TIME_B      SECONDS(2.2)
#define RECHARGE_TIME_C      SECONDS(1.6)
#define RECHARGE_TIME_D      SECONDS(1.3)
#define RECHARGE_TIME_E      SECONDS(2.0)
#define RECHARGE_TIME_F      SECONDS(2.7)

#define DAMAGE_MINOR         DAMAGE(20)
#define DAMAGE_MODERATE      HEALTH(25)
#define DAMAGE_GREAT         HEALTH(30)
#define DAMAGE_VERY_GREAT    HEALTH(40)

#define HAND_FLAGS(s,x)      ACTOR_WEAPON_HAND | ACTOR_##s | ACTOR_##s##_##x
#define HAND_RECHARGE        RECHARGE_TIME_A
#define HAND_DAMAGE          DAMAGE_MINOR

#define CLOUD_FLAGS(s)       ACTOR_WEAPON_CLOUD | ACTOR_##s##_CLOUD | ACTOR_##s
#define CLOUD_RECHARGE       RECHARGE_TIME_B
#define CLOUD_DAMAGE         0

#define BOULDER_FLAGS(s)     SHOOT_FLAGS(s,BOULDER) | ACTOR_NORMAL_SPEED
#define BOULDER_RECHARGE     RECHARGE_TIME_B
#define BOULDER_DAMAGE       DAMAGE_GREAT

#define _W(name,pfx) { name, NULL, pfx##_FLAGS, 0, pfx##_DAMAGE, 0, pfx##_RECHARGE }

/*--------------------------------------------------------------------------*/
/* weapon rules -- light                                                    */
/*--------------------------------------------------------------------------*/

#define SWORD_FLAGS          HAND_FLAGS(LIGHT,SWORD)
#define SWORD_RECHARGE       HAND_RECHARGE
#define SWORD_DAMAGE         HAND_DAMAGE

#define FIERYEXPLOSION_FLAGS CLOUD_FLAGS(LIGHT)
#define FIERYEXPLOSION_RECHARGE CLOUD_RECHARGE
#define FIERYEXPLOSION_DAMAGE CLOUD_DAMAGE

#define LIGHT_BOULDER_FLAGS  BOULDER_FLAGS(LIGHT)
#define LIGHT_BOULDER_RECHARGE BOULDER_RECHARGE
#define LIGHT_BOULDER_DAMAGE BOULDER_DAMAGE

#define ARROW_FLAGS          SHOOT_FLAGS(LIGHT,ARROW) | ACTOR_NORMAL_SPEED
#define ARROW_RECHARGE       RECHARGE_TIME_C
#define ARROW_DAMAGE         DAMAGE_MINOR

#define MAGICSPEAR_FLAGS     SHOOT_FLAGS(LIGHT,MAGICSPEAR) | ACTOR_NORMAL_SPEED
#define MAGICSPEAR_RECHARGE  RECHARGE_TIME_C
#define MAGICSPEAR_DAMAGE    DAMAGE_MODERATE

#define ENERGYBOLT_FLAGS     SHOOT_FLAGS(LIGHT,ENERGYBOLT) | ACTOR_FAST_SPEED
#define ENERGYBOLT_RECHARGE  RECHARGE_TIME_D
#define ENERGYBOLT_DAMAGE    DAMAGE_MODERATE

#define WHIRLWIND_FLAGS      SHOOT_FLAGS(LIGHT,WHIRLWIND) | ACTOR_FAST_SPEED
#define WHIRLWIND_RECHARGE   RECHARGE_TIME_E
#define WHIRLWIND_DAMAGE     DAMAGE_MODERATE

#define FIREBALL_FLAGS       SHOOT_FLAGS(LIGHT,FIREBALL) | ACTOR_FAST_SPEED
#define FIREBALL_RECHARGE    RECHARGE_TIME_C
#define FIREBALL_DAMAGE      DAMAGE_GREAT

/*--------------------------------------------------------------------------*/
/* weapon rules -- dark                                                     */
/*--------------------------------------------------------------------------*/

#define CLUB_FLAGS           HAND_FLAGS(DARK,CLUB)
#define CLUB_RECHARGE        HAND_RECHARGE
#define CLUB_DAMAGE          HAND_DAMAGE

#define SCREAM_FLAGS         CLOUD_FLAGS(DARK)
#define SCREAM_RECHARGE      CLOUD_RECHARGE
#define SCREAM_DAMAGE        CLOUD_DAMAGE

#define DARK_BOULDER_FLAGS   BOULDER_FLAGS(DARK)
#define DARK_BOULDER_RECHARGE BOULDER_RECHARGE
#define DARK_BOULDER_DAMAGE  BOULDER_DAMAGE

#define TAILSPIKES_FLAGS     SHOOT_FLAGS(DARK,TAILSPIKES) | ACTOR_NORMAL_SPEED
#define TAILSPIKES_RECHARGE  RECHARGE_TIME_C
#define TAILSPIKES_DAMAGE    DAMAGE_MINOR

#define EYEBEAM_FLAGS        SHOOT_FLAGS(DARK,EYEBEAM) | ACTOR_FAST_SPEED
#define EYEBEAM_RECHARGE     RECHARGE_TIME_D
#define EYEBEAM_DAMAGE       DAMAGE_GREAT

#define FIERYBREATH_FLAGS    SHOOT_FLAGS(DARK,FIERYBREATH) | ACTOR_NORMAL_SPEED
#define FIERYBREATH_RECHARGE RECHARGE_TIME_F
#define FIERYBREATH_DAMAGE   DAMAGE_VERY_GREAT

#define CLONE_FLAGS          ACTOR_WEAPON_CLONE | ACTOR_DARK_CLONE | ACTOR_DARK
#define CLONE_RECHARGE       0
#define CLONE_DAMAGE         0

#define LIGHTNINGBOLT_FLAGS  SHOOT_FLAGS(DARK,LIGHTNINGBOLT) | ACTOR_FAST_SPEED
#define LIGHTNINGBOLT_RECHARGE RECHARGE_TIME_C
#define LIGHTNINGBOLT_DAMAGE DAMAGE_MODERATE

/*--------------------------------------------------------------------------*/
/* weapon rules -- elemental                                                */
/*--------------------------------------------------------------------------*/

#define TORNADO_FLAGS        SHOOT_FLAGS(ELEMENTAL,TORNADO) |  ACTOR_NORMAL_SPEED
#define TORNADO_RECHARGE     RECHARGE_TIME_C
#define TORNADO_DAMAGE       DAMAGE_MODERATE

#define ELEM_BOULDER_FLAGS   BOULDER_FLAGS(ELEMENTAL)
#define ELEM_BOULDER_RECHARGE BOULDER_RECHARGE
#define ELEM_BOULDER_DAMAGE  BOULDER_DAMAGE

#define FIERYBOLT_FLAGS      SHOOT_FLAGS(ELEMENTAL,FIERYBOLT) | ACTOR_FAST_SPEED
#define FIERYBOLT_RECHARGE   RECHARGE_TIME_D
#define FIERYBOLT_DAMAGE     DAMAGE_MINOR

#define ICEBOLT_FLAGS        SHOOT_FLAGS(ELEMENTAL,ICEBOLT) | ACTOR_NORMAL_SPEED
#define ICEBOLT_RECHARGE     RECHARGE_TIME_B
#define ICEBOLT_DAMAGE       DAMAGE_GREAT

/*--------------------------------------------------------------------------*/
/* monster rules                                                            */
/*--------------------------------------------------------------------------*/

#define MONSTER(x,s,m,p)     ACTOR_##s | ACTOR_##m | ACTOR_##x | ACTOR_##p##_SPEED

#define HEALTH_SHORT         HEALTH(30)
#define HEALTH_AVERAGE       HEALTH(50)
#define HEALTH_LONG          HEALTH(60)
#define HEALTH_VERY_LONG     HEALTH(70)

#define _M(name,pfx) { name, NULL, pfx##_FLAGS, pfx##_DISTANCE, pfx##_HEALTH, pfx##_WEAPON, 0 }

/*--------------------------------------------------------------------------*/
/* monster rules -- light                                                   */
/*--------------------------------------------------------------------------*/

#define KNIGHT_FLAGS         MONSTER(KNIGHT,LIGHT,GROUND,NORMAL)
#define KNIGHT_DISTANCE      DISTANCE(3)
#define KNIGHT_HEALTH        HEALTH_SHORT
#define KNIGHT_WEAPON        ACTOR_LIGHT_SWORD

#define ARCHER_FLAGS         MONSTER(ARCHER,LIGHT,GROUND,NORMAL)
#define ARCHER_DISTANCE      DISTANCE(3)
#define ARCHER_HEALTH        HEALTH_SHORT
#define ARCHER_WEAPON        ACTOR_LIGHT_ARROW

#define VALKYRIE_FLAGS       MONSTER(VALKYRIE,LIGHT,FLY,NORMAL)
#define VALKYRIE_DISTANCE    DISTANCE(3)
#define VALKYRIE_HEALTH      HEALTH_AVERAGE
#define VALKYRIE_WEAPON      ACTOR_LIGHT_MAGICSPEAR

#define GOLEM_FLAGS          MONSTER(GOLEM,LIGHT,GROUND,SLOW)
#define GOLEM_DISTANCE       DISTANCE(3)
#define GOLEM_HEALTH         HEALTH_LONG
#define GOLEM_WEAPON         ACTOR_LIGHT_BOULDER

#define UNICORN_FLAGS        MONSTER(UNICORN,LIGHT,GROUND,NORMAL)
#define UNICORN_DISTANCE     DISTANCE(4)
#define UNICORN_HEALTH       HEALTH_AVERAGE
#define UNICORN_WEAPON       ACTOR_LIGHT_ENERGYBOLT

#define DJINNI_FLAGS         MONSTER(DJINNI,LIGHT,FLY,NORMAL)
#define DJINNI_DISTANCE      DISTANCE(4)
#define DJINNI_HEALTH        HEALTH_LONG
#define DJINNI_WEAPON        ACTOR_LIGHT_WHIRLWIND

#define PHOENIX_FLAGS        MONSTER(PHOENIX,LIGHT,FLY,NORMAL)
#define PHOENIX_DISTANCE     DISTANCE(5)
#define PHOENIX_HEALTH       HEALTH_LONG
#define PHOENIX_WEAPON       ACTOR_LIGHT_CLOUD

#define WIZARD_FLAGS         MONSTER(WIZARD,LIGHT,MASTER,NORMAL)
#define WIZARD_DISTANCE      DISTANCE(3)
#define WIZARD_HEALTH        HEALTH_AVERAGE
#define WIZARD_WEAPON        ACTOR_LIGHT_FIREBALL

/*--------------------------------------------------------------------------*/
/* monster rules -- dark                                                    */
/*--------------------------------------------------------------------------*/

#define GOBLIN_FLAGS         MONSTER(GOBLIN,DARK,GROUND,NORMAL)
#define GOBLIN_DISTANCE      DISTANCE(3)
#define GOBLIN_HEALTH        HEALTH_SHORT
#define GOBLIN_WEAPON        ACTOR_DARK_CLUB

#define MANTICORE_FLAGS      MONSTER(MANTICORE,DARK,GROUND,NORMAL)
#define MANTICORE_DISTANCE   DISTANCE(3)
#define MANTICORE_HEALTH     HEALTH_AVERAGE
#define MANTICORE_WEAPON     ACTOR_DARK_TAILSPIKES

#define BANSHEE_FLAGS        MONSTER(BANSHEE,DARK,FLY,NORMAL)
#define BANSHEE_DISTANCE     DISTANCE(3)
#define BANSHEE_HEALTH       HEALTH_AVERAGE
#define BANSHEE_WEAPON       ACTOR_DARK_CLOUD

#define TROLL_FLAGS          MONSTER(TROLL,DARK,GROUND,SLOW)
#define TROLL_DISTANCE       DISTANCE(3)
#define TROLL_HEALTH         HEALTH_LONG
#define TROLL_WEAPON         ACTOR_DARK_BOULDER

#define BASILISK_FLAGS       MONSTER(BASILISK,DARK,GROUND,NORMAL)
#define BASILISK_DISTANCE    DISTANCE(3)
#define BASILISK_HEALTH      HEALTH_SHORT
#define BASILISK_WEAPON      ACTOR_DARK_EYEBEAM

#define DRAGON_FLAGS         MONSTER(DRAGON,DARK,FLY,NORMAL)
#define DRAGON_DISTANCE      DISTANCE(4)
#define DRAGON_HEALTH        HEALTH_VERY_LONG
#define DRAGON_WEAPON        ACTOR_DARK_FIERYBREATH

#define SHAPESHIFTER_FLAGS   MONSTER(SHAPESHIFTER,DARK,FLY,NORMAL)
#define SHAPESHIFTER_DISTANCE DISTANCE(5)
#define SHAPESHIFTER_HEALTH  HEALTH_AVERAGE
#define SHAPESHIFTER_WEAPON  ACTOR_DARK_CLONE

#define SORCERESS_FLAGS      MONSTER(SORCERESS,DARK,MASTER,NORMAL)
#define SORCERESS_DISTANCE   DISTANCE(3)
#define SORCERESS_HEALTH     HEALTH_AVERAGE
#define SORCERESS_WEAPON     ACTOR_DARK_LIGHTNINGBOLT

/*--------------------------------------------------------------------------*/
/* monster rules -- elemental                                               */
/*--------------------------------------------------------------------------*/

#define ELEMENTAL(x,p)       ACTOR_##x##_ELEM | ACTOR_##p##_SPEED | ACTOR_ELEMENTAL

#define ELEM_AIR_FLAGS       ELEMENTAL(AIR,NORMAL)
#define ELEM_AIR_DISTANCE    DISTANCE(0)
#define ELEM_AIR_HEALTH      HEALTH_AVERAGE
#define ELEM_AIR_WEAPON      ACTOR_ELEMENTAL_TORNADO

#define ELEM_EARTH_FLAGS     ELEMENTAL(EARTH,SLOW)
#define ELEM_EARTH_DISTANCE  DISTANCE(0)
#define ELEM_EARTH_HEALTH    HEALTH_LONG
#define ELEM_EARTH_WEAPON    ACTOR_ELEMENTAL_BOULDER

#define ELEM_FIRE_FLAGS      ELEMENTAL(FIRE,NORMAL)
#define ELEM_FIRE_DISTANCE   DISTANCE(0)
#define ELEM_FIRE_HEALTH     HEALTH_AVERAGE
#define ELEM_FIRE_WEAPON     ACTOR_ELEMENTAL_FIERYBOLT

#define ELEM_WATER_FLAGS     ELEMENTAL(WATER,NORMAL)
#define ELEM_WATER_DISTANCE  DISTANCE(0)
#define ELEM_WATER_HEALTH    HEALTH_LONG
#define ELEM_WATER_WEAPON    ACTOR_ELEMENTAL_ICEBOLT

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

static void actors_init_themes(void);
static void actors_load_theme(char *name);
static void actors_load_sprites(void);

/*--------------------------------------------------------------------------*/
/* variables                                                                */
/*--------------------------------------------------------------------------*/

static THEME themes[32];
static int num_themes = 0;

static void (*progress_func)(char *msg, float progress);

ACTOR orig_actors_list[ACTOR_NUM_ACTORS] = {

   { },                                 /* the null actor, actors[0] */

/*--------------------------------------------------------------------------*/
/* weapons                                                                  */
/*--------------------------------------------------------------------------*/

   _W("knight_weapon", SWORD),
   _W("phoenix_weapon", FIERYEXPLOSION),
   _W("golem_weapon", LIGHT_BOULDER),
   _W("archer_weapon", ARROW),
   _W("valkyrie_weapon", MAGICSPEAR),
   _W("unicorn_weapon", ENERGYBOLT),
   _W("djinni_weapon", WHIRLWIND),
   _W("wizard_weapon", FIREBALL),

   _W("air_elem_weapon", TORNADO),
   _W("earth_elem_weapon", ELEM_BOULDER),
   _W("fire_elem_weapon", FIERYBOLT),
   _W("water_elem_weapon", ICEBOLT),

   _W("goblin_weapon", CLUB),
   _W("banshee_weapon", SCREAM),
   _W("troll_weapon", DARK_BOULDER),
   _W("manticore_weapon", TAILSPIKES),
   _W("basilisk_weapon", EYEBEAM),
   _W("dragon_weapon", FIERYBREATH),
   _W("shapeshifter_weapon", CLONE),            /* not really a weapon */
   _W("sorceress_weapon", LIGHTNINGBOLT),

   _CW("knight_weapon"),                        /* light side weapon clones */
   _CW("phoenix_weapon"),
   _CW("golem_weapon"),
   _CW("archer_weapon"),
   _CW("valkyrie_weapon"),
   _CW("unicorn_weapon"),
   _CW("djinni_weapon"),
   _CW("wizard_weapon"),

   _CW("air_elem_weapon"),                      /* elemental weapon clones */
   _CW("earth_elem_weapon"),
   _CW("fire_elem_weapon"),
   _CW("water_elem_weapon"),

/*--------------------------------------------------------------------------*/
/* monsters                                                                 */
/*--------------------------------------------------------------------------*/

   _M("knight", KNIGHT),
   _M("archer", ARCHER),
   _M("valkyrie", VALKYRIE),
   _M("golem", GOLEM),
   _M("unicorn", UNICORN),
   _M("djinni", DJINNI),
   _M("phoenix", PHOENIX),
   _M("wizard", WIZARD),

   _M("air_elem", ELEM_AIR),
   _M("earth_elem", ELEM_EARTH),
   _M("fire_elem", ELEM_FIRE),
   _M("water_elem", ELEM_WATER),

   _M("goblin", GOBLIN),
   _M("manticore", MANTICORE),
   _M("banshee", BANSHEE),
   _M("troll", TROLL),
   _M("basilisk", BASILISK),
   _M("dragon", DRAGON),
   _M("shapeshifter", SHAPESHIFTER),
   _M("sorceress", SORCERESS),

   _CM("knight"),                       /* light side clones */
   _CM("archer"),
   _CM("valkyrie"),
   _CM("golem"),
   _CM("unicorn"),
   _CM("djinni"),
   _CM("phoenix"),
   _CM("wizard"),

   _CM("air_elem"),                     /* elemental clones */
   _CM("earth_elem"),
   _CM("fire_elem"),
   _CM("water_elem"),

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
/* state movement table                                                     */
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
   sprite_add_state(STATE_FIRE_UP, "fire.up");
   sprite_add_state(STATE_FIRE_DOWN, "fire.down");
   sprite_add_state(STATE_FIRE_LEFT, "fire.left");
   sprite_add_state(STATE_FIRE_RIGHT, "fire.right");
   sprite_add_state(STATE_FIRE_UP_LEFT, "fire.upleft");
   sprite_add_state(STATE_FIRE_DOWN_LEFT, "fire.downleft");
   sprite_add_state(STATE_FIRE_UP_RIGHT, "fire.upright");
   sprite_add_state(STATE_FIRE_DOWN_RIGHT, "fire.downright");
   sprite_add_state(STATE_FIRE_ANY, "fire.any");

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
   actors_init_themes();
}

/*--------------------------------------------------------------------------*/
/* actors_init_themes                                                       */
/*--------------------------------------------------------------------------*/

void actors_init_themes(void)
{
   char *path, str[128];
   FILE *fp;

   path = malloc(PATH_MAX);
   sprintf(path, "%s/THEMES", DATADIR);
   fp = fopen(path, "r");
   if (fp == NULL) {
      fprintf(stderr, "Could not open `%s'.  Giving up.\n", path);
      exit(EXIT_FAILURE);
   }
   free(path);

   while (!feof(fp)) {
      fgets(str, 127, fp);
      str[strlen(str) - 1] = 0;         /* wipe out the '\n' */
      if (str[0] == 0)
         continue;
      if (strcmp(str, "end") == 0)
         break;
      themes[num_themes].name = strdup(str);
      fgets(str, 127, fp);
      str[strlen(str) - 1] = 0;         /* wipe out the '\n' */
      themes[num_themes].descr = strdup(str);
      fgets(str, 127, fp);
      str[strlen(str) - 1] = 0;         /* wipe out the '\n' */
      themes[num_themes].author = strdup(str);
      num_themes++;
   }
   fclose(fp);

   if (num_themes == 0) {
      fprintf(stderr, "There are no defined themes.  Giving up.\n");
      exit(EXIT_FAILURE);
   }
   actors_load_theme(themes[0].name);
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
   free(path);

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

   sprite_select_theme(name);
   actors_load_sprites();
}

/*--------------------------------------------------------------------------*/
/* actors_load_sprites                                                      */
/*--------------------------------------------------------------------------*/

void actors_load_sprites(void)
{
   ACTOR *actor;
   char name[64];
   char msg[128];
   int step, steps;

   steps = 0;
   for (actor = &actors_list[1]; actor->name[0] != 0; actor++)
      steps++;
   step = 0;
   for (actor = &actors_list[1]; actor->name[0] != 0; actor++) {
      if ((actor->type & (ACTOR_ELEMENTAL | ACTOR_WEAPON)) == ACTOR_ELEMENTAL)
         sprintf(name, "actors/elemental/%s", actor->name);
      else
         sprintf(name, "%s/%s/%s",
                 (actor->type & ACTOR_WEAPON) ? "weapons" : "actors",
                 (actor->type & ACTOR_CLONE) ? "clone" :
                    (actor->type & ACTOR_ELEMENTAL) ? "elemental" :
                       (actor->type & ACTOR_LIGHT) ? "light" : "dark",
                 actor->name);
      if (progress_func != NULL) {
         sprintf(msg, "Loading `%s'", name);
         progress_func(msg, (float)step / steps);
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
   return ((actor->type & ACTOR_LIGHT) == (!side) * ACTOR_LIGHT);
}

/****************************************************************************/
/*                                                                          */
/* GTK Stuff                                                                */
/*                                                                          */
/****************************************************************************/

#include <gtk/gtk.h>
#include "gtk-callbacks.h"
#include "gtk-interface.h"
#include "gtk-support.h"

/*--------------------------------------------------------------------------*/
/* functions                                                                */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* variables                                                                */
/*--------------------------------------------------------------------------*/

static GtkWidget *window;

/*--------------------------------------------------------------------------*/
/* select_theme                                                             */
/*--------------------------------------------------------------------------*/

void actors_select_theme(GtkMenuItem *menuitem, gpointer user_data)
{
   GtkWidget *list, *widget;
   int i;

   window = create_theme_window();

   list = lookup_widget(window, "list");
   for (i = 0; i < num_themes; i++) {
      widget = gtk_list_item_new_with_label(themes[i].name);
      gtk_container_add(GTK_CONTAINER(list), widget);
      gtk_widget_show(widget);
      gtk_object_set_data(GTK_OBJECT(widget), "theme", &themes[i]);
   }
   gtk_widget_show(GTK_WIDGET(window));
}

/*--------------------------------------------------------------------------*/
/* actors_list_selection_changed                                            */
/*--------------------------------------------------------------------------*/

void actors_list_selection_changed(GtkList *list, gpointer data)
{
   GtkWidget *descr, *author, *widget;
   GList *item;
   THEME *theme;

   descr = lookup_widget(window, "descr");
   author = lookup_widget(window, "author");

   widget = lookup_widget(window, "list");
   item = GTK_LIST(list)->selection;
   if (item == NULL) {
      gtk_entry_set_text(GTK_ENTRY(descr), "");
      gtk_entry_set_text(GTK_ENTRY(author), "");
      return;
   }

   theme = gtk_object_get_data(GTK_OBJECT(item->data), "theme");
   gtk_entry_set_text(GTK_ENTRY(descr), theme->descr);
   gtk_entry_set_text(GTK_ENTRY(author), theme->author);
}

/*--------------------------------------------------------------------------*/
/* actors_ok_clicked                                                        */
/*--------------------------------------------------------------------------*/

void actors_ok_clicked(GtkButton *button, gpointer data)
{
   GtkWidget *widget;
   GList *item;
   THEME *theme;

   widget = lookup_widget(window, "list");
   item = GTK_LIST(widget)->selection;
   if (item != NULL)
      theme = gtk_object_get_data(GTK_OBJECT(item->data), "theme");
   else
      theme = NULL;
   gtk_widget_destroy(window);
   if (theme != NULL)
      actors_load_theme(theme->name);
}

/*--------------------------------------------------------------------------*/
/* actors_cancel_clicked                                                    */
/*--------------------------------------------------------------------------*/

void actors_cancel_clicked(GtkObject *object, gpointer data)
{
   gtk_widget_destroy(window);
}
