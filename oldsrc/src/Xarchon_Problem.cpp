#include "Xarchon_Problem.hpp"
#include "Xarchon_Interface.hpp"

Xarchon_State::Xarchon_State(void) 
{
  fight=false;
  for (int i=0;i<BOARD_YCELLS;i++) {
    for (int j=0;j<BOARD_XCELLS;j++) {
      board_cells[i][j].actor=NULL;
    }
  }
  attacker=NULL;
  elemental=false;
}

Xarchon_State::Xarchon_State(Xarchon_State *a)
{
#ifdef PROFILE
  static long value=0;
  static int counter=0;
  static struct timeval timeval_s,timeval_e;
  gettimeofday(&timeval_s,NULL);
#endif
  side=a->side;
  turn_num=a->turn_num;
  lumi=a->lumi;
  lumi_d=a->lumi_d;
  fight=a->fight;
  x=a->x;
  y=a->y;
  elemental=a->elemental;
  attacker=Actor_Copy(a->attacker);
  for (int i=0;i<BOARD_YCELLS;i++) {
    for (int j=0;j<BOARD_XCELLS;j++) {
      board_cells[i][j].flags=a->board_cells[i][j].flags;
      board_cells[i][j].rocks=a->board_cells[i][j].rocks;
      board_cells[i][j].actor=Actor_Copy(a->board_cells[i][j].actor);
    }
  }

#ifdef PROFILE
  gettimeofday(&timeval_e,NULL);
  value+=(timeval_e.tv_sec-timeval_s.tv_sec)*1000000+(timeval_e.tv_usec-timeval_s.tv_usec);
  counter++;
  printf ("Xarchon_State *  (%ld,%d)\n",value,counter);
#endif
}

Xarchon_State::~Xarchon_State()
{
  for (int i=0;i<BOARD_YCELLS;i++) {
    for (int j=0;j<BOARD_XCELLS;j++) {
      ACTOR *actor=board_cells[i][j].actor;
      if (actor!=NULL)
	delete actor;
    }
  }
  if (attacker!=NULL)
    delete attacker;
}
  
ACTOR *Xarchon_State::Actor_Copy(ACTOR *a)
{
  if (a==NULL)
    return NULL;
  ACTOR *ret=new ACTOR;
  strcpy(ret->name,a->name);
  ret->type=a->type;
  ret->distance=a->distance;
  ret->strength=a->strength;
  ret->weapon=a->weapon;
  ret->recharge=a->recharge;
  return ret;
}

void Xarchon_State::Get_Real_State(void)
{
  int i,j;
  fight=false;
  elemental=false;
  attacker=NULL;
  board_get_data(&side,&turn_num,&lumi,&lumi_d);
  for (i=0;i<BOARD_YCELLS;i++) {
    for (j=0;j<BOARD_XCELLS;j++) {
      board_cells[i][j].flags=::board_cells[i][j].flags;
      board_cells[i][j].actor=Actor_Copy(::board_cells[i][j].actor);
    }
  }
}

void Xarchon_State::UpdateTurn(void) 
{
#ifdef PROFILE
  static long value=0;
  static int counter=0;
  static struct timeval timeval_s,timeval_e;
  gettimeofday(&timeval_s,NULL);
#endif

  side=!side;
  turn_num++;
  lumi+=lumi_d;
  if (lumi == LUMI_DARKEST || lumi == LUMI_LIGHTEST)
    lumi_d = -lumi_d;
  
  // Imprison + Health
  for (y = 0; y < BOARD_YCELLS; y++) {
    for (x = 0; x < BOARD_XCELLS; x++) {
      if (board_cells[y][x].actor != NULL) {
	CELL *cell=&board_cells[y][x];
	ACTOR *actor = cell->actor;
	/* un-imprison light actor when luminance cycle is lightest; */
	/* un-imprison dark actor when luminance cycle is darkest.   */
	if (cell->flags & CELL_IMPRISON &&
	    ((lumi == LUMI_LIGHTEST && actor->type & ACTOR_LIGHT) ||
	     (lumi == LUMI_DARKEST && !(actor->type & ACTOR_LIGHT))))
	  cell->flags ^= CELL_IMPRISON;
	/* heal actors, twice as fast if on a power point */
	actor->strength = vmin(
            (int) orig_actors_list[actor->type & ACTOR_MASK].strength,
            actor->strength +
            (((cell->flags & CELL_POWER) == CELL_POWER) ? 2:1));
	cell->flags=(cell->flags & ~CELL_LUMI_MASK) | lumi;
      }
    }
  }
#ifdef PROFILE
  gettimeofday(&timeval_e,NULL);
  value+=(timeval_e.tv_sec-timeval_s.tv_sec)*1000000+(timeval_e.tv_usec-timeval_s.tv_usec);
  counter++;
  printf ("Update Turn (%ld,%d)\n",value,counter);
#endif

}

