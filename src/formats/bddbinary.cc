
/* =========FOR INTERNAL USE ONLY. NO DISTRIBUTION PLEASE ========== */

/*********************************************************************
 Copyright 1999-2007, University of Cincinnati.  All rights reserved.
 By using this software the USER indicates that he or she has read,
 understood and will comply with the following:

 --- University of Cincinnati hereby grants USER nonexclusive permission
 to use, copy and/or modify this software for internal, noncommercial,
 research purposes only. Any distribution, including commercial sale
 or license, of this software, copies of the software, its associated
 documentation and/or modifications of either is strictly prohibited
 without the prior consent of University of Cincinnati.  Title to copyright
 to this software and its associated documentation shall at all times
 remain with University of Cincinnati.  Appropriate copyright notice shall
 be placed on all software copies, and a complete copy of this notice
 shall be included in all copies of the associated documentation.
 No right is  granted to use in advertising, publicity or otherwise
 any trademark,  service mark, or the name of University of Cincinnati.


 --- This software and any associated documentation is provided "as is"

 UNIVERSITY OF CINCINNATI MAKES NO REPRESENTATIONS OR WARRANTIES, EXPRESS
 OR IMPLIED, INCLUDING THOSE OF MERCHANTABILITY OR FITNESS FOR A
 PARTICULAR PURPOSE, OR THAT  USE OF THE SOFTWARE, MODIFICATIONS, OR
 ASSOCIATED DOCUMENTATION WILL NOT INFRINGE ANY PATENTS, COPYRIGHTS,
 TRADEMARKS OR OTHER INTELLECTUAL PROPERTY RIGHTS OF A THIRD PARTY.

 University of Cincinnati shall not be liable under any circumstances for
 any direct, indirect, special, incidental, or consequential damages
 with respect to any claim by USER or any third party on account of
 or arising from the use, or inability to use, this software or its
 associated documentation, even if University of Cincinnati has been advised
 of the possibility of those damages.
 *********************************************************************/

#include "sbsat.h"
#include "sbsat_formats.h"


typedef struct {
   void *bddtable_start;
   int   bddtable_pos;
   int   num_variables;
   int   num_functions;
} t_sbsat_env;

