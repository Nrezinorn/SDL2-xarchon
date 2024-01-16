#include "Xarchon_Game.hpp"

Xarchon_Game::Xarchon_Game(void) : seq(3,10,3,10)
{
  state=NULL;
}

Xarchon_Game::~Xarchon_Game(void)
{
  if (state!=NULL)
    delete state;
}

void Xarchon_Game::SetPlayer(Xarchon_Goal_Test *light,Xarchon_Goal_Test *dark)
{
  for (int i=0;i<TOTAL_VALUE_PARMS;i++) {
    ((Xarchon_Goal_Test *)seq.light->gt)->value_parm[i]=light->value_parm[i];
    ((Xarchon_Goal_Test *)seq.dark->gt )->value_parm[i]=dark->value_parm[i];
  }
}

void Xarchon_Game::InitStart(int light_first,int init[BOARD_YCELLS][BOARD_XCELLS])
{
  if (state!=NULL)
    delete state;
  state=new Xarchon_State;
  state->side = !light_first;
  state->lumi   = light_first ? LUMI_LIGHTEST : LUMI_DARKEST;
  state->lumi_d = light_first ? -1            : 1;
  state->turn_num=0;
  state->fight=false;
  /* setup actors on the board */
  if (init==NULL)
    init=init_board_cells[0];
  for (int y = 0; y < BOARD_YCELLS; y++) {
    for (int x = 0; x < BOARD_XCELLS; x++) {
      CELL *cell = &state->board_cells[y][x];
      cell->flags = init[y][x];
      if ((cell->flags & ACTOR_MASK) != 0) 
	cell->actor=state->Actor_Copy(&orig_actors_list[cell->flags & ACTOR_MASK]);
      else
	cell->actor = NULL;
    }
  }
}

void Xarchon_Game::Display(void)
{
  for (int y = 0; y < BOARD_YCELLS; y++) {
    printf ("\n");
    for (int x = 0; x < BOARD_XCELLS; x++) {
      CELL *cell = &state->board_cells[y][x];
      if (cell->actor!=NULL) {
	if (cell->actor->type & ACTOR_LIGHT)
	  printf("%c",*cell->actor->name);
	else
	  printf("%c",*cell->actor->name-32);
      }
      else 
	printf (" ");
    }
  }
  printf ("\nTurn Number %d \n",state->turn_num);
}

void Xarchon_Game::Play(void)
{
  while (!CheckEnd()) {
    Display();
    Move();
  }
}

void Xarchon_Game::Move(void)
{
  Xarchon_Strategy *strategy=(Xarchon_Strategy *)seq.Next_Strategy(state);
  strategy->InitState(state);
  Operator<Xarchon_State> *oper=strategy->Next();
  Xarchon_State *next_state=oper->Operate(state);
  delete oper;
  delete state;
  state=next_state;
  if (state->fight)
    Move();
}


bool Xarchon_Game::CheckEnd(void)
{
  int x, y;
  CELL *cell;
  int num_light = 0, num_dark = 0;
  int num_light_pp = 0, num_dark_pp = 0;
  int num_pp = 0;

  if (state->fight)
    return false;
  
  for (y = 0; y < BOARD_YCELLS; y++) {
    for (x = 0; x < BOARD_XCELLS; x++) {
      cell = &state->board_cells[y][x];
      if (cell->flags & CELL_POWER) {
	num_pp++;
	if (cell->actor!=NULL) {
	  if ( (cell->actor->type & ACTOR_LIGHT) )
	    num_light_pp++;
	  else
	    num_dark_pp++;
	}
      }
      if (cell->actor != NULL) {
	if ( (state->board_cells[y][x].flags & CELL_IMPRISON)==0) {
	  if ( (cell->actor->type & ACTOR_LIGHT) )
	    num_light++;
	  else
	    num_dark++;
	}
      }
    }
  }

  if (num_light==0 || num_dark==0)
    return true;
  if (num_light_pp==num_pp || num_dark_pp==num_pp)
    return true;
  
  if (state->turn_num>=10)
    return true;
  return false;
}