int Xarchon_State::EndOfGame(void)
{
  if (fight)
    return 0;

  int num_power=0,light_power=0,dark_power=0,total_light=0,total_dark=0;

  /* Calculate Physical Value */
  /* ************************ */
  for (int i=0;i<BOARD_YCELLS;i++) {
    for (int j=0;j<BOARD_XCELLS;j++) {
      CELL *cell=&board_cells[i][j];
      if (cell->flags & CELL_POWER)
	num_power++;
      if (board_cells[i][j].actor!=NULL) {
	ACTOR *actor=cell->actor;
	if (actor_is_side(actor,0)) {
	  if (cell->flags & CELL_POWER) 
	    light_power++;
	  total_light++;
	}
	if (actor_is_side(actor,1)) {
	  if (cell->flags & CELL_POWER) 
	    dark_power++;
	  total_dark++;
	}
      }
    }
  }

  /* Winning Bonus */
  /* ************* */

  if (total_light==0 || dark_power==num_power)
    return -1;
  if (total_dark==0 || light_power==num_power)
    return 1;
  return 0;
}


Xarchon_Move_Operator::Xarchon_Move_Operator(int s,int x2,int y2,int x1,int y1)
{
  cmd.b.spell=s;
  cmd.b.x2=x2;
  cmd.b.y2=y2;
  cmd.b.x1=x1;
  cmd.b.y1=y1;
}

Xarchon_State* Xarchon_Move_Operator::Operate(Xarchon_State *a) 
{ 
  // All spells are not implemented in this version - maybe some other version.
  Xarchon_State *newstate=new Xarchon_State(a);
  switch (cmd.b.spell) {
  case 0 : 
    newstate->attacker=newstate->board_cells[cmd.b.y1][cmd.b.x1].actor;
    newstate->board_cells[cmd.b.y1][cmd.b.x1].actor=NULL;
    if (newstate->board_cells[cmd.b.y2][cmd.b.x2].actor==NULL) {
      newstate->board_cells[cmd.b.y2][cmd.b.x2].actor=newstate->attacker;
      newstate->attacker=NULL;
      newstate->UpdateTurn();
    }
    else {
      newstate->fight=true;
      newstate->x=cmd.b.x2;
      newstate->y=cmd.b.y2;
    }
    return newstate;
  }
  return newstate;
}

Xarchon_Fight_Operator::Xarchon_Fight_Operator(bool w,int d)
{
  win=w;
  damage=d;
}

Xarchon_State *Xarchon_Fight_Operator::Operate(Xarchon_State *a)
{
  Xarchon_State *newstate=new Xarchon_State(a);
  newstate->fight=false;
  if (win) {
    delete newstate->board_cells[newstate->y][newstate->x].actor;
    if (newstate->elemental) {
      newstate->board_cells[newstate->y][newstate->x].actor=NULL;
      newstate->elemental=false;
    }
    else {
      newstate->board_cells[newstate->y][newstate->x].actor=newstate->attacker;
      newstate->board_cells[newstate->y][newstate->x].actor->strength-=damage;
      if (newstate->board_cells[newstate->y][newstate->x].actor->strength<1)
	newstate->board_cells[newstate->y][newstate->x].actor->strength=1;
    }
  }
  else {
    newstate->board_cells[newstate->y][newstate->x].actor->strength-=damage;
    if (newstate->board_cells[newstate->y][newstate->x].actor->strength<1)
      newstate->board_cells[newstate->y][newstate->x].actor->strength=0;
    delete newstate->attacker;
  }
  newstate->attacker=NULL;
  newstate->UpdateTurn();
  return newstate;
}


