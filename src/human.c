/*--------------------------------------------------------------------------*/
/* human interface support                                                  */
/*--------------------------------------------------------------------------*/

#include <config.h>

#ifdef HAVE_LINUX_JOYSTICK_H
#include <fcntl.h>
#include <math.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/joystick.h>
#endif

#include <stdlib.h>

#include "main.h"
#include "human.h"
#include "actors.h"
#include "canvas.h"
#include "board.h"
#include "field.h"
#include "toolkit.h"

/*--------------------------------------------------------------------------*/
/* defines                                                                  */
/*--------------------------------------------------------------------------*/

#define JOY_MAX_DEV     HUMAN_JOY_MAX_DEV
#define MOUSE_RELATIVE  (JOY_MAX_DEV + 0)
#define MOUSE_ABSOLUTE  (JOY_MAX_DEV + 1)
#ifdef HAVE_LINUX_JOYSTICK_H
#define JOY_AXIS_MIN    -32768
#define JOY_AXIS_MAX    32767
#define JOY_MAX_AXES    2
#define JOY_MAX_BUTS    2
#endif

/*--------------------------------------------------------------------------*/
/* structures                                                               */
/*--------------------------------------------------------------------------*/

#ifdef HAVE_LINUX_JOYSTICK_H
typedef struct {
   int fd;                              /* must be first field here */
   int axes[JOY_MAX_AXES];
   int buts[JOY_MAX_BUTS];
} JOY_DATA;
#endif

/*--------------------------------------------------------------------------*/
/* functions                                                                */
/*--------------------------------------------------------------------------*/

#ifdef HAVE_LINUX_JOYSTICK_H
void human_joystick_update(int num);
void human_joystick_frame(int *keys_down);
#endif

void human_mouse_frame(int *keys_down);

/*--------------------------------------------------------------------------*/
/* variables                                                                */
/*--------------------------------------------------------------------------*/

static int human_side;
static int human_turn_mode;
static HUMAN_CONFIG *configs[2];

#ifdef HAVE_LINUX_JOYSTICK_H
static JOY_DATA joy_data[JOY_MAX_DEV] = {
   { -1 }, { -1 }, { -1 }, { -1 }       /* set joy_data.fd to -1 in each */
};
#endif

/*--------------------------------------------------------------------------*/
/* human_joystick_init                                                      */
/*--------------------------------------------------------------------------*/

int human_joystick_init(int num)
{
#ifdef HAVE_LINUX_JOYSTICK_H
   char dev[32];

   if (joy_data[num].fd == -1) {
      sprintf(dev, "/dev/js%d", num);
      joy_data[num].fd = open(dev, O_RDONLY);
      if (joy_data[num].fd < 0)
         return 0;
      fcntl(joy_data[num].fd, F_SETFL, O_NONBLOCK);
      /*
      ioctl(joy_data[num].fd, JSIOCGAXES, &joy_data[num].num_axes);
      ioctl(joy_data[num].fd, JSIOCGBUTTONS, &joy_data[num].num_buts);
      */
   }
   return 1;
#else
   return 0;
#endif /* HAVE_LINUX_JOYSTICK_H */
}

/*--------------------------------------------------------------------------*/
/* human_joystick_update                                                    */
/*--------------------------------------------------------------------------*/

#ifdef HAVE_LINUX_JOYSTICK_H

void human_joystick_update(int num)
{
   struct js_event js;
   int rc;

   while (1) {
      rc = read(joy_data[num].fd, &js, sizeof(struct js_event));
      if (rc != sizeof(struct js_event))
         break;
      switch (js.type & 3) {
         case JS_EVENT_AXIS:
            if (js.number < JOY_MAX_AXES)
               joy_data[num].axes[js.number] = js.value;
            break;
         case JS_EVENT_BUTTON:
            if (js.number < JOY_MAX_BUTS)
               joy_data[num].buts[js.number] = js.value;
            break;
      }
   }
}

#endif /* HAVE_LINUX_JOYSTICK_H */

/*--------------------------------------------------------------------------*/
/* human_start                                                              */
/*--------------------------------------------------------------------------*/

void human_start(int side, HUMAN_CONFIG *config)
{
   configs[side] = config;
#ifdef HAVE_LINUX_JOYSTICK_H
   if (config->non_keyboard >= 0 && config->non_keyboard < JOY_MAX_DEV)
      if (!human_joystick_init(config->non_keyboard))
         config->non_keyboard = -1;
#endif
   if (config->non_keyboard >= MOUSE_RELATIVE)
      canvas_mouse_init(config->non_keyboard - MOUSE_RELATIVE);
}

/*--------------------------------------------------------------------------*/
/* human_turn                                                               */
/*--------------------------------------------------------------------------*/

void human_turn(int side, int mode, COMMAND *cmd)
{
   human_side = side;
   human_turn_mode = mode;
}

/*--------------------------------------------------------------------------*/
/* human_joystick_frame                                                     */
/*--------------------------------------------------------------------------*/

#ifdef HAVE_LINUX_JOYSTICK_H

