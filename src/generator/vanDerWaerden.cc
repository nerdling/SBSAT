#include <stdio.h>
#include <sys/time.h>
#include "ite.h"

void vanDerWaerden(int n, int k, int p) {
	fprintf(stdout, "p cnf %d %d\n", n*k, n+(n*((n-1)/(p-1))));
	for(int x = 1; x <= n; x++) {
		fprintf(stdout, "#1 [ %d ", x);
		for(int y = 1; y < k; y++)
		  fprintf(stdout, "%d ", (y*n)+x);
		fprintf(stdout, "] 1\n");
	}
	
	for(int x = 1; x < n; x++) {
		for(int y = 1; y <= (n-1)/(p-1); y++) {
			int tmp = ((x-1)*n)+1;
			for(int z = 1; z <= p; z++) {
				fprintf(stdout, "-%d ", tmp);
				tmp+=y;
			}
			fprintf(stdout, "0\n");
		}
	}
}

