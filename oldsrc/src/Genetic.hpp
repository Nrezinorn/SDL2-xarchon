#ifndef GENETIC_HPP
#define GENETIC_HPP

#include <list>
using namespace std;

/* **********************************************
   Problem
   **********************************************
   
   Classes 
   -------
   State              - Domain of the seach space
   Uniary Operator    - State -> State 
   Binary Operator    - State X State -> State X State  
   Population         - list of <States>
   Genetic_Generation - Population -> Population (Single Step)
   Evolution          - Population -> Population (Process)
   
*/

class Genetic_State {};

template <class T> class Genetic_Fitness_Function
{
public:
  virtual double Calculate_Fitness(T *a) = 0;
};

template <class T> class Genetic_Fitness_State
{
public:
  double fitness;
  T *state;
public:
  Genetic_Fitness_State (T *s=NULL,double f=0)
  {
    state=s;
    fitness=f;
  }
  virtual ~Genetic_Fitness_State(void)
  {
    if (state!=NULL)
      delete state;
  }
};

template <class T> class Genetic_Operator 
{
public:
  double weight;
public:
  virtual int NumOfOperands(void) = 0 ;
  virtual int NumOfResults(void)  = 0;
  virtual bool PushOperand(T *s) = 0;
  virtual T *PopResult(void) = 0;
  virtual void Operate(void) = 0;
};

template <class T> class Genetic_No_Operator : public Genetic_Operator<T>
{
public:
  virtual int NumOfOperands(void) { return 0; }
  virtual int NumOfResults(void)  { return 1; }
  virtual bool PushOperand(T *s)  { return true; }
  virtual void Operate(void) {} 
};

template <class T> class Genetic_Uniary_Operator : public Genetic_Operator<T>
{
public:
  T *data;
public:
  Genetic_Uniary_Operator(void)
  {
    data=NULL;
  }

  virtual int NumOfOperands(void) { return 1; }
  virtual int NumOfResults(void)  { return 1; }
  virtual bool PushOperand(T *s) 
  {
    data=s;
    return true;
  }
  virtual T *PopResult(void) 
  {
    return data;
  };
};

template <class T> class Genetic_Binary_Operator : public Genetic_Operator<T>
{
public:
  T *first;
  T *second;
public:
  Genetic_Binary_Operator(void)
  {
    first=NULL;
    second=NULL;
  }
  virtual int NumOfOperands(void) { return 2; }
  virtual int NumOfResults(void)  { return 2; }

  virtual bool PushOperand(T *s) 
  {
    if (first==NULL) {
      first=s;
      return true;
    }
    else {
      second=s;
      return false;
    }
  }

  virtual T *PopResult(void) 
  {
    T *ret;
    if (second==NULL) {
      ret=first;
      first=NULL;
    }
    else {
      ret=second;
      second=NULL;
    }
    return ret;
  }
};

template <class T> class Genetic_Operator_Createnew : public Genetic_No_Operator<T> {};
template <class T> class Genetic_Operator_Reproduce : public Genetic_Uniary_Operator<T> {};
template <class T> class Genetic_Operator_Mutate    : public Genetic_Uniary_Operator<T> {};
template <class T> class Genetic_Operator_Crossover : public Genetic_Binary_Operator<T> {};

template <class T> class Genetic_Population 
{
public:
  int size;
  Genetic_Fitness_State<T> *pop;
  double total_fitness;
public:
  Genetic_Population(int s)
  {
    total_fitness=0;
    size=s;
    pop=new Genetic_Fitness_State<T>[s];
  }
  
  virtual ~Genetic_Population(void)
  {
    if (pop==NULL)
      return ;
    delete pop;
  }  

  T *Choose_State(void)
  {
    int max=(int)total_fitness;
    int r=rand()%max;
    double count=0;
    for (int i=0;i<size;i++) {
      count+=pop[i].fitness;
      if (r<count)
	return pop[i].state;
    }
    return pop[0].state;
  }

  void Eval_Fitness(Genetic_Fitness_Function<T> *op) 
  {
    for (int i=0;i<size;i++) {
      pop[i].fitness=op->Calculate_Fitness(pop[i].state);
      total_fitness+=pop[i].fitness;
    }
  }

  T *Choose_Best(void)
  {
    T *best=NULL;
    double best_fit=-9999999;
    for (int i=0;i<size;i++) {
      if (pop[i].fitness>best_fit) {
	best_fit=pop[i].fitness;
	best=pop[i].state;
      }
    }
    return best;
  }
};

template <class T> class Genetic_Generation
{
public:
  list<Genetic_Operator <T> *> operators;
  double total_op_weight;
  Genetic_Fitness_Function <T> *fit_func;
public:
  Genetic_Generation(void)
  {
    total_op_weight=0;
    fit_func=0;
  }

  virtual ~Genetic_Generation(void)  {};

  virtual void AddOperator(Genetic_Operator<T> *op) 
  {
    total_op_weight+=op->weight;
    operators.push_front(op);
  }

  virtual Genetic_Population<T> *Operate(Genetic_Population<T> *p) = 0;
  virtual Genetic_Operator<T> *Choose_Operator(void)
  {
    int max=(int)total_op_weight;
    list<Genetic_Operator<T> *>::iterator start=operators.begin();
    list<Genetic_Operator<T> *>::iterator end=operators.end();
    list<Genetic_Operator<T> *>::iterator iter;
    int r=rand()%max;
    int count=0;
    for (iter=start;iter!=end;iter++) {
      Genetic_Operator<T> *op=*iter;
      count+=(int)op->weight;
      if (r<count)
	return op;
    }
    return *start;
  }
};

template <class T> class Genetic_Evolution
{
public:
  virtual Genetic_Population<T> *Operate(Genetic_Population<T> *p) = 0;
};


/* Generation and Evolution definite types */

template <class T> class Genetic_Generation_a : public Genetic_Generation<T>
{
public:
  virtual Genetic_Population<T> *Operate(Genetic_Population<T> *p)
  {
    int i;
    Genetic_Population<T> *newpop=new Genetic_Population<T>(p->size);
    int cur_size=0;
    while (cur_size<newpop->size) {
      Genetic_Operator<T> *op=Choose_Operator();
      if ( (cur_size+op->NumOfResults())>newpop->size)
	continue;
      for (i=0;i<op->NumOfOperands();i++) {
	T *s=p->Choose_State();
	op->PushOperand(s);
      }
      op->Operate();
      for (i=0;i<op->NumOfResults();i++) {
	T *s=op->PopResult();
	newpop->pop[cur_size].state=s;
	newpop->pop[cur_size].fitness=0;
	cur_size++;
      }
    }
    newpop->Eval_Fitness(fit_func);
    return newpop;
  }

};


template <class T> class Genetic_Evolution_a : public Genetic_Evolution<T>
{
public:
  int max_gen_num;
  Genetic_Generation<T> *go;
public:
  virtual Genetic_Population<T> *Operate(Genetic_Population<T> *p) 
  {
    Genetic_Population<T> *cur=p;
    Genetic_Population<T> *res=cur;
    for (int i=0;i<max_gen_num;i++) {
      printf ("Running Generation %d\n",i);
      res=go->Operate(cur);
      delete cur;
      cur=res;
    }
    return cur;
  }
};
	
#endif
