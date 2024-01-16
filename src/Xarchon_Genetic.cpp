#include "Xarchon_Genetic.hpp"

Xarchon_Genetic_Population::Xarchon_Genetic_Population(int s) : Genetic_Population<Xarchon_Goal_Test>(s)
{
  for (int i=0;i<size;i++) 
    pop[i].state=NULL;
}

Xarchon_Genetic_Population::~Xarchon_Genetic_Population()
{
}

void Xarchon_Genetic_Population::Create_New(int range)
{
  int range2=range/2;
  for (int i=0;i<size;i++) {
    Xarchon_Goal_Test *newtest=new Xarchon_Goal_Test;
    for (int j=0;j<TOTAL_VALUE_PARMS;j++) {
      double val=(int)rand()%range-range2;
      newtest->value_parm[j]=val;
    }
    pop[i].state=newtest;
  }
}

void Xarchon_Genetic_Population::Load(char *name)
{
  Create_New(200);
}

void Xarchon_Genetic_Population::Save(char *name)
{
  //  FILE *file;
  //  file =fopen(name,"r+");
  for (int i=0;i<size;i++) {
    Xarchon_Goal_Test *gt=pop[i].state;
    printf ("Value %g ",pop[i].fitness);
    for (int j=0;j<TOTAL_VALUE_PARMS;j++)
      printf("%g ",gt->value_parm[j]);
    printf ("\n");
  }
  //  fclose(file);
}


Xarchon_Genetic_Reproduce::Xarchon_Genetic_Reproduce(void)
{
  weight=1;
}

void Xarchon_Genetic_Reproduce::Operate(void) 
{
  Xarchon_Goal_Test *newtest=new Xarchon_Goal_Test;
  for (int i=0;i<TOTAL_VALUE_PARMS;i++) 
    newtest->value_parm[i]=data->value_parm[i];
  data=newtest;
}

Xarchon_Genetic_Mutate::Xarchon_Genetic_Mutate(void)
{
  weight=1;
}

void Xarchon_Genetic_Mutate::Operate(void) 
{
  Xarchon_Goal_Test *newtest=new Xarchon_Goal_Test;
  for (int i=0;i<TOTAL_VALUE_PARMS;i++) {
    newtest->value_parm[i]=data->value_parm[i];
    int r=rand()%10;
    if (r==0) 
      newtest->value_parm[i]+=(rand()%100)-50;
  }
  data=newtest;
}

Xarchon_Genetic_Crossover::Xarchon_Genetic_Crossover(void)
{
  weight=20;
}

void Xarchon_Genetic_Crossover::Operate(void) 
{
  Xarchon_Goal_Test *newtest1=new Xarchon_Goal_Test;
  Xarchon_Goal_Test *newtest2=new Xarchon_Goal_Test;
  int r=rand()%TOTAL_VALUE_PARMS,i=0;
  for (i=0;i<r;i++) {
    newtest1->value_parm[i]=first->value_parm[i];
    newtest2->value_parm[i]=second->value_parm[i];
  }
  for (;i<TOTAL_VALUE_PARMS;i++) {
    newtest1->value_parm[i]=second->value_parm[i];
    newtest2->value_parm[i]=first->value_parm[i];
  }
  first=newtest1;
  second=newtest2;
}

Xarchon_Genetic_Fitness::Xarchon_Genetic_Fitness(void)
{
  eval_parms[EVAL_LIVE_SCORE]=100;
  eval_parms[EVAL_LIVE_HP_SCORE]=1;
  eval_parms[EVAL_POWERPOINT_SCORE]=50;
  eval_parms[EVAL_WIN_SCORE]=2000;
  eval_parms[EVAL_TURN_SCORE]=-5;
}

double Xarchon_Genetic_Fitness::Evaluate_Board(Xarchon_State *a,int side)
{
  double ret_value=0;
  
  /* Calculate Power Points possession */
  /* ********************************* */
  int magicpointx[5]={0,4,4,4,8};
  int magicpointy[5]={4,0,4,8,4};
  int power=0;
  for (int k=0;k<5;k++) {
    ACTOR *poweractor=a->board_cells[magicpointy[k]][magicpointx[k]].actor;
    if (poweractor!=NULL) {
      if (actor_is_side(poweractor,side))
	power++;
    }
  }
  for (int i=0;i<power;i++) 
    ret_value+=eval_parms[EVAL_POWERPOINT_SCORE]*i;
  
  /* Calculate Physical Value */
  /* ************************ */
  int count_light=0,count_dark=0;
  for (int i=0;i<BOARD_YCELLS;i++) {
    for (int j=0;j<BOARD_XCELLS;j++) {
      if (a->board_cells[i][j].actor!=NULL) {
	CELL *cell=&a->board_cells[i][j];
	ACTOR *actor=cell->actor;
	int life=actor->strength;
	if (actor_is_side(actor,0) && side==0) {
	  count_light++;
	  ret_value+=eval_parms[EVAL_LIVE_SCORE];
	  ret_value+=eval_parms[EVAL_LIVE_HP_SCORE]*life;
	}
	if (actor_is_side(actor,1) && side==1) {
	  count_dark++;
	  ret_value+=eval_parms[EVAL_LIVE_SCORE];
	  ret_value+=eval_parms[EVAL_LIVE_HP_SCORE]*life;
	}
      }
    }
  }
  
  if ( (side==0 && count_dark==0) || (side==1 && count_light==0))
    ret_value+=eval_parms[EVAL_WIN_SCORE];
  
  ret_value += eval_parms[EVAL_TURN_SCORE] * a->turn_num;
  
  return ret_value;
}

