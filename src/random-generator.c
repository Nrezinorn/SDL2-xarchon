#include <stdlib.h>

#define RANDOM_COUNT 2000

int main(void)
{
   int i;

   printf("#define RANDOM_COUNT %d\n", RANDOM_COUNT);
   printf("static int random_numbers[RANDOM_COUNT] = {\n", RANDOM_COUNT);
   for (i = 0; i < RANDOM_COUNT; i++)
      printf("   %d%s\n", random(), (i != RANDOM_COUNT - 1) ? "," : "\n};" );
   return 0;
}