void human_joystick_frame(int *keys_down)
{
   int joy;
   int directions[8] = {
      STATE_MOVE_RIGHT, STATE_MOVE_UP_RIGHT,
      STATE_MOVE_UP, STATE_MOVE_UP_LEFT,
      STATE_MOVE_LEFT, STATE_MOVE_DOWN_LEFT,
      STATE_MOVE_DOWN, STATE_MOVE_DOWN_RIGHT
   };
   int radius;
   double angle;

   joy = configs[human_side]->non_keyboard;
   if (joy >= 0 && joy < JOY_MAX_DEV && joy_data[joy].fd != -1) {
      human_joystick_update(joy);
      if (joy_data[joy].buts[0] || joy_data[joy].buts[1])
         keys_down[STATE_FIRE] = 1;
      radius = (joy_data[joy].axes[0]) * (joy_data[joy].axes[0])
             + (joy_data[joy].axes[1]) * (joy_data[joy].axes[1]);
      if (radius > (JOY_AXIS_MAX / 4) * (JOY_AXIS_MAX / 4)) {
         angle = atan2(-joy_data[joy].axes[1], joy_data[joy].axes[0]);
         keys_down[directions[(int)(rint(angle / M_PI_4) + 16) % 8]] = 1;
      }
   }
}

#endif /* HAVE_LINUX_JOYSTICK_H */

/*--------------------------------------------------------------------------*/
/* human_mouse_frame                                                        */
/*--------------------------------------------------------------------------*/

void human_mouse_frame(int *keys_down)
{
   int mouse_drag_direction;
   int dx, dy;
   int grad;

   if (configs[human_side]->non_keyboard >= MOUSE_RELATIVE) {
      if (canvas_mouse_get(&dx, &dy))
         keys_down[STATE_FIRE] = 1;
      if (configs[human_side]->non_keyboard == MOUSE_ABSOLUTE) {
         switch (human_turn_mode) {
            case IFACE_BOARD:
               board_absolute_control_delta(&dx, &dy);
               break;
            case IFACE_FIELD:
               field_absolute_control_delta(&dx, &dy);
               break;
         }
      }
      if (dx == 0) {
         if (dy > 0)
            mouse_drag_direction = STATE_MOVE_DOWN;
         else if (dy < 0)
            mouse_drag_direction = STATE_MOVE_UP;
         else
            mouse_drag_direction = 0; /* there's no mouse movement */
      } else {
         grad = 2 * dy / dx;
         if (grad >= 4) {
            if (dx > 0)
               mouse_drag_direction = STATE_MOVE_DOWN;
            else
               mouse_drag_direction = STATE_MOVE_UP;
         } else if (grad >= 1) {
            if (dx > 0)
               mouse_drag_direction = STATE_MOVE_DOWN_RIGHT;
            else
               mouse_drag_direction = STATE_MOVE_UP_LEFT;
         } else if (grad >= -1) {
            if (dx > 0)
               mouse_drag_direction = STATE_MOVE_RIGHT;
            else
               mouse_drag_direction = STATE_MOVE_LEFT;
         } else {
            if (dx > 0)
               mouse_drag_direction = STATE_MOVE_UP_RIGHT;
            else
               mouse_drag_direction = STATE_MOVE_DOWN_LEFT;
         }
      }
      if (mouse_drag_direction)
         keys_down[mouse_drag_direction] = 1;
   }
}

/*--------------------------------------------------------------------------*/
/* human_frame                                                              */
/*--------------------------------------------------------------------------*/

void human_frame(int *keys_down)
{
   int i;

   for (i = 0; i < STATE_MOVE_COUNT; i++)
      if (configs[human_side]->keys[human_side][i])
         keys_down[i] =
               canvas_key_down(configs[human_side]->keys[human_side][i]);
      else
         keys_down[i] = 0;
   if (keys_down[STATE_MOVE_UP] && keys_down[STATE_MOVE_LEFT])
      keys_down[STATE_MOVE_UP_LEFT] = 1;
   if (keys_down[STATE_MOVE_DOWN] && keys_down[STATE_MOVE_LEFT])
      keys_down[STATE_MOVE_DOWN_LEFT] = 1;
   if (keys_down[STATE_MOVE_UP] && keys_down[STATE_MOVE_RIGHT])
      keys_down[STATE_MOVE_UP_RIGHT] = 1;
   if (keys_down[STATE_MOVE_DOWN] && keys_down[STATE_MOVE_RIGHT])
      keys_down[STATE_MOVE_DOWN_RIGHT] = 1;

#ifdef HAVE_LINUX_JOYSTICK_H
   human_joystick_frame(keys_down);
#endif

   human_mouse_frame(keys_down);
}

/*--------------------------------------------------------------------------*/
/* human_config_read                                                        */
/*--------------------------------------------------------------------------*/

void human_config_read(FILE *fp, HUMAN_CONFIG *config)
{
   int i, j;

   for (i = 0; i < 2; i++)
      for (j = 0; j < STATE_MOVE_COUNT; j++)
         fscanf(fp, "%lX", &config->keys[i][j]);
   fscanf(fp, "%d", &config->non_keyboard);
}

/*--------------------------------------------------------------------------*/
/* human_config_write                                                       */
/*--------------------------------------------------------------------------*/

void human_config_write(FILE *fp, HUMAN_CONFIG *config)
{
   int i, j;

   for (i = 0; i < 2; i++) {
      for (j = 0; j < STATE_MOVE_COUNT; j++)
         fprintf(fp, "0x%lX ", config->keys[i][j]);
      fprintf(fp, "\n");
   }
   fprintf(fp, "%d\n", config->non_keyboard);
}
