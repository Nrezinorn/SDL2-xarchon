/*--------------------------------------------------------------------------*/
/* main                                                                     */
/*--------------------------------------------------------------------------*/

#ifndef __MY_MAIN_H
#define __MY_MAIN_H

/*--------------------------------------------------------------------------*/
/* defines                                                                  */
/*--------------------------------------------------------------------------*/

#define GAME_ARCHON     0
#define GAME_ADEPT      1
#define NUM_GAMES       2

#define CELL_XSIZE      32
#define CELL_YSIZE      32
/* if CELL_XSIZE and/or CELL_YSIZE are powers of 2, we can take advantage */
/* of that and use a simple & to get the remainder. */
#define CELL_XOFFSET(x)	((x)&(CELL_XSIZE - 1))
#define CELL_YOFFSET(y)	((y)&(CELL_YSIZE - 1))
/* Otherwise, we have to use these more expensive versions
#define CELL_XOFFSET(x)	((x) % CELL_XSIZE)
#define CELL_YOFFSET(y)	((y) % CELL_YSIZE)
*/

#define CANVAS_XCELLS   19
#define CANVAS_YCELLS   13
#define CANVAS_WIDTH    (CELL_XSIZE * CANVAS_XCELLS)
#define CANVAS_HEIGHT   (CELL_YSIZE * CANVAS_YCELLS)

#define CELL_X(x) ((x) * CELL_XSIZE)
#define CELL_Y(y) ((y) * CELL_YSIZE)

#define FPS             40

#define HEALTH_SCALE    (CANVAS_HEIGHT/24)
        /* scale of health & damage from original Archon to XArchon */
#define TIME_SCALE      (FPS/12.5)
        /* scale of frame speed from original Archon (12.5 fps) to XArchon */

/*--------------------------------------------------------------------------*/
/* useful macros                                                            */
/*--------------------------------------------------------------------------*/

#define vmax(a,b) ((a) > (b) ? (a) : (b))
#define vmin(a,b) ((a) < (b) ? (a) : (b))
#define vabs(a)   (((a) < 0) ? -(a) : (a))

/*--------------------------------------------------------------------------*/
/* functions                                                                */
/*--------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

int main_random(void);
void main_expose_event(void);
void main_destroy_event(void);
void main_new_game(int game_type);
void main_game_unpause(void);
void main_game_stop(void);
void main_toggle_sound(void);
void main_game_exit(void);
int main_handle_events(int in_board_game);
void main_config_read(void);
void main_config_write(void);

#ifdef __cplusplus
}
#endif

/*--------------------------------------------------------------------------*/
/* variables                                                                */
/*--------------------------------------------------------------------------*/

extern int random_index;

#endif /* __MY_MAIN_H */
