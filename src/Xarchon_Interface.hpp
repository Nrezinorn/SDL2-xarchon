#ifndef XARCHON_INTERFACE_HPP
#define XARCHON_INTERFACE_HPP

#include "Xarchon_Problem.hpp"
#include "Xarchon_Game.hpp"
#include "Xarchon_Genetic.hpp"

/* ***************************************************** */
/* Interface Function                                    */
/* ***************************************************** */
// returns the probability that the attacker will win and al - attacker loss dl - defender loss

#ifdef __cplusplus
const int maxsize=20;
#endif

extern "C" {
  void Xarchon_AI_Computer(int *x1,int *x2,int *y1,int *y2);
  void Xarchon_Genetics_Main(void);
  void Field_Statistics(ACTOR *defender,ACTOR *attacker,CELL *cell,double *p,int *al,int *dl);
#ifdef __cplusplus
  double FillTable(int cur_x,int cur_y,double p,double table[maxsize][maxsize],int max_x,int max_y);
#else
  double FillTable(int cur_x,int cur_y,double p,double table[][],int max_x,int max_y);
#endif
}

#endif