Xarchon_Op_Generator::Xarchon_Op_Generator(void)
{
  Clear();
}

Xarchon_Op_Generator:: ~Xarchon_Op_Generator()
{
}
 
void Xarchon_Op_Generator::Clear(void)
{
  for (int i=0;i<BOARD_YCELLS;i++) {
    for (int j=0;j<BOARD_XCELLS;j++) {
      board_cells_visited[i][j]=false;
      board_cells_distance[i][j]=99;
    }
  }
}

void Xarchon_Op_Generator::Init(Xarchon_State *t) 
{
#ifdef PROFILE
  static long value=0;
  static int counter=0;
  static struct timeval timeval_s,timeval_e;
  gettimeofday(&timeval_s,NULL);
#endif

  batch.clear();
  /* IF there is a fight create fight operators */
  if (t->fight) {
    Xarchon_Fight_Operator *win_op=new Xarchon_Fight_Operator(true,5);
    Xarchon_Fight_Operator *lose_op=new Xarchon_Fight_Operator(false,5);
    
    batch.push_front(win_op);
    batch.push_front(lose_op);
    batch_iter=batch.begin();
    return;
  }
  // Add Movements commands
  // Create Move Statements
  for (int i=0;i<BOARD_YCELLS;i++) {
    for (int j=0;j<BOARD_XCELLS;j++) {
      if (t->board_cells[i][j].actor!=0) { 
	if ( (t->board_cells[i][j].flags & CELL_IMPRISON)==0) {
	  if (actor_is_side(t->board_cells[i][j].actor,t->side)) {
	    Clear();
	    Generate_Move(t,j,i,t->board_cells[i][j].actor->type & ACTOR_FLY,
			  0,t->board_cells[i][j].actor->distance,j,i,t->board_cells[i][j].actor);
	  }
	}
      }
    }
  }
#ifdef DEBUGAI
  for (batch_iter=batch.begin();batch_iter!=batch.end();batch_iter++) {
    Operator <Xarchon_State> *dop=*batch_iter;
    Xarchon_Goal_Test gt;
    double val=gt.Goal_Value(dop->Operate(t));
    printf ("value %g\n",val);
  }
#endif

#ifdef DEBUGAI 
  printf ("Branch_Factor %d\n",batch.size());
#endif

  batch_iter=batch.begin();

#ifdef PROFILE
  gettimeofday(&timeval_e,NULL);
  value+=(timeval_e.tv_sec-timeval_s.tv_sec)*1000000+(timeval_e.tv_usec-timeval_s.tv_usec);
  counter++;
  printf ("Generator (%ld,%d)\n",value,counter);
#endif

}

void Xarchon_Op_Generator::Generate_Move(Xarchon_State *t,int x,int y,bool fly,int cur_move,int max_move,int s_x,int s_y,ACTOR *a)
{
  if (x<0 || y<0 || x>=BOARD_XCELLS || y>=BOARD_YCELLS || cur_move>max_move) 
    return;
  if ( board_cells_distance[y][x]>cur_move)
    board_cells_distance[y][x]=cur_move;
  else
    return;
  if (board_cells_distance[y][x]>max_move)
    return;
  if (x!=s_x || y!=s_y) {  
    if (t->board_cells[y][x].actor==NULL) {
      if (!board_cells_visited[y][x]) {
	Xarchon_Move_Operator *move_op=new Xarchon_Move_Operator(0,x,y,s_x,s_y);
	batch.push_back(move_op);
	board_cells_visited[y][x]=true;
      }
    }
    else {
      if (actor_is_side(t->board_cells[y][x].actor,t->side)) {
        if (!fly)
  	  return;
      }
      else {
	if (!board_cells_visited[y][x]) {
	  Xarchon_Move_Operator *move_op=new Xarchon_Move_Operator(0,x,y,s_x,s_y);
	  batch.push_front(move_op);
	  board_cells_visited[y][x]=true;
	  if (!fly)
	    return;
	}
	else
	  if (!fly)
	    return;
      }
    }
  }
  Generate_Move(t  , x-1 , y   , fly ,cur_move+1,max_move,s_x,s_y,a);
  Generate_Move(t  , x+1 , y   , fly ,cur_move+1,max_move,s_x,s_y,a);
  Generate_Move(t  , x   , y-1 , fly ,cur_move+1,max_move,s_x,s_y,a);
  Generate_Move(t  , x   , y+1 , fly ,cur_move+1,max_move,s_x,s_y,a);
  if (fly) {
    Generate_Move(t, x-1 , y-1 , fly ,cur_move+1,max_move,s_x,s_y,a);
    Generate_Move(t, x-1 , y+1 , fly ,cur_move+1,max_move,s_x,s_y,a);
    Generate_Move(t, x+1 , y-1 , fly ,cur_move+1,max_move,s_x,s_y,a);
    Generate_Move(t, x+1 , y+1 , fly ,cur_move+1,max_move,s_x,s_y,a);
  }
}    
  

