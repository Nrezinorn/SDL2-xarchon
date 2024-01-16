#ifndef PROBLEM_HPP
#define PROBLEM_HPP

#include <queue>
#include <list>

using namespace std;

/* 
*********************************************************************************
Problem
*********************************************************************************

Classes 
-------
State              - Domain of the search Space
Goal_Result        - Domain of result space (Boolean,Real)
Path_Cost_Value    - Domain of the path cost (void,Real,Vector)

Initial State      - State
Goal               - Goal_Result

Operator           - State -> State
Operator_Generator - State -> list <Operators> (One by one)
Goal_Test          - State -> Goal_Result
Path_Cost          - State X Operator -> Path_Cost_Value
Problem            - (Operatore_Generator,Goal_Test,Path_Cost)
Strategy           - State -> Operator 
Strategy_Sequence  - State -> Strategy

*********************************************************************************
*/

class State {
public:
  virtual ~State(void) {};
};

template <class T> class Operator
{
public :
  virtual ~Operator(void) {};
  virtual T* Operate(T *a) { return a;}; 
};

template <class T> class Operator_Generator 
{ 
public:
  virtual ~Operator_Generator(void) {};
  virtual void Init(T *t) = 0;
  virtual Operator<T> *Next(void) =0 ;
  virtual bool IsEnd(void) = 0;
};

template <class G,class T> class Goal_Test
{
public:
  virtual ~Goal_Test(void) {};
  virtual G Goal_Value(T *a) = 0;
  //  virtual G *Goal_Compare(G *cmp,G *a,G *b) =0;
  // This function compares which Goal is closer to cmp;a or b and returns it
};

template <class T,class P> class Path_Cost
{
public:
  virtual ~Path_Cost(void) {};
  virtual P Cost(T *a,Operator<T> *o) = 0;
};

template <class G,class T,class P> class Problem 
{
public:
  virtual ~Problem(void) {};
public:
  Operator_Generator<T> *gen;
  Goal_Test<G,T> *gt;
  Path_Cost<T,P> *pc;
};

template <class G,class T,class P> class Strategy
{
public:
  Problem<G,T,P> *problem;
public:
  virtual ~Strategy(void) {};
  virtual void InitState(T *state) = 0;
  virtual Operator<T> *Next(void) = 0;
  // Given the caller and the list of search node - What is the goal value expected for me.
};

template <class G,class T,class P> class Strategy_Sequence
{
public:
  virtual ~Strategy_Sequence(void) {};
  virtual Strategy<G,T,P> *Next_Strategy(T *p) = 0;
};

template <class G,class T> class Goal_Accumulator
{
public:
  virtual ~Goal_Accumulator(void) {}
  // Return true if the state,operator and value are kept in the accumulator
  virtual void Put(T *state,Operator<T> *o,G v) = 0;
  virtual T *  Get_State (void) = 0;
  virtual Operator<T> * Get_Operator(void) = 0 ;
  virtual G Get_Value(void) = 0;
};

template <class G,class T,class P> class Single_Goal_Accumulator : public Goal_Accumulator<G,T>
{
public:
  G value;
  T *state;
  Operator<T> *oper;
  bool first;
public:
  Single_Goal_Accumulator(void)
  {
    state=NULL;
    oper=NULL;
    first=true;
  }
  
  virtual ~Single_Goal_Accumulator(void)
  {
    Reset();
  }

  virtual void Reset(void)
  {
    first=true;
    if (state!=NULL)
      delete state;
    if (oper!=NULL)
      delete oper;
    state=NULL;
    oper=NULL;
  }
      
  virtual void Put(T *s,Operator<T> *o,G v) 
  {
    if (first) {
      first=false;
      state=s;
      oper=o;
      value=v;
      return;
    }
    P pred;
    if (pred(v,value)) {
      delete state;
      delete oper;
      state=s;
      value=v;
      oper=o;
    }
    else {
      delete o;
      delete s;
    }
  }

  virtual G Get_Value(void)
  {
    return value;
  }

  virtual T * Get_State(void)
  {
    T *ret=state;
    state=NULL;
    return ret;
  }

  virtual Operator<T> *Get_Operator(void) 
  {
    Operator<T> *ret=oper;
    oper=NULL;
    return ret;
  }
};

