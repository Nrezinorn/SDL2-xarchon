#define GENETICS_MAIN
#include "main.c"

#include "main.h"
#include "computer.h"

void Xarchon_Genetics_Main(void);

int main(int argc, char *argv[])
{
  if ((void *)init == (void *)loop)
     ;
  srand(time(NULL));
  computer_config_read(NULL,NULL);
  Xarchon_Genetics_Main();
  return 0;
}