double Xarchon_Genetic_Fitness::Calculate_Fitness(Xarchon_Goal_Test *a)
{
  double ret_fit=0;
  Xarchon_Game game;
    
  for (int i=0;i<pop->size;i++ ) {
    printf ("Testing Citizen %d\n",i);
    // Playing as Light
    game.SetPlayer(a,(Xarchon_Goal_Test *)&pop->pop[i]);
    game.InitStart(0);
    game.Play();

    ret_fit+=Evaluate_Board(game.state,0)-Evaluate_Board(game.state,1);

    // Playing as Dark
    game.SetPlayer((Xarchon_Goal_Test *)&pop->pop[i],a);
    game.InitStart(0);
    game.Play();
    
    ret_fit+=Evaluate_Board(game.state,1)-Evaluate_Board(game.state,0);

  }
  return ret_fit;
}


Xarchon_Genetic_Generation::Xarchon_Genetic_Generation(void)
{
  AddOperator(&reproduce);
  AddOperator(&mutate);
  AddOperator(&crossover);
}

Xarchon_Genetic_Generation::~Xarchon_Genetic_Generation(void)
{
}

Genetic_Population<Xarchon_Goal_Test> *Xarchon_Genetic_Generation::Operate(Genetic_Population<Xarchon_Goal_Test> *p)
{
  Genetic_Population<Xarchon_Goal_Test> *ret=::Genetic_Generation_a<Xarchon_Goal_Test>::Operate(p);

  double minvalue=0;
  bool first=true;

  for (int i=0;i<ret->size;i++) {
    if (first) {
      first=false;
      minvalue=ret->pop[i].fitness;
      continue;
    }
    if (minvalue>ret->pop[i].fitness)
      minvalue=ret->pop[i].fitness;
  }
  
  if (minvalue<0) {
    minvalue=minvalue * -2;
    for (int i=0;i<ret->size;i++)
      ret->pop[i].fitness+=minvalue;
  }
  return ret;
}

Xarchon_Evolution::Xarchon_Evolution(char *test,char *p,int num)
{
  num_gen=num;
  testname=test;
  popname=p;
  test_pop=new Xarchon_Genetic_Population(20);
  gen_pop=new Xarchon_Genetic_Population(10);
  fitness=new Xarchon_Genetic_Fitness;
  gg=new Xarchon_Genetic_Generation;
  test_pop->Load(test);
  gen_pop->Load(p);
  fitness->pop=test_pop;
  gg->fit_func=fitness;
  gen_pop->Eval_Fitness(fitness);
  
  {
    double minvalue=0;
    bool first=true;
    
    for (int i=0;i<gen_pop->size;i++) {
      if (first) {
	first=false;
	minvalue=gen_pop->pop[i].fitness;
	continue;
      }
      if (minvalue>gen_pop->pop[i].fitness)
	minvalue=gen_pop->pop[i].fitness;
    }
    
    if (minvalue<0) {
      minvalue=minvalue * -2;
      for (int i=0;i<gen_pop->size;i++)
	gen_pop->pop[i].fitness+=minvalue;
    }
  }




  gen_pop->Save(p);
}

Genetic_Population<Xarchon_Goal_Test> *Xarchon_Evolution::Operate(Genetic_Population<Xarchon_Goal_Test> *p)
{
  return Operate();
}

Genetic_Population<Xarchon_Goal_Test> *Xarchon_Evolution::Operate(void)
{
  Genetic_Population<Xarchon_Goal_Test> *cur;
  for (int i=0;i<num_gen;i++) {
    cur=gen_pop;
    printf ("Running Generation %d\n",i);
    gen_pop=(Xarchon_Genetic_Population *)gg->Operate(gen_pop);
    gen_pop->Save(popname);
  }
  return gen_pop;
}

Xarchon_Evolution::~Xarchon_Evolution(void)
{
  delete test_pop;
  delete gen_pop;
  delete fitness;
  delete gg;
}
