/* =========FOR INTERNAL USE ONLY. NO DISTRIBUTION PLEASE ========== */

/*********************************************************************
 Copyright 1999-2003, University of Cincinnati.  All rights reserved.
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
/* variable size (growing) overlapping buckets */
/* parallel buckets */

#include "ite.h"

typedef struct {
   BDDNode *memory;
   int bucket_overlap;
   int sizeBuckets;
   int numBuckets;
   int memorySize;
   int max;
} bdd_pool_type;

/* FIXME: number of pools hardcoded to 100 */
bdd_pool_type bddmemory_vsb[100];
int bddmemory_vsb_index=0;
int numBDDPool=0;
int curBDDPos =0;
BDDNode **hash_memory = NULL;
int hash_memory_size = 0;
int hash_memory_mask = 0;
BDDNode *bddtable_free = NULL;

void bdd_gc();
void bdd_fix_inferences(BDDNode *node);

void bdd_bdd_alloc_pool(int pool)
{
   if (pool >= 100)
   {
      dE_printf1("Increase the number of BDD Pools\n");
      exit(1);
   }
   bddmemory_vsb[pool].max = _bdd_pool_size; 
   bddmemory_vsb[pool].memory = (BDDNode*)ite_calloc(bddmemory_vsb[pool].max, sizeof(BDDNode), 2, "bdd memory pool");
}

void FreeInferencePool();

void
bdd_bdd_free_pools()
{
   int i;
   for (i=0;i<=numBDDPool;i++) {
      free(bddmemory_vsb[i].memory);
   }
   FreeInferencePool();
   free(hash_memory);
}

void
bddvsb_init()
{ 
   /* allocate the first pool */
   bdd_bdd_alloc_pool(0);
   numBDDPool = 0;
   curBDDPos  = 2;

   hash_memory_size = (1 << (numBuckets+sizeBuckets));
   hash_memory_mask = (1 << (numBuckets+sizeBuckets))-1;
   hash_memory = (BDDNode**)ite_calloc(hash_memory_size, sizeof(BDDNode*),
         2, "hash_memory");

   if (false_ptr == NULL && true_ptr == NULL)
   {
      BDDNode * next = bddmemory_vsb[0].memory;
      false_ptr = next;
      false_ptr->variable = -1;
      false_ptr->thenCase = false_ptr;
      false_ptr->elseCase = false_ptr;
      false_ptr->inferences = NULL;
      next++;
      true_ptr = next;
      true_ptr->variable = -1;
      true_ptr->thenCase = true_ptr;
      true_ptr->elseCase = true_ptr;
      true_ptr->inferences = NULL;
   }
}

inline int
hash_fn(int v, BDDNode *r, BDDNode *e)
{
   return (v + (*(int*)&r) + (*(int*)&e)) & hash_memory_mask;
}

BDDNode * 
bddvsb_find_or_add_node (int v, BDDNode * r, BDDNode * e)
{
   ite_counters[BDD_NODE_FIND]++;

   assert(v >= r->variable && v >= e->variable);
   if (DEBUG_LVL&32) {
      printf("%d ", v);
      if (r==true_ptr) printf(" true "); 
      else
         if (r==false_ptr) printf(" false "); else
            printf(" x ");
      if (e==true_ptr) printf(" true "); 
      else
         if (e==false_ptr) printf(" false "); else
            printf(" x ");
      printf("\n");
   }

   /* test */
#ifdef MK_NULL
   int t_r, t_e;
   if (r == false_ptr) t_r = 0;
   else if (r == true_ptr) t_r = 1;
   else t_r = 2;

   if (e == false_ptr) t_e = 0;
   else if (e == true_ptr) t_e = 1;
   else t_e = 2; 

   symrec *ptr=getsym_i(v);
   if (ptr && t_e != 2 && t_r != 2) {
      assert (t_e != t_r);
      /* return predefined bddnode 
       * t f
       * f t */
      BDDNode *curr;
      if (t_r == 1) curr = &(ptr->true_false);
      else curr = &(ptr->false_true);
      GetInferFoAN(curr); //Get Inferences
      return curr;
   }
#endif

   //int hash_pos = (v + (*(int*)&r)/sizeof(BDDNode) + (*(int*)&e)/sizeof(BDDNode)) & hash_memory_mask;
   //int hash_pos = (v^(*(int*)&r)^(*(int*)&e)) & hash_memory_mask; // slow!
   //int hash_pos = (v + (*(int*)&r)>>2 + (*(int*)&e)>>2) & hash_memory_mask;
   //int hash_pos = (v + (*(int*)&r) + (*(int*)&e)) & hash_memory_mask;
   int hash_pos = hash_fn(v, r, e);

   BDDNode **node = hash_memory+hash_pos;
   BDDNode **prev = NULL; 
   ite_counters[BDD_NODE_STEPS]++;

   while (!(*node == NULL || ((*node)->variable == v && (*node)->thenCase == r && (*node)->elseCase == e)))
   {
      prev = node;
      node = &((*node)->next);
      ite_counters[BDD_NODE_STEPS]++;
   }

   if (*node != NULL) {
      /* if this node has one in front of it in the chain
       * swap it so the next time this node needs less steps */

		if (prev && prev != node) {
			
         BDDNode *tmp = *prev;
         *prev = *node;
         *node = tmp;

         BDDNode *tmp_next = (*prev)->next;

         (*prev)->next = *node;
         (*node)->next = tmp_next;

         node = prev;
      }

	} else {
      /* could not find the node => allocate new one */
      ite_counters[BDD_NODE_NEW]++;
      /* not safe
      if (bddtable_free == NULL && bddmemory_vsb[numBDDPool].max == curBDDPos) {
         bdd_gc();
         node = hash_memory+hash_pos;
         while (!(*node == NULL)) node = &((*node)->next);
      }
      */
      if (bddtable_free != NULL) {
         (*node) = bddtable_free;
         bddtable_free = bddtable_free->next;
         (*node)->next = NULL;
      } else
      {
         if (bddmemory_vsb[numBDDPool].max == curBDDPos) {
            ++numBDDPool;
            bdd_bdd_alloc_pool(numBDDPool);
            curBDDPos = 0;
         }
         (*node) = bddmemory_vsb[numBDDPool].memory+curBDDPos++;
      }
      (*node)->variable = v;
      (*node)->thenCase = r;
      (*node)->elseCase = e;
      GetInferFoAN(*node); //Get Inferences
   }
   return *node;
}

