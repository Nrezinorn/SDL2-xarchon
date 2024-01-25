#ifndef XARCHON_PROBLEM_HPP
#define XARCHON_PROBLEM_HPP

extern "C" {
#include <sys/time.h>
#include "main.h"
#include "stdio.h"
#include "actors.h"
#include "board.h"
#include "computer.h"
#include "field.h"
#include "actors.h"
#include "iface.h"
}

#include "Problem.hpp"

#include <list>

// < double , Xarchon_State , int >

class Xarchon_State : public State 
{
public:
  int side;
  int turn_num;
  int lumi;
  int lumi_d;
  CELL board_cells[BOARD_YCELLS][BOARD_XCELLS];
  bool fight;      // If there is a fight
  bool elemental;  // If the attacker is an elemental
  int x,y;         // Coordinates of the battle
  ACTOR *attacker; // The attacker ACTOR
public:
  Xarchon_State(void) ;
  Xarchon_State(Xarchon_State *a) ;
  virtual ~Xarchon_State();
  void Destroy(void);
  void Get_Real_State(void);
  ACTOR *Actor_Copy(ACTOR *a);
  void UpdateTurn(void) ;
  int EndOfGame(void);
};

class Xarchon_Operator : public Operator <Xarchon_State> 
{
public:
  Xarchon_Operator(void) {}

  virtual ~Xarchon_Operator(void) {}
  virtual void Display(void) = 0;
};

class Xarchon_Move_Operator : public Xarchon_Operator
{
public:
  COMMAND cmd;
public:
  Xarchon_Move_Operator(int spell,int x2=0,int y2=0,int x1=0,int y1=0);
  virtual ~Xarchon_Move_Operator(void) {};
  virtual Xarchon_State* Operate(Xarchon_State *a) ;
  virtual void Display(void) 
    {
      printf ("(%d,%d) - (%d,%d) ",cmd.b.x2,cmd.b.y2,cmd.b.x1,cmd.b.y1);
    }
};

class Xarchon_Fight_Operator : public Xarchon_Operator
{
public:
  bool win;
  int damage;
public:
  Xarchon_Fight_Operator(bool w,int d);
  virtual ~Xarchon_Fight_Operator(void) {};
  virtual Xarchon_State *Operate(Xarchon_State *a) ;
  virtual void Display(void)
    {
      //        printf ("%s attacking %s ",attacker->name,defender->name);
    }
};

class Xarchon_Op_Generator : public Operator_Generator <Xarchon_State>
{
public:
  list <Operator <Xarchon_State> *> batch;
  list <Operator <Xarchon_State> *>::iterator batch_iter;
  bool board_cells_visited[BOARD_YCELLS][BOARD_XCELLS];
  int board_cells_distance[BOARD_YCELLS][BOARD_XCELLS];
  virtual ~Xarchon_Op_Generator() ; 
public:
  Xarchon_Op_Generator(void);
  void Clear(void);
  void Generate_Move(Xarchon_State *t,int x,int y,bool fly,int cur_move,int max_move,int s_x,int s_y,ACTOR *a);
  virtual void Init(Xarchon_State *t);
  virtual Operator<Xarchon_State> *Next(void)  ;
  virtual bool IsEnd(void) ;
};

enum {
  LIVE_SCORE,
  LIVE_HP_SCORE,
  POWERPOINT_SCORE,
  WIN_SCORE,
  RANDOM_SCORE,
  TOTAL_VALUE_PARMS
};

class Xarchon_Goal_Test : public Goal_Test <double,Xarchon_State>
{
public:
  double value_parm[TOTAL_VALUE_PARMS];
public:
  Xarchon_Goal_Test(void) ;
  virtual double Goal_Value(Xarchon_State *a) ;
};

class Xarchon_Fight_Goal_Test : public Goal_Test <double,Xarchon_State>
{
public:
  Goal_Test<double,Xarchon_State> *win;
  Goal_Test<double,Xarchon_State> *lose;
public:
  Xarchon_Fight_Goal_Test(Goal_Test <double,Xarchon_State> *w,Goal_Test <double, Xarchon_State> *l);
  virtual double Goal_Value(Xarchon_State *a);
};


class Xarchon_Path_Cost : public Path_Cost <Xarchon_State,int>
{
public:
  Xarchon_Path_Cost(void) {};
  virtual int Cost(Xarchon_State *a,Operator<Xarchon_State> *o) ;
};

class Xarchon_Problem : public Problem <double,Xarchon_State,int>
{
public:
  Xarchon_Problem(void);
  virtual ~Xarchon_Problem();
};

class Xarchon_Strategy     : public Strategy <double,Xarchon_State,int>
{
public:
  Operator<Xarchon_State> *oper;
public:
  Xarchon_Strategy(void);
  virtual ~Xarchon_Strategy(void);
  virtual Operator<Xarchon_State> *Next(void) ;
};

class Xarchon_War_Lord     : public Xarchon_Strategy
{
public:
  virtual void InitState(Xarchon_State *state) ;
};

class Xarchon_Player_Strategy : public Xarchon_Strategy
{
public:
  int depth;
  int branch;
public:
  Xarchon_Player_Strategy(int d,int b);
};

class Xarchon_Light_Player : public Xarchon_Player_Strategy
{
public:
  Xarchon_Goal_Test *gt;
public:
  Xarchon_Light_Player(int d,int b) ;
  ~Xarchon_Light_Player(void);
  virtual void InitState(Xarchon_State *state) ;
};

class Xarchon_Dark_Player  : public Xarchon_Player_Strategy
{
public:
  Xarchon_Goal_Test *gt;
public:
  Xarchon_Dark_Player(int d,int b);
  ~Xarchon_Dark_Player(void);
  virtual void InitState(Xarchon_State *state) ;
};


class Xarchon_Sequence : public Strategy_Sequence <double,Xarchon_State,int>
{
public:
  Xarchon_Light_Player *light;
  Xarchon_Dark_Player *dark;
  Xarchon_War_Lord *warlord;
public:
  Xarchon_Sequence(int light_depth,int light_branch,int dark_depth,int dark_branch);
  virtual ~Xarchon_Sequence(void);
  virtual Strategy<double,Xarchon_State,int> *Next_Strategy(Xarchon_State *p) ;
};

class Xarchon_Dark_Accumulator : public Min_Goal_Accumulator<double,Xarchon_State> {};
class Xarchon_Light_Accumulator : public Max_Goal_Accumulator<double,Xarchon_State> {};
class Xarchon_WarLord_Accumulator : public Goal_Accumulator<double,Xarchon_State> 
{
};

class Xarchon_Goal_Search : public Successor_Goal_Test<double,Xarchon_State>
{
public:
  bool max;
  int depth,branch;
public:
  Xarchon_Goal_Search(int depth,int branch,bool m,
		      Goal_Accumulator<double,Xarchon_State> *res,
		      Goal_Test<double,Xarchon_State> *end);
  virtual ~Xarchon_Goal_Search(void);
  
  virtual double Goal_Value(Xarchon_State *a);
};



#endif
