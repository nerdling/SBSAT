
#include <stdio.h>
#include "ite.h"

void makeXor(int variables, int functions, int length, int width);
void vanDerWaerden(int n, int k, int p);

int main(int argc, char **argv) {
	if(argc == 4) {
		int n = atoi(argv[1]);
		int k = atoi(argv[2]);
		int p = atoi(argv[3]);
		vanDerWaerden(n, k, p);
		return 0;
	}
	if(argc == 5) {
		int variables = atoi(argv[1]);
		int functions = atoi(argv[2]);
		int length = atoi(argv[3]);
		int width = atoi(argv[4]);
		makeXor(variables, functions, length, width);
		return 0;
	}

	fprintf(stderr, "usage: %s #vars #functions length max_width\n", argv[0]);
	fprintf(stderr, "usage: %s n k p\n", argv[0]);
	return 0;
}