void
bdd_flag_nodes(BDDNode *node)
{
   if (node->flag == 1) return;
   node->flag = 1;
   //node->inferences = NULL;
   bdd_flag_nodes(node->thenCase);
   bdd_flag_nodes(node->elseCase);
}


// BDD Garbage collection
void
bdd_gc()
{
   int i,j;

   // don't do it if there are free nodes
   if (bddtable_free != NULL) return;

#ifndef NDEBUG
   d3_printf1("BDD_GC START\n");
   int totalin=0, totalout=0;
   // count free nodes -- statistics -- can be removed
   BDDNode *p = bddtable_free;
   while (p!=NULL) {
      totalin++;
      p = p->next;
   }
#endif


   // clean all flags
   for (i=0;i<=numBDDPool;i++)
   {
      int max = bddmemory_vsb[i].max;
      if (i == numBDDPool) max = curBDDPos;
      for (j=0;j<max;j++) 
         (bddmemory_vsb[i].memory+j)->flag = 0;
   }

   // flag all referenced nodes
   true_ptr->flag = 1;
   false_ptr->flag = 1;
   for (i=0;i<nmbrFunctions;i++)
   {
     // printBDDInfs(functions[i]);
     // fprintf(stddbg, "\n");
      bdd_flag_nodes(functions[i]);
   }
   for (i=0;i<original_numout;i++)
   {
      bdd_flag_nodes(original_functions[i]);
   }

   
   // deallocate infereces
   //FreeInferencePool(); 
   

   // clean the hash table
   for (i=0;i<=hash_memory_mask; i++)
   {
      hash_memory[i] = NULL;
   }

   // remove unreferenced and rehash referenced
   bddtable_free = NULL;
   for (i=0;i<=numBDDPool;i++)
   {
      int max = bddmemory_vsb[i].max;
      if (i == numBDDPool) max = curBDDPos;
      for (j=0;j<max;j++)
      {
         BDDNode *node = (bddmemory_vsb[i].memory+j);
         if (node->flag == 0)
         {
            // deleted 
            DeallocateInferences_var(node->inferences, node->variable);
            memset(node, 0, sizeof(BDDNode));
            node->next = bddtable_free;
            bddtable_free = node;
         } else
         {
            // rehash
            int hash_pos = hash_fn(node->variable, node->thenCase, node->elseCase);
            BDDNode **hash_node = hash_memory+hash_pos;
            node->next = *hash_node;;
            *hash_node = node;
         }
      }
   }
/*
   for (i=0;i<nmbrFunctions;i++)
   {
      bdd_fix_inferences(functions[i]);
   }
   for (i=0;i<original_numout;i++)
   {
      bdd_fix_inferences(original_functions[i]);
   }
   */
/*
   fprintf(stddbg, "out:\n");
   for (i=0;i<nmbrFunctions;i++)
   {
      printBDDInfs(functions[i]);
      fprintf(stddbg, "\n");
   }
*/
#ifndef NDEBUG
   // count free nodes -- statistics -- can be removed
   p = bddtable_free;
   while (p!=NULL) {
      totalout++;
      p = p->next;
   }
   d3_printf4("BDD_GC %d -> %d (%.02f%%)\n", 
         totalin, totalout, totalin==0?0:1.0*(totalout-totalin)/totalin);
#endif
   
}


