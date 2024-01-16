#ifndef XARCHON_GAME_HPP
#define XARCHON_GAME_HPP

#include "Xarchon_Problem.hpp"
#include "board.h"

class Xarchon_Game
{
public:
  Xarchon_State *state;
  Xarchon_Sequence seq;
public:
  Xarchon_Game(void);
  virtual ~Xarchon_Game(void);
  void SetPlayer(Xarchon_Goal_Test *light,Xarchon_Goal_Test *dark);
  void InitStart(int light_first,int init[BOARD_YCELLS][BOARD_XCELLS]=NULL);
  void Display(void);
  void Play(void);
  void Move(void);
  bool CheckEnd(void);
};

#endif