void
Binary_to_BDD()
{
   FILE *fin = NULL;
   t_sbsat_env sbsat_env;
   void *_bddtable = NULL;
   void *_bddtable_start = NULL;
   int   _bddtable_len = 0;
   void *_equalityvble = NULL; 
   void *_functions = NULL;
   void *_functiontype = NULL; 
   void *_length = NULL;
   void *_variablelist = NULL;
   void *_independantVars = NULL;
   int _numinp = 0;
   int _numout = 0;
   char tmp_str[32];

   strcpy(tmp_str, "sbsatenv.bin");
   cerr << "Reading " << tmp_str << endl;
   assert(ite_filesize(tmp_str) == sizeof(t_sbsat_env));
   fin = fopen(tmp_str, "rb");
   if (!fin) return;
   fread(&sbsat_env, sizeof(t_sbsat_env), 1, fin);
   fclose(fin);

   _numinp = sbsat_env.num_variables;
   _numout = sbsat_env.num_functions;
   _bddtable_len = sbsat_env.bddtable_pos;
   _bddtable_start = sbsat_env.bddtable_start;


   strcpy(tmp_str, "bddtable.bin");
   cerr << "Reading " << tmp_str << endl;
   assert(_bddtable_len == (int)(ite_filesize(tmp_str)/sizeof(BDDNode)));
   fin = ite_fopen(tmp_str, "rb");
   if (!fin) return;
   _bddtable = calloc(_bddtable_len, sizeof(BDDNode));
   if (fread(_bddtable, sizeof(BDDNode), _bddtable_len, fin) != (size_t)_bddtable_len)
      fprintf(stderr, "fread on bddtable failed\n");
   fclose(fin);

   strcpy(tmp_str, "functions.bin");
   cerr << "Reading " << tmp_str << endl;
   assert(_numout == (int)(ite_filesize(tmp_str)/sizeof(BDDNode*)));
   fin = ite_fopen(tmp_str, "rb");
   if (!fin) return;
   _functions = calloc(_numout, sizeof(BDDNode*));
   if (fread(_functions, sizeof(BDDNode*), _numout , fin) != (size_t)_numout)
      fprintf(stderr, "fread on functions failed\n");
   fclose(fin);

   strcpy(tmp_str, "functiontype.bin");
   cerr << "Reading " << tmp_str << endl;
   assert(_numout == (int)(ite_filesize(tmp_str)/sizeof(int)));
   fin = ite_fopen(tmp_str, "rb");
   if (!fin) return;
   _functiontype = calloc(_numout, sizeof(int));
   if (fread(_functiontype, sizeof(int), _numout , fin) != (size_t)_numout)
      fprintf(stderr, "fread on functiontype failed\n");
   fclose(fin);

   strcpy(tmp_str, "equalityvble.bin");
   cerr << "Reading " << tmp_str << endl;
   assert(_numout == (int)(ite_filesize(tmp_str)/sizeof(int)));
   fin = ite_fopen(tmp_str, "rb");
   if (!fin) return;
   _equalityvble = calloc(_numout, sizeof(int));
   if (fread(_equalityvble, sizeof(int), _numout , fin) != (size_t)_numout)
      fprintf(stderr, "fread on equalityvble failed\n");
   fclose(fin);

   strcpy(tmp_str, "length.bin");
   cerr << "Reading " << tmp_str << endl;
   assert(_numout == (int)(ite_filesize(tmp_str)/sizeof(int)));
   fin = ite_fopen(tmp_str, "rb");
   if (!fin) return;
   _length = calloc(_numout, sizeof(int));
   if (fread(_length, sizeof(int), _numout , fin) != (size_t)_numout)
      fprintf(stderr, "fread on length failed\n");
   fclose(fin);

   strcpy(tmp_str, "variablelist.bin");
   cerr << "Reading " << tmp_str << endl;
   assert(_numinp+1 == (int)(ite_filesize(tmp_str)/sizeof(varinfo)));
   fin = ite_fopen(tmp_str, "rb");
   _variablelist = calloc(_numinp+1, sizeof(varinfo));
   if (!fin) return;
   if (fread(_variablelist, sizeof(varinfo), _numinp+1 , fin) != (size_t)_numinp+1)
      fprintf(stderr, "fread on variablelist failed\n");
   fclose(fin);

   strcpy(tmp_str, "independantVars.bin");
   cerr << "Reading " << tmp_str << endl;
   assert(_numinp+1 == (int)(ite_filesize(tmp_str)/sizeof(int)));
   fin = ite_fopen(tmp_str, "rb");
   _independantVars = calloc(_numinp+1, sizeof(int));
   if (!fin) return;
   if (fread(_independantVars, sizeof(int), _numinp+1 , fin) != (size_t)_numinp+1)
      fprintf(stderr, "fread on independantVars failed\n");
   fclose(fin);

   reload_bdd_circuit(_numinp, _numout, 
                      _bddtable, _bddtable_len,
                      _bddtable_start, 
                      _equalityvble, _functions,
                      _functiontype, _length,
                      _variablelist, _independantVars);
   ite_free(&_bddtable);
   ite_free(&_equalityvble);
   ite_free(&_functions);
   ite_free(&_functiontype);
   ite_free(&_length);
   ite_free(&_variablelist);
   ite_free(&_independantVars);
}

void
reload_bdd_circuit(int _numinp, int _numout,
                   void *_bddtable, int _bddtable_len,
                   void *_bddtable_start,
                   void *_equalityvble, 
                   void *_functions, 
                   void *_functiontype, 
                   void *_length, 
                   void *_variablelist,
                   void *_independantVars)
{
   bdd_init();
   d2_printf3("Have %d variables and %d functions .. \n",
                   _numinp, _numout);

   d2_printf1("BDD and Circuit Init..\n");
   numinp = _numinp;
   numout = _numout;
   vars_alloc(numinp+1);
   functions_alloc(numout);
   nmbrFunctions = numout;
   ite_free((void**)&length);
   length = (int *)ite_calloc(numout+1, sizeof(int), 9, "length");
   if (variablelist) delete [] variablelist;
   variablelist = new varinfo[numinp + 1];

   int shift=0;
   bddtable_load(_bddtable, _bddtable_len, _bddtable_start, &shift);
   memmove(functions, _functions, sizeof(BDDNode*)*numout);
   memmove(functionType, _functiontype, sizeof(int)*numout);
   memmove(equalityVble, _equalityvble, sizeof(int)*numout);
   memmove(length, _length, sizeof(int)*numout);
   memmove(variablelist, _variablelist, sizeof(varinfo)*(numinp+1));
   memmove(independantVars, _independantVars, sizeof(int)*(numinp+1));

   /* fix functions */
   d2_printf1("Fixing functions .. \n");
   int i;
   for (i=0;i<numout;i++) {
      if (functions[i]) functions[i] = (BDDNode*)((char*)(functions[i])+shift);
   }
   create_all_syms(_numinp);
}        