void
bdd_fix_inferences(BDDNode *node)
{
   if (node == true_ptr || node == false_ptr) return;
   if (node->inferences != NULL) return;
   bdd_fix_inferences(node->thenCase); 
   bdd_fix_inferences(node->elseCase); 
   GetInferFoAN(node);
}

typedef struct {
   void *bddtable_start;
   int   bddtable_pos;
   int   num_variables;
   int   num_functions;
} t_sbsat_env;

FILE *
ite_fopen(char *filename, char *fileflags)
{
   FILE *f = fopen(filename, fileflags);
   if (!f) {
      cerr << "Can't open " << filename << endl;
   }
   return f;
}

long
ite_filesize(char *filename)
{
   struct stat buf;
   if (stat(filename, &buf) != 0) {
      /* error in errno */
      return 0;
   };
   return buf.st_size;
}

void
read_bdd_circuit()
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

   cerr << "Reading sbsatenv .. " << endl;
   assert(ite_filesize("sbsatenv.bin") == sizeof(t_sbsat_env));
   fin = fopen("sbsatenv.bin", "rb");
   if (!fin) return;
   fread(&sbsat_env, sizeof(t_sbsat_env), 1, fin);
   fclose(fin);

   _numinp = sbsat_env.num_variables;
   _numout = sbsat_env.num_functions;
   _bddtable_len = sbsat_env.bddtable_pos;
   _bddtable_start = sbsat_env.bddtable_start;


   cerr << "Reading bddtable .. " << endl;
   assert(_bddtable_len == (int)(ite_filesize("bddtable.bin")/sizeof(BDDNode)));
   fin = ite_fopen("bddtable.bin", "rb");
   if (!fin) return;
   _bddtable = calloc(_bddtable_len, sizeof(BDDNode));
   fread(_bddtable, sizeof(BDDNode), _bddtable_len, fin);
   fclose(fin);

   cerr << "Reading functions .. " << endl;
   assert(_numout == (int)(ite_filesize("functions.bin")/sizeof(BDDNode*)));
   fin = ite_fopen("functions.bin", "rb");
   if (!fin) return;
   _functions = calloc(_numout, sizeof(BDDNode*));
   fread(_functions, sizeof(BDDNode*), _numout , fin);
   fclose(fin);

   cerr << "Reading function types .. " << endl;
   assert(_numout == (int)(ite_filesize("functiontype.bin")/sizeof(int)));
   fin = ite_fopen("functiontype.bin", "rb");
   if (!fin) return;
   _functiontype = calloc(_numout, sizeof(int));
   fread(_functiontype, sizeof(int), _numout , fin);
   fclose(fin);

   cerr << "Reading equality Vble.. " << endl;
   assert(_numout == (int)(ite_filesize("equalityvble.bin")/sizeof(int)));
   fin = ite_fopen("equalityvble.bin", "rb");
   if (!fin) return;
   _equalityvble = calloc(_numout, sizeof(int));
   fread(_equalityvble, sizeof(int), _numout , fin);
   fclose(fin);

   cerr << "Reading length .. " << endl;
   assert(_numout == (int)(ite_filesize("length.bin")/sizeof(int)));
   fin = ite_fopen("length.bin", "rb");
   if (!fin) return;
   _length = calloc(_numout, sizeof(int));
   fread(_length, sizeof(int), _numout , fin);
   fclose(fin);

   cerr << "Reading variablelist .. " << endl;
   assert(_numinp+1 == (int)(ite_filesize("variablelist.bin")/sizeof(varinfo)));
   fin = ite_fopen("variablelist.bin", "rb");
   _variablelist = calloc(_numinp+1, sizeof(varinfo));
   if (!fin) return;
   fread(_variablelist, sizeof(varinfo), _numinp+1 , fin);
   fclose(fin);

   cerr << "Reading independantVars .. " << endl;
   assert(_numinp+1 == (int)(ite_filesize("independantVars.bin")/sizeof(int)));
   fin = ite_fopen("independantVars.bin", "rb");
   _independantVars = calloc(_numinp+1, sizeof(int));
   if (!fin) return;
   fread(_variablelist, sizeof(int), _numinp+1 , fin);
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
   d2_printf3("Have %ld variables and %ld functions .. \n",
                   numinp, numout);

   d2_printf1("BDD and Circuit Init..\n");
   numinp = _numinp;
   numout = _numout;
   vars_alloc(numinp+1);
   functions_alloc(numout);
   nmbrFunctions = numout;
   curBDDPos = _bddtable_len;
	length = new int[numout + 1];
   variablelist = new varinfo[numinp + 1];
   independantVars = (int *)calloc(numinp + 1, sizeof(int));

   assert(bddmemory_vsb[0].max > curBDDPos);
   memmove(bddmemory_vsb[0].memory, _bddtable, sizeof(BDDNode)*curBDDPos);
   memmove(functions, _functions, sizeof(BDDNode*)*numout);
   memmove(functionType, _functiontype, sizeof(int)*numout);
   memmove(equalityVble, _equalityvble, sizeof(int)*numout);
   memmove(length, _length, sizeof(int)*numout);
   memmove(variablelist, _variablelist, sizeof(varinfo)*(numinp+1));
   memmove(independantVars, _independantVars, sizeof(int)*(numinp+1));

   int shift = (char*)(bddmemory_vsb[0].memory) - (char*)(_bddtable_start);

   /* fix functions */
   d2_printf1("Fixing functions .. \n");
   int i;
   for (i=0;i<numout;i++) {
      if (functions[i]) functions[i] = (BDDNode*)((char*)(functions[i])+shift);
   }

   /* fix bdd table */
   d2_printf1("Fixing bdd table .. \n");
   for (i=0;i<curBDDPos;i++) {
      bddmemory_vsb[0].memory[i].thenCase = (BDDNode*)
         ((char*)bddmemory_vsb[0].memory[i].thenCase + shift);
      bddmemory_vsb[0].memory[i].elseCase = (BDDNode*)
         ((char*)bddmemory_vsb[0].memory[i].elseCase + shift);
      bddmemory_vsb[0].memory[i].next = NULL;

      /* fix bdd hash table */
      int v = bddmemory_vsb[0].memory[i].variable;
      BDDNode * r = bddmemory_vsb[0].memory[i].thenCase;
      BDDNode * e = bddmemory_vsb[0].memory[i].elseCase;
      int hash_pos = (v + (*(int*)&r) + (*(int*)&e)) & hash_memory_mask;
      BDDNode **node = hash_memory+hash_pos;
      bddmemory_vsb[0].memory[i].next = *node;
      bddmemory_vsb[0].memory[i].inferences = NULL;
      *node = bddmemory_vsb[0].memory + i;
   }

   d2_printf1("Fixing bdd table inferences .. \n");
   GetInferFoAN(bddmemory_vsb[0].memory+0);
   GetInferFoAN(bddmemory_vsb[0].memory+1);
   for (i=2;i<curBDDPos;i++) {
      if (bddmemory_vsb[0].memory[i].inferences == NULL)
         bdd_fix_inferences(bddmemory_vsb[0].memory+i);
   }
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
  *_bddtable = bddmemory_vsb[0].memory;
  *_bddtable_len = curBDDPos;
  *_bddtable_msize = sizeof(BDDNode);
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
dump_bdd_table()
{
   cerr << "dump_bdd_table ---------------------------------" << endl;
   if (numBDDPool) {
      cerr << "Can't dump split BDD pool -- please increase bdd-pool-size" << endl;
      return;
   }
   FILE *fout = NULL;

   fout = ite_fopen("bddtable.bin", "wb");
   if (!fout) return;
   fwrite(bddmemory_vsb[0].memory, sizeof(BDDNode), curBDDPos, fout);
   fclose(fout);

   fout = ite_fopen("functions.bin", "wb");
   if (!fout) return;
   fwrite(functions, sizeof(BDDNode*), numout , fout);
   fclose(fout);

   fout = ite_fopen("functiontype.bin", "wb");
   if (!fout) return;
   fwrite(functionType, sizeof(int), numout , fout);
   fclose(fout);

   fout = ite_fopen("equalityvble.bin", "wb");
   if (!fout) return;
   fwrite(equalityVble, sizeof(int), numout , fout);
   fclose(fout);

   fout = ite_fopen("length.bin", "wb");
   if (!fout) return;
   fwrite(length, sizeof(int), numout , fout);
   fclose(fout);

   fout = ite_fopen("variablelist.bin", "wb");
   if (!fout) return;
   fwrite(variablelist, sizeof(varinfo), numinp+1 , fout);
   fclose(fout);

   fout = ite_fopen("independantVars.bin", "wb");
   if (!fout) return;
   fwrite(independantVars, sizeof(int), numinp+1 , fout);
   fclose(fout);

   t_sbsat_env sbsat_env;
   sbsat_env.bddtable_start = bddmemory_vsb[0].memory;
   sbsat_env.bddtable_pos = curBDDPos;
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
