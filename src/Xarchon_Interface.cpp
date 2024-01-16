#include "Xarchon_Interface.hpp"

/* ***************************************************** */
/* Interface Function                                    */
/* ***************************************************** */

void Xarchon_AI_Computer(int *x1,int *y1,int *x2,int *y2)
{
  Xarchon_State *current_state=new Xarchon_State;
  current_state->Get_Real_State();

  Xarchon_Sequence seq(2,15,2,15);
  Xarchon_Strategy *strategy=(Xarchon_Strategy *)seq.Next_Strategy(current_state);
  strategy->InitState(current_state);

  Xarchon_Move_Operator *move_op=(Xarchon_Move_Operator *)strategy->Next();
  *x1=move_op->cmd.b.x1;
  *y1=move_op->cmd.b.y1;
  *x2=move_op->cmd.b.x2;
  *y2=move_op->cmd.b.y2;

  delete move_op;
  delete current_state;
}
  
void Field_Statistics(ACTOR *defender,ACTOR *attacker,CELL *cell,double *p,int *al,int *dl)
{

#ifdef PROFILE
  static long value=0;
  static int counter=0;
  static struct timeval timeval_s,timeval_e;
  gettimeofday(&timeval_s,NULL);
#endif


  int attacker_damage,defender_damage,attacker_hits,defender_hits;
  // This function can be either be used in Xarchon the one in computer.c or another one for the genetic trials
  // This function from the trials will used a table stored for the information (load a table and return the values)
  computer_field_score(attacker,defender,&attacker_damage,&defender_damage,&attacker_hits,&defender_hits);
#ifdef DEBUGAI
  printf ("(%s):(%d,%d) (%s):(%d,%d)\n",
	  attacker->name,attacker_hits,attacker_damage,
	  defender->name,defender_hits,defender_damage);
#endif

  int defender_hp=field_initial_life(defender,cell,attacker);
  int attacker_hp=field_initial_life(attacker,cell,defender);
  
  int defender_attack=orig_actors_list[defender->weapon].strength;
  int attacker_attack=orig_actors_list[attacker->weapon].strength;
  if (attacker_attack==0 && defender_attack==0) {
    attacker_attack=20;
    defender_attack=20;
  }
  if (defender_attack==0)
    defender_attack=attacker_attack;
  if (attacker_attack==0)
    attacker_attack=defender_attack;
  
  if (attacker_hits==0)
    attacker_hits=1;
  if (defender_hits==1)
    defender_hits=1;
  double p_attack=(double)attacker_hits/(double)(defender_hits+attacker_hits); // The chance of the attacker to hit

  int attacker_num_hits=defender_hp/attacker_attack;
  int defender_num_hits=attacker_hp/defender_attack;
#ifdef DEBUGAI
  printf ("# of hits %d,%d\n",attacker_num_hits,defender_num_hits);
#endif
  if ( (defender_hp%attacker_attack)!=0)
    attacker_num_hits++;
  if ( (attacker_hp%defender_attack)!=0)
    defender_num_hits++;

#ifdef DEBUGAI
#endif
  
  // table[i][j] -> i - defender , j attacker # of hits 
  // attacker_num_hits * i + j 
  double table[maxsize][maxsize];
  for (int i=0;i<maxsize;i++)
    for (int j=0;j<maxsize;j++)
      table[i][j]=-1;
#ifdef DEBUGAI
  printf ("p_attack :%g\n,",p_attack);
#endif
  FillTable(attacker_num_hits,defender_num_hits,p_attack,table,attacker_num_hits,defender_num_hits);
  
  // should update p - to reflect statistics
  // also should update al,dl

  double dl_d=0,al_d=0;
  // Case the defender wins
  for (int i=0;i<attacker_num_hits;i++) {
    dl_d=i*defender_attack*table[i][defender_num_hits];
  }
  for (int i=0;i<defender_num_hits;i++) {
    al_d=i*attacker_attack*table[attacker_num_hits][i];
  }
  
  *p=0;
  for (int i=0;i<defender_num_hits;i++)
    *p+=table[attacker_num_hits][i];
  //  *p=0.5;
  *al=(int)al_d;
  *dl=(int)dl_d;
  if (*al<attacker->strength)
    *al=5;
  if (*dl<defender->strength)
    *dl=5;
#ifdef DEBUGAI
  printf ("Attacker (%d:%d)(%d) : %s , Defender (%d:%d)(%d) : %s   (%g) \n",
	  attacker_hp,attacker_attack,attacker_num_hits,attacker->name,defender_hp,defender_attack,defender_num_hits,defender->name,*p);
#endif

#ifdef PROFILE
  gettimeofday(&timeval_e,NULL);
  value+=(timeval_e.tv_sec-timeval_s.tv_sec)*1000000+(timeval_e.tv_usec-timeval_s.tv_usec);
  counter++;
  printf ("Field Statistics (%ld,%d)\n",value,counter);
#endif

}

double FillTable(int cur_a,int cur_d,double p,double table[maxsize][maxsize],int max_a,int max_d)
{
  if (cur_a<0 || cur_d<0)
    return 0;
  if (cur_a==0 && cur_d==0)
    return 1;
  if (table[cur_a][cur_d]!=-1)
    return table[cur_a][cur_d];
  double val=0;
  if (cur_a==max_a && cur_d<max_d) {
    FillTable(cur_a,cur_d-1,p,table,max_a,max_d); 
    val=p*FillTable(cur_a-1,cur_d,p,table,max_a,max_d);
  }
  else
    if (cur_d==max_d && cur_a<max_a) {
      FillTable(cur_a-1,cur_d,p,table,max_a,max_d);
      val=(1-p)*FillTable(cur_a,cur_d-1,p,table,max_a,max_d);
    }
    else
      val=p*FillTable(cur_a-1,cur_d,p,table,max_a,max_d)+(1-p)*FillTable(cur_a,cur_d-1,p,table,max_a,max_d);
  table[cur_a][cur_d]=val;
#ifdef DEBUGAI
  printf ("[%d][%d]=%g",cur_a,cur_d,val);
#endif
  return val;
}


#define CHECK_ONE_GAME
#define GENETIC_RUNA

/* Genetics Main */
/* ************* */

void Xarchon_Genetics_Main(void)
{  
  printf ("Start of Genetics Main\n");
#ifdef CHECK_ONE_GAME 
  {
    Xarchon_Game game;
    Xarchon_Goal_Test light;
    Xarchon_Goal_Test dark;
    light.value_parm[POWERPOINT_SCORE]=0;
    dark.value_parm[POWERPOINT_SCORE]=0;
    light.value_parm[LIVE_HP_SCORE]=0;
    dark.value_parm[LIVE_HP_SCORE]=0;
    int i=0;
    while (true) {
      printf ("Starting Game # %d\n",i++);
      game.InitStart(1);
      game.SetPlayer(&light,&dark);
      game.Play();
    }
  } 
#endif 
  
#ifdef GENETIC_RUN 
  {
    Xarchon_Evolution evolution("testpop","evolpop",10);
    evolution.Operate();
  }
#endif
  printf ("\nEnd of Evolution\n");
}