Operator<Xarchon_State> *Xarchon_Op_Generator::Next(void) 
{
  Operator <Xarchon_State> *cur=*batch_iter;
  batch_iter++;
  return cur;
}

bool Xarchon_Op_Generator::IsEnd(void) 
{
  return batch_iter==batch.end();
}

Xarchon_Goal_Test::Xarchon_Goal_Test(void)
{
  value_parm[LIVE_SCORE]=300;
  value_parm[LIVE_HP_SCORE]=30;
  value_parm[POWERPOINT_SCORE]=200;
  value_parm[WIN_SCORE]=300;
  value_parm[RANDOM_SCORE]=30;
}

double Xarchon_Goal_Test::Goal_Value(Xarchon_State *a) 
{
#ifdef PROFILE
  static long value=0;
  static int counter=0;
  static struct timeval timeval_s,timeval_e;
  gettimeofday(&timeval_s,NULL);
#endif

  /* Calculate Fight value */
  /* ********************* */
  if (a->fight) {
    Xarchon_Fight_Goal_Test k(this,this);
    return k.Goal_Value(a);
  }

  double ret_value=0;

  int num_power=0,light_power=0,dark_power=0,total_light=0,total_dark=0;

  /* Calculate Physical Value */
  /* ************************ */
  for (int i=0;i<BOARD_YCELLS;i++) {
    for (int j=0;j<BOARD_XCELLS;j++) {
      CELL *cell=&a->board_cells[i][j];
      if (cell->flags & CELL_POWER)
	num_power++;
      if (a->board_cells[i][j].actor!=NULL) {
	ACTOR *actor=cell->actor;
	int life=field_initial_life(actor,cell,NULL); // How much strength does the actor actually have
	if (actor_is_side(actor,0)) {
	  if (cell->flags & CELL_POWER) 
	    light_power++;
	  total_light++;
	  ret_value+=value_parm[LIVE_SCORE];
	  ret_value+=value_parm[LIVE_HP_SCORE]*life;
	}
	if (actor_is_side(actor,1)) {
	  if (cell->flags & CELL_POWER) 
	    dark_power++;
	  total_dark++;
	  ret_value-=value_parm[LIVE_SCORE];
	  ret_value-=value_parm[LIVE_HP_SCORE]*life;
	}
      }
    }
  }

  /* Power Points Bonus */
  /* ****************** */
  for (int i=0;i<light_power;i++)
    ret_value+=value_parm[POWERPOINT_SCORE]*i;
  for (int i=0;i<dark_power;i++)
    ret_value-=value_parm[POWERPOINT_SCORE]*i;
  
  /* Winning Bonus */
  /* ************* */

  if (total_light==0 || dark_power==num_power)
    ret_value-=value_parm[WIN_SCORE];
  if (total_dark==0 || light_power==num_power)
    ret_value+=value_parm[WIN_SCORE];

  /* Random Factor */
  /* ************* */
  if (value_parm[RANDOM_SCORE]<0)
    value_parm[RANDOM_SCORE]*=-1;
  int r=(int)value_parm[RANDOM_SCORE];
  if (r==0)
    r=1;
  int random_value=rand()%r;
  ret_value+=random_value;

#ifdef PROFILE
  gettimeofday(&timeval_e,NULL);
  value+=(timeval_e.tv_sec-timeval_s.tv_sec)*1000000+(timeval_e.tv_usec-timeval_s.tv_usec);
  counter++;
  printf ("Xarchon Goal (%ld,%d)\n",value,counter);
#endif

  return ret_value;
}