void
get_bdd_circuit(int *_numinp, int *_numout,
                   void **_bddtable, int *_bddtable_len, int *_bddtable_msize,
                   void **_equalityvble, 
                   void **_functions,  int *_functions_msize,
                   void **_functiontype, 
                   void **_length, 
                   void **_variablelist, int *_variablelist_msize,
                   void **_independantVars)
{
  *_numinp = numinp;
  *_numout = numout;
  bddtable_get(_bddtable, _bddtable_len, _bddtable_msize);
  *_equalityvble = equalityVble;
  *_functions = functions;
  *_functions_msize = sizeof(BDDNode*);
  *_functiontype = functionType;
  *_length = length;
  *_variablelist = variablelist; 
  *_variablelist_msize = sizeof(varinfo);
  *_independantVars = independantVars; 
}
        
void
BDD_to_Binary()
{
   void *_bddtable;
   int _bddtable_len;
   int _bddtable_msize;
   cerr << "dump_bdd_table ---------------------------------" << endl;
   FILE *fout = NULL;
   char tmp_str[32];

   strcpy(tmp_str, "bddtable.bin");
   fout = ite_fopen(tmp_str, "wb");
   if (!fout) return;
   bddtable_get(&_bddtable, &_bddtable_len, &_bddtable_msize);
   fwrite(_bddtable, _bddtable_msize, _bddtable_len, fout);
   fclose(fout);

   strcpy(tmp_str, "functions.bin");
   fout = ite_fopen(tmp_str, "wb");
   if (!fout) return;
   fwrite(functions, sizeof(BDDNode*), numout , fout);
   fclose(fout);

   strcpy(tmp_str, "functiontype.bin");
   fout = ite_fopen(tmp_str, "wb");
   if (!fout) return;
   fwrite(functionType, sizeof(int), numout , fout);
   fclose(fout);

   strcpy(tmp_str, "equalityvble.bin");
   fout = ite_fopen(tmp_str, "wb");
   if (!fout) return;
   fwrite(equalityVble, sizeof(int), numout , fout);
   fclose(fout);

   strcpy(tmp_str, "length.bin");
   fout = ite_fopen(tmp_str, "wb");
   if (!fout) return;
   fwrite(length, sizeof(int), numout , fout);
   fclose(fout);

   strcpy(tmp_str, "variablelist.bin");
   fout = ite_fopen(tmp_str, "wb");
   if (!fout) return;
   fwrite(variablelist, sizeof(varinfo), numinp+1 , fout);
   fclose(fout);

   strcpy(tmp_str, "independantVars.bin");
   fout = ite_fopen(tmp_str, "wb");
   if (!fout) return;
   fwrite(independantVars, sizeof(int), numinp+1 , fout);
   fclose(fout);

   t_sbsat_env sbsat_env;
   sbsat_env.bddtable_start = _bddtable;
   sbsat_env.bddtable_pos = _bddtable_len;
   sbsat_env.num_variables = numinp;
   sbsat_env.num_functions = numout;
   fout = fopen("sbsatenv.bin", "wb");
   if (!fout) return;
   fwrite(&sbsat_env, sizeof(t_sbsat_env), 1, fout);
   fclose(fout);



   /*
   for (char* i=bddmemory_vsb[0].memory;i<bddmemory_vsb[i].memory+curBDDPos;i++)
   cout << "Pool is starting at " << bddmemory_vsb[0].memory << endl;
   for (int i=0;i<curBDDPos;i++) {
      BDDNode *bdd = (bddmemory_vsb[0].memory + i);

      cout << "var: " << bdd->variable << endl;
      cout << "then: " << bdd->thenCase << " idx: " << bdd->thenCase-bddmemory_vsb[0].memory << endl;
      cout << "else: " << bdd->elseCase << " idx: " << bdd->elseCase-bddmemory_vsb[0].memory << endl;
      // have to fix next anyway 
      // cout << "next: " << next << endl;

      // must be 0 (NULL)
      //int t_var;
      //void *var_ptr;
      //infer *inferences;
      //SmurfFactoryAddons *addons;
   }
   */
}
