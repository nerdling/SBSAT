
#include <stdio.h>
#include "ite.h"

void makeXor(int variables, int functions, int length, int width);
void vanDerWaerden(char *vdw_type, int n, int k, int p);
void rn(char *rn_type, int n, int k, int l);

int main(int argc, char **argv) {
   if (argc > 1 && !strcmp(argv[1], "vdw")) { 
      if (argc < 6 || (argc > 2 && !strcmp(argv[2], "--help"))) {
         fprintf(stderr, "usage: %s vdw vdw_type n k p\n", argv[0]);
         fprintf(stderr, " where wdw_type is one of cnf, xcnf, cnf2, ite\n");
         fprintf(stderr, "       n is number of variables\n");
         fprintf(stderr, "       k is number of buckets\n");
         fprintf(stderr, "       p is progression size\n");
         return 0;
      }
		int n = atoi(argv[3]);
		int k = atoi(argv[4]);
		int p = atoi(argv[5]);
      vanDerWaerden(argv[2], n, k, p);
		return 0;
	} else
   if (argc > 1 && !strcmp(argv[1], "vdw")) { 
      if (argc < 6 || (argc > 2 && !strcmp(argv[2], "--help"))) {
         fprintf(stderr, "usage: %s xor #vars #functions length max_width\n", argv[0]);
         return 0;
      }
      int variables = atoi(argv[2]);
      int functions = atoi(argv[3]);
		int length = atoi(argv[4]);
		int width = atoi(argv[5]);
      makeXor(variables, functions, length, width);
		return 0;
	} else
   if (argc > 1 && !strcmp(argv[1], "rn")) { 
      if (argc < 5 || (argc > 2 && !strcmp(argv[2], "--help"))) {
         fprintf(stderr, "usage: %s rn n k l\n", argv[0]);
         fprintf(stderr, "       n - vertices\n");
         fprintf(stderr, "       k - aquatances\n");
         fprintf(stderr, "       l - strangers\n");
         return 0;
      }
      int n = atoi(argv[2]);
      int k = atoi(argv[3]);
		int l = atoi(argv[4]);
      rn("cnf", n, k, l);
		return 0;
	}

	fprintf(stderr, "usage: %s xor --help\n", argv[0]);
	fprintf(stderr, "usage: %s vdw --help\n", argv[0]);
	fprintf(stderr, "usage: %s rn --help\n", argv[0]);
	return 0;
}