Xarchon_Fight_Goal_Test::Xarchon_Fight_Goal_Test(Goal_Test <double,Xarchon_State> *w,Goal_Test <double,Xarchon_State> *l)
{
  win=w;
  lose=l;
}

double Xarchon_Fight_Goal_Test::Goal_Value(Xarchon_State *a)
{
  if (a->fight) {
    int al,dl;
    double p;
    Field_Statistics(a->board_cells[a->y][a->x].actor,a->attacker,&a->board_cells[a->y][a->x],&p,&al,&dl);
    Xarchon_Fight_Operator win_oper  (true,al);
    Xarchon_Fight_Operator lose_oper (false,dl);
    Xarchon_State *win_s=win_oper.Operate(a);
    Xarchon_State *lose_s=lose_oper.Operate(a);
    double win_goal=win->Goal_Value(win_s);
    double lose_goal=lose->Goal_Value(lose_s);
    double ret_value=win_goal*p+lose_goal*(1-p);
    delete win_s;
    delete lose_s;
    return ret_value;
  }
  else
    return win->Goal_Value(a);
}

int Xarchon_Path_Cost::Cost(Xarchon_State *a,Operator<Xarchon_State> *o)
{
  return 0;
}

Xarchon_Problem::Xarchon_Problem(void)
{
  gen=new Xarchon_Op_Generator();
  gt=new Xarchon_Goal_Test();
  pc=new Xarchon_Path_Cost();
}

Xarchon_Problem::~Xarchon_Problem()
{
  delete gen;
  delete gt;
  delete pc;
}

Xarchon_Strategy::Xarchon_Strategy(void)
{
  problem=new Xarchon_Problem;
  oper=NULL;
}

Xarchon_Strategy::~Xarchon_Strategy(void)
{
  delete problem;
}

Operator<Xarchon_State> *Xarchon_Strategy::Next(void)
{
  return oper;
}

void Xarchon_War_Lord::InitState(Xarchon_State *state) 
{
  if (!state->fight) {
    oper=NULL;
    return;
  }
  double chance;
  int al,dl;
  Field_Statistics(state->board_cells[state->y][state->x].actor,state->attacker,&state->board_cells[state->y][state->x],&chance,&al,&dl);
  chance=chance *1000;
  int r=(rand()%1000);
#ifdef DEBUGAI
  printf ("Random %d",r);
#endif
  if (r < (int)chance)
    oper=new Xarchon_Fight_Operator(true,al);
  else
    oper=new Xarchon_Fight_Operator(false,dl);
}

Xarchon_Player_Strategy::Xarchon_Player_Strategy(int d,int b)
{
  depth=d;
  branch=b;
}

Xarchon_Light_Player::Xarchon_Light_Player(int d,int b) : Xarchon_Player_Strategy(d,b)
{
  gt=new Xarchon_Goal_Test;
}

Xarchon_Light_Player::~Xarchon_Light_Player(void)
{
  delete gt;
}

void Xarchon_Light_Player::InitState(Xarchon_State *state) 
{
  Xarchon_Light_Accumulator light;
  Xarchon_Goal_Search search(depth,branch,true,&light,gt);
  search.Goal_Value(state);
  oper=light.Get_Operator();
}

Xarchon_Dark_Player::Xarchon_Dark_Player(int d,int b) : Xarchon_Player_Strategy(d,b)
{
  gt=new Xarchon_Goal_Test;
}

Xarchon_Dark_Player::~Xarchon_Dark_Player(void)
{
  delete gt;
}

void Xarchon_Dark_Player::InitState(Xarchon_State *state) 
{
  Xarchon_Dark_Accumulator dark;
  Xarchon_Goal_Search search(depth,branch,false,&dark,gt);
  search.Goal_Value(state);
  oper=dark.Get_Operator();
}


Xarchon_Sequence::Xarchon_Sequence(int light_depth,int light_branch,int dark_depth,int dark_branch)
{
  light=new Xarchon_Light_Player(light_depth,light_branch);
  dark=new Xarchon_Dark_Player(dark_depth,dark_branch);
  warlord=new Xarchon_War_Lord;
}

Xarchon_Sequence::~Xarchon_Sequence(void)
{
  delete light;
  delete dark;
  delete warlord;
}