template <class G,class T,class P> class Multi_Goal_Accumulator : public Goal_Accumulator<G,T> 
{
public:
  int size;
  T* *states;
  Operator<T>* *operators;
  G* values;
  int count;
  int current;
public:
  Multi_Goal_Accumulator(int s)
  {
    size=s;
    states=new T* [size];
    operators=new Operator<T>* [size];
    values=new G [size];
    for (int i=0;i<size;i++) {
      states[i]=NULL;
      operators[i]=NULL;
    }
    count=0;
    current=0;
  }
  
  virtual ~Multi_Goal_Accumulator(void)
  {
    for (int i=0;i<size;i++) {
      if (states[i]!=NULL)
	delete states[i];
      if (operators[i]!=NULL)
	delete operators[i];
    }
    delete states;
    delete operators;
    delete values;
  }
 
  virtual void Put(T *s,Operator<T> *o,G v) 
  {
    P pred;
    if (count<size) {
      states[count]=s;
      operators[count]=o;
      values[count]=v;
      count++;
      return;
    }
    for (int i=0;i<count;i++) {
      if (pred(v,values[i])) {
	delete states[i];
	delete operators[i];
	states[i]=s;
	operators[i]=o;
	values[i]=v;
	return;
      }
    }
    delete s;
    delete o;
  }

  virtual void Init(void)  { current=0;};
  virtual void Next(void)  { current++;};
  virtual bool IsEnd(void) { return current>=count;};
  virtual G Get_Value(void){ return values[current];};
  virtual T *Get_State(void) 
  { 
    T *ret=states[current];
    states[current]=NULL;
    return ret;
  }
  virtual Operator<T> *Get_Operator(void)
  {
    Operator<T> *ret=operators[current];
    operators[current]=NULL;
    return ret;
  }
};

template <class G,class T> class Max_Goal_Accumulator : public Single_Goal_Accumulator<G,T,greater<G> > {};
template <class G,class T> class Min_Goal_Accumulator : public Single_Goal_Accumulator<G,T,less<G> > {};

template <class G,class T> class Successor_Goal_Test : public Goal_Test<G,T>
{
public:
  Operator_Generator<T> *generator;
  Goal_Accumulator<G,T> *goal_acc;
  Goal_Test<G,T> *tester;
public:
  Successor_Goal_Test(Operator_Generator<T> *g,Goal_Accumulator<G,T> *ga,Goal_Test<G,T> *t)
  {
    generator=g;
    goal_acc=ga;
    tester=t;
  }
  
  virtual G Goal_Value(T *a) 
  {
    for (generator->Init(a);!generator->IsEnd();) {
      Operator<T> *op=generator->Next();
      T *next_state=op->Operate(a);
      G value=tester->Goal_Value(next_state);
      goal_acc->Put(next_state,op,value);
    }
    return goal_acc->Get_Value();
  }
};


template <class G,class T> class Minimax_Goal_Test : public Successor_Goal_Test<G,T> 
{ 
public:
  int depth;
  Goal_Accumulator <G,T> *other_acc;
public:
  Minimax_Goal_Test(int d,Operator_Generator<T> *g,Goal_Accumulator<G,T> *ga,Goal_Accumulator <G,T> *o,Goal_Test<G,T> *end) 
    : Successor_Goal_Test<G,T>(g,ga,end)
  {
    depth=d;
    other_acc=o;
  }

  virtual G Goal_Value(T *a)
  {
    if (depth==0) 
      return tester->Goal_Value(a);
    
    Goal_Test<G,T> *endtester=tester;
    tester=new Minimax_Goal_Test(depth-1,generator,other_acc,goal_acc,tester);
    G ret_val=Successor_Goal_Test<G,T>::Goal_Value(a);
    delete tester;
    tester=endtester;
    delete next;

    return ret_val;
  }
};

template <class G,class T> class Multi_Minimax_Goal_Test : public Successor_Goal_Test<G,T>
{
public:
  int branch;
  int depth;
  Goal_Accumulator <G,T> *other;
public:
  Multi_Minimax_Goal_Test(int d,int b,
			  Operator_Generator<T> *gen,
			  Goal_Accumulator<G,T> *mine,
			  Goal_Accumulator<G,T> *o,
			  Goal_Test<G,T> *t)
    : Successor_Goal_Test<G,T>(gen,mine,t)
  {
    depth=d;
    branch=b;
    other=o;
  }
  
  virtual G Goal_Value(T *a)
  {
    Goal_Accumulator <G,T> *old=goal_acc;
    Multi_Goal_Accumulator <G,T,greater<G> > *mga=new Multi_Goal_Accumulator<G,T,greater<G> >(branch);
    goal_acc=mga;
    Successor_Goal_Test<G,T>::Goal_Value(a);
    goal_acc=old;
    for (mga->Init();!mga->IsEnd();mga->Next()) { 
      T *state=mga->Get_State();
      Multi_Minimax_Goal_Test *next=new Multi_Minimax_Goal_Test(d-1,b,gen,other,goal_acc,tester);
      next->Goal_Value(state);
      goal_acc->Put(accumulator->value,acc->state,acc->op);
    }
    goal_acc->Put(mga->Get_State(),mga->Get_Operator(),mga->Get_Result());
    return goal_acc->Get_Value();
  }
};

#endif

