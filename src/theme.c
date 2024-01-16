/*--------------------------------------------------------------------------*/
/* theme                                                                    */
/*--------------------------------------------------------------------------*/

#include <config.h>

#include <string.h>

#include "theme.h"
#include "actors.h"

/*--------------------------------------------------------------------------*/
/* variables                                                                */
/*--------------------------------------------------------------------------*/

static THEME themes[] = {
   {
      "archon",
      "Modelled after the original ARCHON",
      "Matt Kimball, Dan LaPine, Mark Shoulson, Dan Hursh"
   },
   { NULL }                             /* indicator */
};

static char selected_theme[64] = "";

static void (*progress_func)(char *msg, float progress);

/*--------------------------------------------------------------------------*/
/* theme_init                                                               */
/*--------------------------------------------------------------------------*/

void theme_init(void (*_progress_func)(char *msg, float progress))
{
   progress_func = _progress_func;
   actors_init(progress_func);
   theme_select(themes[0].name);
}

/*--------------------------------------------------------------------------*/
/* theme_select                                                             */
/*--------------------------------------------------------------------------*/

void theme_select(char *name)
{
   if (strcmp(name, selected_theme) == 0)
      progress_func("Selected theme is already the current theme", 1.0);
   else {
      strcpy(selected_theme, name);
      actors_load_theme(name);
      progress_func("Theme has been successfully loaded", 1.0);
   }
}

/*--------------------------------------------------------------------------*/
/* theme_get_themes                                                         */
/*--------------------------------------------------------------------------*/

THEME *theme_get_themes(void)
{
   return themes;
}
