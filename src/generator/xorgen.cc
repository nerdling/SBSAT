
#include <stdio.h>
#include <sys/time.h>
#include "ite.h"

void makeXor(int variables, int functions, int length, int width) {
	struct timeval tv;
	int *pick = new int[length+(length/4)];
	gettimeofday(&tv, NULL);
	srand(tv.tv_usec);
	fprintf(stdout, "p xor %d %d\n", variables, functions);
	for(int x = 0; x < functions; x++) {
		int var;
      int y;
      /* pick variables */
		for(y = 0; y < (length+(length/4)); y++) {
			pick[y] = (rand() % variables) + 1;
		}
      /* linear part */
      int lin_max = 6*length/7;
      for(y = 0; y < lin_max; y++) {
				var = pick[(rand() % (length+(length/7)))];
				fprintf(stdout, "x%d ", var);
      }
      /* non-linear part */
		for(/*cont*/; y < length; y++) {
			int rand_width = (rand() % width) + 1;
			for(int z = 0; z < rand_width; z++) {
				var = pick[(rand() % (length+(length/7)))];
				fprintf(stdout, "x%d", var);
			}
			fprintf(stdout, " ");
		}
		int equalvar = rand() % 2;
		fprintf(stdout, "= %d\n", equalvar);
	}
   delete [] pick;
}

