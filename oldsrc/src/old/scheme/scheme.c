/*--------------------------------------------------------------------------*/
/* scheme interface support                                                 */
/*--------------------------------------------------------------------------*/

#include <guile/gh.h>

#include "scheme.h"
#include "actors.h"

/*--------------------------------------------------------------------------*/
/* functions                                                                */
/*--------------------------------------------------------------------------*/

static SCM scheme_make_board(void);
extern int main(int argc, char *argv[]);

/*--------------------------------------------------------------------------*/
/* variables                                                                */
/*--------------------------------------------------------------------------*/

static char *dir_names[STATE_MOVE_COUNT] = {
   "", "up", "down", "left", "right",
   "upleft", "downleft", "upright", "downright"
};

static COMMAND *command;

/*--------------------------------------------------------------------------*/
/* scheme_init                                                              */
/*--------------------------------------------------------------------------*/

void scheme_init(int argc, char *argv[])
{
   static int first = 1;
   char str[64];

   if (first) {
      first = 0;
      gh_enter(0, NULL, scheme_init);
      return;
   }
   gh_eval_file(LIBDIR"/scheme/init.scm");
   main(argc, argv);
}

/*--------------------------------------------------------------------------*/
/* scheme_start                                                             */
/*--------------------------------------------------------------------------*/

void *scheme_start(int which, char *name)
{
   char tmp[PATH_MAX];
   SCM scm;

   sprintf(tmp, "%s/scheme/%s.scm", LIBDIR, name);
   gh_eval_file(tmp);
   sprintf(tmp, "(%s-start '%s)", name, (which == 0) ? "light" : "dark");
   scm = gh_eval_str(tmp);
   return (void *)scm;
}

/*--------------------------------------------------------------------------*/
/* scheme_turn                                                              */
/*--------------------------------------------------------------------------*/

void scheme_turn(void *data, int mode, COMMAND *cmd)
{
   command = cmd;
   if (mode == IFACE_BOARD) {
      gh_call1(gh_car((SCM)data), scheme_make_board());
      
      cmd->b.spell = 0;
      cmd->b.x1 = 7;
      cmd->b.y1 = 0;
      cmd->b.x2 = 6;
      cmd->b.y2 = 0;
   }
   else {
      gh_call0(gh_cadr((SCM)data));
      cmd->f.dir = 1;
      cmd->f.fire = 0;
   }
}

/*--------------------------------------------------------------------------*/
/* scheme_make_board                                                        */
/*--------------------------------------------------------------------------*/

SCM scheme_make_board(void)
{
   int x, y;

   for (y = 0; y < BOARD_YCELLS; y++)
      for (x = 0; x < BOARD_XCELLS; x++) {
      }
   }
}

/*--------------------------------------------------------------------------*/
/* scheme_make_actor                                                        */
/*--------------------------------------------------------------------------*/

SCM scheme_make_actor(ACTOR *a)
{
   SCM scm_actor;

   scm_actor = gh_eval_str("
}

#if 0
void iface_scheme_turn(int mode, COMMAND *cmd)
{
   char str[64];
   SCM scm;
   char *dir_name;

   sprintf(str, "(%s \"turn\" \'(\"%s\"))",
           (char *)side->data, (mode == IFACE_BOARD ? "board" : "field"));
   scm = gh_eval_str(str);
   if (mode == IFACE_BOARD) {
      cmd->b.spell = SCM_INUM(gh_car(scm));
      cmd->b.x1 = SCM_INUM(gh_cadr(scm));
      cmd->b.y1 = SCM_INUM(gh_caddr(scm));
      cmd->b.x2 = SCM_INUM(gh_caddr(gh_cdr(scm)));
      cmd->b.y2 = SCM_INUM(gh_caddr(gh_cddr(scm)));
   } else {
      dir_name = SCM_CHARS(gh_car(scm));
      for (cmd->f.dir = STATE_MOVE_LAST; cmd->f.dir >= STATE_MOVE_FIRST; cmd->f.dir--)
         if (strcmp(dir_names[cmd->f.dir], dir_name) == 0)
            break;
      cmd->f.fire = SCM_NFALSEP(gh_cadr(scm));
   }
}
#endif
