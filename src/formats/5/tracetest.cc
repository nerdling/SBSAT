#include "ite.h"
#include "bddnode.h"

BDDNode **xorFunctions;
char filename[128]="stdin";
extern int s_error;

//#define IN trace_in
//#define PARSE trace_parse

//#define IN blif_in
//#define PARSE blif_parse

#define IN prover_in
#define PARSE prover_parse

//#define IN prover3_in
//#define PARSE prover3_parse

//#define IN iscas_in
//#define PARSE iscas_parse

extern FILE *IN;
int PARSE();

int main(int argc, char **argv)
{
   //numinp=500000;
   //numout=500000;
   //vars_alloc(numinp);
   //functions_alloc(numout);
   bdd_init();
   sym_init();
   parser_init();

   ++argv, --argc;  /* skip over program name */
   if ( argc > 0 ) {
      while (s_error==0 && argc > 0) {
         strncpy(filename, argv[0], 127);
         printf("Reading: %s\n", filename); fflush(stdout);
         if ( (IN = fopen( filename, "r" )) != NULL) {
            //printf(".");
            PARSE();
            fclose(IN);
         }
         ++argv, --argc;  
      }
   }
   else
   {
      IN = stdin;
      PARSE();
   }
   numout = functions_max;
   for (int i=0;i<numout;i++)
   {
      if (functions[i]) {
         printBDDfile(functions[i], stddbg);
         fprintf(stddbg, "\n");
      }
   }
   //bddnode_free();
   return 0;
}

