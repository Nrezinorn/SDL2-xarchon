#ifndef XARCHON_GENETIC_HPP
#define XARCHON_GENETIC_HPP

#include <stdio.h>

#include "Genetic.hpp"
#include "Xarchon_Problem.hpp"
#include "Xarchon_Game.hpp"


// Genetic Material is Xarchon_Goal_Test

class Xarchon_Genetic_Population : public Genetic_Population<Xarchon_Goal_Test>
{
public:
  Xarchon_Genetic_Population(int s);
  virtual ~Xarchon_Genetic_Population();
  void Create_New(int range);
  void Load(char *name);
  void Save(char *name);
};

enum {
  EVAL_LIVE_SCORE,
  EVAL_LIVE_HP_SCORE,
  EVAL_POWERPOINT_SCORE,
  EVAL_WIN_SCORE,
  EVAL_TURN_SCORE,
  EVAL_TOTAL_VALUE_PARMS
};

class Xarchon_Genetic_Fitness : public Genetic_Fitness_Function<Xarchon_Goal_Test>
{
public:
  Xarchon_Genetic_Population *pop;
  double eval_parms[EVAL_TOTAL_VALUE_PARMS];
public:
  Xarchon_Genetic_Fitness(void);
  virtual double Evaluate_Board(Xarchon_State *a,int side);
  virtual double Calculate_Fitness(Xarchon_Goal_Test *a);
};

class Xarchon_Genetic_Reproduce : public Genetic_Operator_Reproduce<Xarchon_Goal_Test>
{
public:
  Xarchon_Genetic_Reproduce(void);
  virtual void Operate(void) ;
};

class Xarchon_Genetic_Mutate : public Genetic_Operator_Mutate <Xarchon_Goal_Test>
{
public:
  Xarchon_Genetic_Mutate(void);
  virtual void Operate(void) ;
};

class Xarchon_Genetic_Crossover : public Genetic_Operator_Crossover <Xarchon_Goal_Test>
{
public:
  Xarchon_Genetic_Crossover(void);
  virtual void Operate(void) ;
};

class Xarchon_Genetic_Generation : public Genetic_Generation_a<Xarchon_Goal_Test>
{
  Xarchon_Genetic_Reproduce reproduce;
  Xarchon_Genetic_Mutate mutate;
  Xarchon_Genetic_Crossover crossover;
public:
  Xarchon_Genetic_Generation(void);
  virtual Genetic_Population<Xarchon_Goal_Test> *Operate(Genetic_Population<Xarchon_Goal_Test> *p);
  virtual ~Xarchon_Genetic_Generation(void);
};

class Xarchon_Evolution : public Genetic_Evolution<Xarchon_Goal_Test>
{
public:
  char *testname,*popname;
  int num_gen;
  Xarchon_Genetic_Fitness *fitness;
  Xarchon_Genetic_Population *test_pop,*gen_pop;
  Xarchon_Genetic_Generation *gg;
public:
  Xarchon_Evolution(char *test,char *pop,int num);
  Genetic_Population<Xarchon_Goal_Test> *Xarchon_Evolution::Operate(Genetic_Population<Xarchon_Goal_Test> *p);
  Genetic_Population<Xarchon_Goal_Test> *Xarchon_Evolution::Operate(void);
  virtual ~Xarchon_Evolution(void);
};

#endif