Strategy<double,Xarchon_State,int> *Xarchon_Sequence::Next_Strategy(Xarchon_State *p) 
{
  if (p->fight)
    return warlord;
  if (p->side==0)
    return light;
  if (p->side==1)
    return dark;
  return warlord;
}

Xarchon_Goal_Search::Xarchon_Goal_Search(int d,int b,bool m,
					 Goal_Accumulator<double,Xarchon_State> *res,
					 Goal_Test<double,Xarchon_State> *t)
  : Successor_Goal_Test<double,Xarchon_State>(NULL,NULL,t)
{
  generator=new Xarchon_Op_Generator;
  max=m;
  goal_acc=res;
  depth=d;
  branch=b;
}

Xarchon_Goal_Search::~Xarchon_Goal_Search(void)
{
  delete generator;
}


double Xarchon_Goal_Search::Goal_Value(Xarchon_State *a)
{
  int endofgame=a->EndOfGame();
  if (endofgame!=0)
    return tester->Goal_Value(a);

  if (a->fight) {
    if (max) {
      Xarchon_Light_Accumulator light_win,light_lose;
      Xarchon_Goal_Search light_search_win (depth,branch,max,&light_win,tester);
      Xarchon_Goal_Search light_search_lose (depth,branch,max,&light_lose,tester);
      Xarchon_Fight_Goal_Test fighter(&light_search_win,&light_search_lose);
      return fighter.Goal_Value(a);
    }
    else {
      Xarchon_Dark_Accumulator dark_win,dark_lose;
      Xarchon_Goal_Search dark_search_win  (depth,branch,max,&dark_win,tester);
      Xarchon_Goal_Search dark_search_lose (depth,branch,max,&dark_lose,tester);
      Xarchon_Fight_Goal_Test fighter(&dark_search_win,&dark_search_lose);
      return fighter.Goal_Value(a);
    }
  }

  if (depth==0) 
    tester->Goal_Value(a);

  if (depth==1) 
    return Successor_Goal_Test<double,Xarchon_State>::Goal_Value(a);
  
  Goal_Accumulator<double,Xarchon_State> *old=goal_acc;
  
  if (max) {
    Multi_Goal_Accumulator<double,Xarchon_State,greater<double> > mga(branch);
    goal_acc=&mga;
    Successor_Goal_Test<double,Xarchon_State>::Goal_Value(a);
    goal_acc=old;
      for (mga.Init();!mga.IsEnd();mga.Next()) { 
      Xarchon_State *state=mga.Get_State();
      Operator<Xarchon_State> *oper=mga.Get_Operator();
#ifdef DEBUGAI
      ((Xarchon_Operator *)oper)->Display();
#endif
      Xarchon_Dark_Accumulator dark;
      Xarchon_Goal_Search search(depth-1,branch,!max,&dark,tester);
      double new_value=search.Goal_Value(state);
#ifdef DEBUGAI
      double old_value=tester->Goal_Value(state);
      printf ("(%g,%g) \n ",old_value,new_value);
#endif
      goal_acc->Put(state,oper,new_value);
    }
  }
  
  else {
    Multi_Goal_Accumulator<double,Xarchon_State,less<double> > mga(branch);
    goal_acc=&mga;
    Successor_Goal_Test<double,Xarchon_State>::Goal_Value(a);
    goal_acc=old;
    for (mga.Init();!mga.IsEnd();mga.Next()) { 
      Xarchon_State *state=mga.Get_State();
      Operator<Xarchon_State> *oper=mga.Get_Operator();
#ifdef DEBUGAI
      ((Xarchon_Operator *)oper)->Display();
#endif
      Xarchon_Light_Accumulator light;
      Xarchon_Goal_Search search(depth-1,branch,!max,&light,tester);
      double new_value=search.Goal_Value(state);
#ifdef DEBUGAI
      double old_value=tester->Goal_Value(state);
      printf ("(%g,%g) \n ",old_value,new_value);
#endif
      goal_acc->Put(state,oper,new_value);
    }
  }
#ifdef DEBUGAI
  double val=goal_acc->Get_Value();
  printf ("Eval %g \n",val);
#endif
  return goal_acc->Get_Value();
}
