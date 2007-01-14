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
 any trademark, service mark, or the name of University of Cincinnati.


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

// ******************************************************* ~/kernel/bdd_tool.h
//
//  W. Mark Vanfleet, NSA, C12, August 19, 2000
//  Laura A. Pugh, G&T Summer Intern, June-July-August-2000
//
//  State Machine Used to Represent Functions (SMURF)
// 
//  Version 1.0 August 19, 2000
//
// ***************************************************************************

#include "sbsat.h"
#include "sbsat_solver.h"

#include <time.h>

#define mid_most 0

#define VALUE_FLAG // to enable keeping score for t and e paths
//#define TRACE_FLAG // to enable trace feature

//#define ZERO_ONE_VAR 65535
#define ZERO_ONE_VAR ((1 << 20) - 1)

#undef T

// defines to control memory management
#define max_bdd (1 << 18)     // BDD DAG must be a power of 2
#define max_sm (1 << 21)      // SM DAG must be a power of 2
//#define score_const_1 2.6     // Johnson Heuristic Parameter 1
//#define score_const_2 2.6     // Johnson Heuristic Parameter 2
#define score_const_1 JHEURISTIC_K     // Johnson Heuristic Parameter 1
#define score_const_2 JHEURISTIC_K     // Johnson Heuristic Parameter 2
#define max_vars_per_smurf 10 // Define max complexity of equations
#define max_vars 3000         // max number of user defined variables
#define max_smurfs 100000     // max number of equations or constraints

// some constants
#define max_bdd_mask (max_bdd - 1)
#define max_vars_list_space (max_smurfs * max_vars_per_smurf)

struct BinaryDD {  
  int v;                // top variable in BDD
  int T;                // index into BDD DAG for true branch of BDD
  int E;                // index into BDD DAG for false branch of BDD
  int vars;             // index into vars array
  int index;            // index into BDD DAG
  int Not;              // index into BDD DAG for NOT of current BDD
  struct sm_node *smurf;// pointer to state
  struct BinaryDD *next;// next BDD in chain
};

struct sm_node {
  int v;                // variable v
  struct sm_node *T;    // pointer to state under assumption v is true
  struct sm_node *E;    // pointer to state under assumption v is false
  int index;            // SM index back into SM DAG
  int pure;             // pure literal inference
  float score;          // score of variable
  float score_t;           // score given the variable is true
  float score_e;           // score given the variable is false
  float score_level;       // score adjusted for level
  int level;               // number of inferences made to get to this point
  int inference;        // -v, v, or 0 for no inference
  struct sm_node *next; // next node in current state, NULL if last in state
};

static struct sm_node *ZERO, *ONE, *SM = NULL;

int   next_user_var;
float *user_score_array;
float *user_score_t;
float *user_score_e;
int   *user_var_to_village_index;

int next_var_to_village, *var_to_village_list;

int next_village;
struct sm_node **village;

int next_array_of_villages;

//struct sm_node *array_of_villages[][max_smurfs];

sm_node ***makeSM_Array(int x, int y) {
   sm_node ***tmp = new sm_node**[x];
   for (int i=0 ; i < x ; i++) tmp[i] = new sm_node*[y];
   return tmp;
}
sm_node ***array_of_villages;

// static struct sm_node **village_current;

int next_assumption, *assumption_array;
int *top_assumption;
int node_depth;

int next_max_stack, *max_stack;

int next_inference, *inference_queue;

int next_inf_history, *inf_history_var;
int *inf_history_assume;
int *inf_history_flag;

int next_sm, *next_sm_memo;

//static struct BinaryDD BDD[max_bdd];
BinaryDD *BDD;

int next_var, *vars_array;

int next_eqn, *eqn;

int next_i, next_j;

int next_index, *vars_scratch;

int next_var_list=2;

//char variable_list[][16];

char **makeCHAR_Array(int x, int y) {
   char **tmp = new char*[x];
   for (int i=0 ; i < x ; i++) tmp[i] = new char[y];
   return tmp;
}

char **variable_list;

//

// statistical variables
float total_time=0;
float brancher_time=0;
//clock_t s=0,f=0;
//double s=0,f=0;
double s=0;
int total_pops,total_pushes,total_inferences,max_assumptions;
int total_assumptions;
int total_bdd_used=0;
int num_vars=0;
int trim_count;

//The function prototypes:
void is_valid_state();
void ite_wvf_initialize();
void sm_initialize();
int top_variable(int F,int G,int H);
int reduceT(int X,int V);
int reduceE(int Y,int V);
int find_or_add_node_wvf(int F,int G,int H);
int ite_wvf_var(int variable);
int ite_wvf(int A,int B,int C);
int ite_wvf_XOR(int v1,int v2) ;
int ite_wvf_XOR3(int v1,int v2,int v3) ;
int ite_wvf_XOR4(int v1,int v2,int v3,int v4) ;
int ite_wvf_XOR5(int v1,int v2,int v3,int v4,int v5) ;
int ite_wvf_XOR6(int v1,int v2,int v3,int v4,int v5,int v6) ;
int ite_wvf_XOR7(int v1,int v2,int v3,int v4,int v5,int v6,int v7) ;
int ite_wvf_XOR8(int v1,int v2,int v3,int v4,int v5,int v6,int v7,int v8) ;
int ite_wvf_XOR9(int v1,int v2,int v3,int v4,int v5,int v6,int v7,int v8,int v9) ;
int ite_wvf_XOR10(int v1,int v2,int v3,int v4,int v5,int v6,int v7,int v8,int v9,int v10) ;
int ite_wvf_EQU(int X,int Y);
int ite_wvf_majv(int a,int b,int c);
int ite_wvf_BBBM(int a,int b, int c,int d);
int exist_quantify(int e,int cube);
int mk_vars(int x,int y,int z);
int mk_vassign(int x,int y,int z);
int mk_linear(int x,int y,int z);
int state(int f);
int state1(int f,int level);
void score(sm_node *smj, int infs, int level);
int reduce(int f,int V,int val);
void village_push();
void village_pop();
int enqueue_inference(int x, int flag);
int process_inferences();
void brancher();
void print_statistics();
int gcf(int f,int c);
int add_k_state(int n,int k);
int add_k_state1(int n,int k);
void solution_found();
void wvf_solve();
   
/***************************/

#define ite_wvf_not(Z) (Z ^ 0x1)
#define ite_wvf_NOT(Z) (Z ^ 0x1)

/*********************************/

void ite_wvf_initialize() {
	
	user_score_array = new float[max_vars];
	user_score_t = new float[max_vars];
	user_score_e = new float[max_vars];
	eqn = new int[max_bdd];
	user_var_to_village_index = new int[max_vars];
	var_to_village_list = new int[max_vars_list_space];
	village = new sm_node*[max_smurfs];
   array_of_villages = makeSM_Array(max_vars, max_smurfs); 
//	*array_of_villages = new sm_node*[max_vars];
	assumption_array = new int[max_vars];
	top_assumption = new int[max_vars];
	max_stack = new int[max_vars];
	inference_queue = new int[max_vars];
	inf_history_var = new int[max_vars];
	inf_history_assume = new int[max_vars];
	inf_history_flag = new int[max_vars];
	next_sm_memo = new int[max_sm];
	vars_array = new int[max_vars_list_space];
	vars_scratch = new int[max_vars];
	variable_list = makeCHAR_Array(max_vars, 16);
	
	//BDD = (BinaryDD *)calloc(max_bdd, sizeof(BinaryDD));
	BDD = new BinaryDD[max_bdd];
	
  // set flag to indicate bdd is free	
  for (int i = 0; i < max_bdd; i++) {
   	BDD[i].v = 0;
    	BDD[i].next = (BDD+(i+1));
  		BDD[i].index = i;
  }
  BDD[max_bdd_mask].next = NULL;
  BDD[max_bdd_mask-1].next = NULL;

  //reserving variable 0 and 1

  // defining false BDD
  BDD[0].v = ZERO_ONE_VAR;
  BDD[0].T = 0;
  BDD[0].E = 0;
  BDD[0].vars = 0;
  BDD[0].Not = 1;

  // defining true BDD
  BDD[1].v = ZERO_ONE_VAR;
  BDD[1].T = 1;
  BDD[1].E = 1;
  BDD[1].vars = 0;
  BDD[1].Not = 0;

  if (SM != NULL) {
    BDD[0].smurf = (SM+0);
    BDD[1].smurf = (SM+1);
  }

  total_bdd_used = 2;

  eqn[0] = 0;
  eqn[1] = 1;
  next_eqn = 2;

  vars_array[0] = 0;
  next_var = 1;
  next_i = 0;
  next_j = 0;
  num_vars = 0;
}

/**************************************/

void sm_initialize() {
    int i;

    // allocate state machine memory
    if (SM == NULL)
	    SM = (struct sm_node *)malloc(max_sm * sizeof(struct sm_node));
    if (SM == NULL) {
	    printf("\nCould not allocate space for SM storage.\n");
	    exit(1);
    }
    printf("Allocated for [SM] %15ld bytes.\n", (long)(max_sm * sizeof(struct sm_node)));

    for (i=0; i<max_sm; i++) {
	    SM[i].index = i;
	    SM[i].next = (SM+(i+1));
    }
    SM[max_sm].next = NULL;
	
    // create state machine constant zero and one
    ZERO = (SM+0);
    ONE = (SM+1);
    ZERO->v = 0;
    ZERO->T = ZERO;
    ZERO->E = ZERO;
    ZERO->pure = 0;
    ZERO->score = 0;
    ZERO->score_t = 0;
    ZERO->score_e = 0;
    ZERO->inference = 0;
    ZERO->next = NULL;
    ONE->v = 1;
    ONE->T = ONE;
    ONE->E = ONE;
    ONE->pure = 0;
    ONE->score = 0;
    ONE->score_t = 0;
    ONE->score_e = 0;
    ONE->inference = 0;
    ONE->next = NULL;

    if (BDD != NULL) {
      BDD[0].smurf = ZERO;
      BDD[1].smurf = ONE;
    }

    next_sm = 2;

    village[0] = ZERO;
    next_village = 1;
    var_to_village_list[0] = 0;
    next_var_to_village = 1;
    next_user_var = 0;
    next_inference = 0;
    next_array_of_villages = 0;
    next_assumption = 0;
    next_inf_history = 0;
    next_sm_memo[0] = 0;
    next_sm_memo[1] = 1;

    // initialize add_state memo table
    for (i = 2; i < max_sm; i++) next_sm_memo[i] = -1;
    for (i=0; i<next_user_var; i++) top_assumption[i] = 0;
	 total_assumptions=0;
    total_pops = 0;
    total_pushes = 0;
    total_inferences=0;
    max_assumptions=0;
    node_depth = 0;
    trim_count = 0;
}

/*************************************/

int top_variable(int F,int G,int H) {
   if ((BDD[F].v <= BDD[G].v) && (BDD[F].v <= BDD[H].v))
      return (BDD[F].v);
   if (BDD[G].v <= BDD[H].v)
      return (BDD[G].v);
   return (BDD[H].v);
}

/********************************/

int reduceT(int X,int V) {
   // X is the BDD number and V is the top variable

   if (BDD[X].v == V)
      return (BDD[X].T);
   return X;
}

/***********************************/

int reduceE(int Y,int V) {
   // Y is the BDD number and V is the top variable

   if (BDD[Y].v == V)
      return (BDD[Y].E);
   return Y;
}

/***********************************/

//F is a variable, G and H are BDD numbers
int find_or_add_node_wvf(int F, int G, int H) {
   struct BinaryDD *b,*s,*bn;
  
   // search BDD DAG to see if BDD already exists
   s = (BDD+(((F+G+H) << 6) & max_bdd_mask));
   for (b=s; b->v != 0; b = b->next)
      if ((b->v == F) && (b->T == G) && (b->E == H)) return(b->index);
   if (b->next == NULL) {
      for (b=(BDD+2); (b != s) && (b->v != 0); b=b->next)
         if ((b->v == F) && (b->T == G) && (b->E == H))
            return(b->index);
         if (b == s) {
            printf("Error: Exceeded allocated BDD space.\n");
            exit(1);
         }
	}
  
   if ((G == 1) && (H == 0)) {
      num_vars++;
      if (num_vars > max_vars) {
         printf("Error: number of configured variables exceeded.\n");
         exit(1);
      }
	}
  
   bn = b->next;
  
   // define primary BDD
   b->v = F;
   b->T = G;
   b->E = H;
   b->vars = mk_vars(F,G,H);
   b->Not = bn->index;
   b->smurf = NULL;
   total_bdd_used++;
  
   // define complement of primary BDD
   bn->v = F;
   bn->T = ite_wvf_not(G);
   bn->E = ite_wvf_not(H);
   bn->vars = b->vars;
   bn->Not = b->index;
   bn->smurf = NULL;
   total_bdd_used++;
  
   return(b->index);
}

/*******************************/

int ite_wvf_var(int variable) {
   int absvar = abs(variable);
  
   if ((absvar > max_vars) || (absvar >= ZERO_ONE_VAR)) {
      printf("Error: %d is too large of a variable.\n",variable);
      exit (1);
   }
  
	if (absvar < 2) {
      printf("Error: %d is an illegal variable identifer.\n",variable);
      printf("Note: 0 is reserved for false, 1 is reserved for true.\n");
      exit (1);
   }
  
	if (variable < 0)
      return (ite_wvf_not(find_or_add_node_wvf(absvar,1,0)));
   return (find_or_add_node_wvf(variable, 1, 0));
}

/******************************/

// A, B, and C are indexs into the BDD DAG
int ite_wvf(int A, int B,int C) {
   //main funtion in order to find the ite
  
   int Var, T_branch, F_branch;

   //Check base cases
   if (A < 2) {
      if (A == 1) return B;
      return C;
   }
  
	if (A == B) B = 1;
   else if (A == ite_wvf_not(B)) B = 0;
   if (A == C) C = 0;
   else if (A == ite_wvf_not(C)) C = 1;
   if (B == C) return B;
   if ((B < 2) && (C < 2)) return (A ^ C);

   Var = top_variable(A,B,C);
   T_branch = ite_wvf(reduceT(A,Var),reduceT(B,Var),reduceT(C,Var));
   F_branch = ite_wvf(reduceE(A,Var),reduceE(B,Var),reduceE(C,Var));
   if (T_branch == F_branch) return (T_branch);
   return (find_or_add_node_wvf(Var,T_branch,F_branch));
}

/***************************/

#define ite_wvf_AND(X,Y) ite_wvf(X,Y,0)

/**************************/

#define ite_wvf_AND3(X,Y,Z) ite_wvf_AND(X, ite_wvf_AND(Y,Z))

/*************************/

#define ite_wvf_AND4(A,B,C,D) ite_wvf_AND(A, ite_wvf_AND(B, ite_wvf_AND(C,D)))

/*************************/

#define ite_wvf_AND5(A,B,C,D,E) ite_wvf_AND(A, ite_wvf_AND(B, ite_wvf_AND(C, ite_wvf_AND(D,E))))

/*************************/

#define ite_wvf_AND6(A,B,C,D,E,F) ite_wvf_AND(A, ite_wvf_AND(B, ite_wvf_AND(C, ite_wvf_AND(D, ite_wvf_AND(E,F)))))

/*************************/

#define ite_wvf_AND7(A,B,C,D,E,F,G) ite_wvf_AND(A, ite_wvf_AND(B, ite_wvf_AND(C, ite_wvf_AND(D, ite_wvf_AND(E, ite_wvf_AND(F,G))))))

/************************/

#define ite_wvf_AND8(A,B,C,D,E,F,G,H) ite_wvf_AND(A, ite_wvf_AND(B, ite_wvf_AND(C, ite_wvf_AND(D, ite_wvf_AND(E, ite_wvf_AND(F, ite_wvf_AND(G,H)))))))

/************************/

#define ite_wvf_AND9(A,B,C,D,E,F,G,H,I)  ite_wvf_AND(A, ite_wvf_AND(B, ite_wvf_AND(C, ite_wvf_AND(D, ite_wvf_AND(E, ite_wvf_AND(F, ite_wvf_AND(G, ite_wvf_AND(H,I))))))))

/****************************/

#define ite_wvf_AND10(A,B,C,D,E,F,G,H,I,J)  ite_wvf_AND(A, ite_wvf_AND(B, ite_wvf_AND(C, ite_wvf_AND(D, ite_wvf_AND(E, ite_wvf_AND(F, ite_wvf_AND(G, ite_wvf_AND(H, ite_wvf_AND(I,J)))))))))

/****************************/

#define ite_wvf_OR(X,Y) ite_wvf(X,1,Y)

/***************************/

#define ite_wvf_OR3(X,Y,Z) ite_wvf_OR(X, ite_wvf_OR(Y,Z))

/***************************/

#define ite_wvf_OR4(A,B,C,D) ite_wvf_OR(A, ite_wvf_OR(B, ite_wvf_OR(C,D)))
 
/***************************/

#define ite_wvf_OR5(A,B,C,D,E) ite_wvf_OR(A, ite_wvf_OR(B, ite_wvf_OR(C, ite_wvf_OR(D,E))))

/***************************/

#define ite_wvf_OR6(A,B,C,D,E,F) ite_wvf_OR(A, ite_wvf_OR(B, ite_wvf_OR(C, ite_wvf_OR(D, ite_wvf_OR(E,F)))))

/***************************/

#define ite_wvf_OR7(A,B,C,D,E,F,G) ite_wvf_OR(A, ite_wvf_OR(B, ite_wvf_OR(C, ite_wvf_OR(D, ite_wvf_OR(E, ite_wvf_OR(F,G))))))

/***************************/

#define ite_wvf_OR8(A,B,C,D,E,F,G,H) ite_wvf_OR(A, ite_wvf_OR(B, ite_wvf_OR(C, ite_wvf_OR(D, ite_wvf_OR(E, ite_wvf_OR(F, ite_wvf_OR(G,H)))))))

/***************************/

#define ite_wvf_OR9(A,B,C,D,E,F,G,H,I) ite_wvf_OR(A, ite_wvf_OR(B, ite_wvf_OR(C, ite_wvf_OR(D, ite_wvf_OR(E, ite_wvf_OR(F, ite_wvf_OR(G, ite_wvf_OR(H,I))))))))

/**************************/

#define ite_wvf_OR10(A,B,C,D,E,F,G,H,I,J) ite_wvf_OR(A, ite_wvf_OR(B, ite_wvf_OR(C, ite_wvf_OR(D, ite_wvf_OR(E, ite_wvf_OR(F, ite_wvf_OR(G, ite_wvf_OR(H, ite_wvf_OR(I,J)))))))))

/***************************/

// since Y is used twice, this is a function, not a #define

int ite_wvf_XOR(int v1,int v2) 
{ return(ite_wvf(v1,ite_wvf_NOT(v2),v2)); }
int ite_wvf_XOR3(int v1,int v2,int v3) 
{ return(ite_wvf_XOR(v1,ite_wvf_XOR(v2,v3))); }
int ite_wvf_XOR4(int v1,int v2,int v3,int v4) 
{ return(ite_wvf_XOR(v1,ite_wvf_XOR(v2,ite_wvf_XOR(v3,v4)))); }
int ite_wvf_XOR5(int v1,int v2,int v3,int v4,int v5) 
{ return(ite_wvf_XOR(v1,ite_wvf_XOR(v2,ite_wvf_XOR(v3,ite_wvf_XOR(v4,v5))))); }
int ite_wvf_XOR6(int v1,int v2,int v3,int v4,int v5,int v6)
{ return ite_wvf_XOR(v1,ite_wvf_XOR(v2,ite_wvf_XOR(v3,ite_wvf_XOR(v4,ite_wvf_XOR(v5,v6))))); }
int ite_wvf_XOR7(int v1,int v2,int v3,int v4,int v5,int v6,int v7)
{ return ite_wvf_XOR(v1,ite_wvf_XOR(v2,ite_wvf_XOR(v3,ite_wvf_XOR(v4,ite_wvf_XOR(v5,
                ite_wvf_XOR(v6,v7)))))); }
int ite_wvf_XOR8(int v1,int v2,int v3,int v4,int v5,int v6,int v7,int v8)
{ return ite_wvf_XOR(v1,ite_wvf_XOR(v2,ite_wvf_XOR(v3,ite_wvf_XOR(v4,ite_wvf_XOR(v5,ite_wvf_XOR(v6,
	       	ite_wvf_XOR(v7,v8))))))); }
int ite_wvf_XOR9(int v1,int v2,int v3,int v4,int v5,int v6,int v7,int v8, int v9)
{ return ite_wvf_XOR(v1,ite_wvf_XOR(v2,ite_wvf_XOR(v3,ite_wvf_XOR(v4,ite_wvf_XOR(v5,ite_wvf_XOR(v6,
		ite_wvf_XOR(v7,ite_wvf_XOR(v8,v9)))))))); }
int ite_wvf_XOR10(int v1,int v2,int v3,int v4,int v5,int v6,int v7,int v8,
	      int v9,int v10)
{ return ite_wvf_XOR(v1,ite_wvf_XOR(v2,ite_wvf_XOR(v3,ite_wvf_XOR(v4,ite_wvf_XOR(v5,ite_wvf_XOR(v6,
		ite_wvf_XOR(v7,ite_wvf_XOR(v8,ite_wvf_XOR(v9,v10))))))))); }

/*****************************/

#define ite_wvf_IMP(X,Y) ite_wvf(X,Y,1)

/*****************************/

#define ite_wvf_NAND(X,Y) ite_wvf(X,ite_wvf_NOT(Y),1)

/****************************/

#define ite_wvf_NOR(X,Y) ite_wvf(X,0,ite_wvf_NOT(Y))

/***************************/

//since Y is used twice, this is a function, not a #define

int ite_wvf_EQU(int X,int Y) {
  return(ite_wvf(X,Y,ite_wvf_NOT(Y)));
}

/**************************/

#define ite_wvf_NIMP(X,Y) ite_wvf(X,ite_wvf_NOT(Y),0)

/*************************/

//since b and c are used twice, this is a function, not a #define

int ite_wvf_majv(int a,int b,int c) {
   return (ite_wvf(a,ite_wvf_OR(b,c),ite_wvf_AND(b,c)));
}

/**************************/

// since b and c are used twice, this is a function, not a #define

int ite_wvf_BBBM(int a,int b,int c,int d) {
   return (ite_wvf_AND3(ite_wvf_OR(a,b),ite_wvf_OR(b,c),ite_wvf_OR(c,d)));
}

/*************************/

int quantify(int e,int v) {
   int a,b;
   if ((e < 2) || (v < BDD[e].v)) return (e);
   a = quantify(BDD[e].T,v);
   b = quantify(BDD[e].E,v);
   if (a == b) return (a);
   if (BDD[e].v == v) return (ite_wvf_OR(a,b));
   return (find_or_add_node_wvf(BDD[e].v,a,b));
}

/*************************/
  
// cube is the ite_wvf_and of ite_wvf_var of psoitive variable ids
int exist_quantify(int e,int cube) {
   int a,b;
   if (e < 2) return (e);
   while (BDD[e].v > BDD[cube].v) cube = BDD[cube].T;
   if (cube < 2) return (e);
   a = exist_quantify(BDD[e].T,cube);
   b = exist_quantify(BDD[e].E,cube);
   if (a == b) return (a);
   if (BDD[e].v == BDD[cube].v) return (ite_wvf_OR(a,b));
   return (find_or_add_node_wvf(BDD[e].v,a,b));
}

/*************************/
  
#define print_bdd2(BDD_num) print_bdd1(BDD_num,0)

void print_it()
{
	printf("The BDD DAG\n");
	printf("  Index	v	T	E	vars	not	smurf\n");
	printf(" -----------------------------------\n");
	for(int g = 0; g < max_bdd; g++)
	{
		if(BDD[g].v!=0)
		{ 
			printf("%7d: ", g);
			if((BDD[g].v < 2) ||
				(BDD[g].v >= next_var_list) ||
				(BDD[g].v == ZERO_ONE_VAR))
				printf("%5d ", BDD[g].v);
//			else
//				printf("%s(%d)", int_to_sym(BDD[g].v), BDD[g].v);
			if(BDD[g].smurf == NULL)
 				printf("%5d %5d %5d %5d		-\n", 
					BDD[g].T, BDD[g].E, BDD[g].vars, BDD[g].Not);
			else
 				printf("%5d %5d %5d %5d	%5d	-\n", 
					BDD[g].T, BDD[g].E, BDD[g].vars, BDD[g].Not, (BDD[g].smurf)->index);
		}
	}
}

/*************************/

#define int_to_sym(num) variable_list[abs(num)]

/*************************************/

void print_bdd1(int BDD_num,int print_counter) {
   int i;

   for(i=0; i<print_counter; i++) printf("    ");
   if (BDD_num < 2) {
      printf ("%d\n",BDD_num);
      return;
   }
   if ((BDD[BDD_num].T > 1) || (BDD[BDD_num].E > 1)) {
      if ((BDD[BDD_num].v < 2) || (BDD[BDD_num].v >= next_var_list))
	      printf ("ite_wvf %d\n",BDD[BDD_num].v);
//      else
//	      printf("ite_wvf %s(%d)\n",int_to_sym(BDD[BDD_num].v),BDD[BDD_num].v);
      print_bdd1(BDD[BDD_num].T,print_counter+1);
      print_bdd1(BDD[BDD_num].E,print_counter+1);
      return;
   }
  
	if ((BDD[BDD_num].v < 2) || (BDD[BDD_num].v >= next_var_list)) {
      printf ("ite_wvf %d %d %d\n",BDD[BDD_num].v,BDD[BDD_num].T,BDD[BDD_num].E);
      return;
   }
  
//	printf ("ite_wvf %s(%d) %d %d\n",int_to_sym(BDD[BDD_num].v),BDD[BDD_num].v,
//	                             BDD[BDD_num].T,BDD[BDD_num].E);
   return;
}

/******************************/

int mk_vars(int x,int y, int z) {
   int a,b,i,j,k,m,start;
  
   next_index=0;
   // merge sort two variable lists
   vars_scratch[0] = x;
   i = 1;
   j = BDD[y].vars;
   k = BDD[z].vars;
   a = vars_array[j++];
   b = vars_array[k++];
   while ((a > 0) && (b > 0)) {
      if (a < b) {
	      vars_scratch[i++] = a;
	      a = vars_array[j++];
	   } else if (a == b) {
	      vars_scratch[i++] = a;
	      a = vars_array[j++];
	      b = vars_array[k++];
	   } else {
	      vars_scratch[i++] = b;
	      b = vars_array[k++];
	   }
   }
  
	while (a > 0) {
      vars_scratch[i++] = a;
      a = vars_array[j++];
   }
  
	while (b > 0) {
      vars_scratch[i++] = b;
      b = vars_array[k++];
   }
  
	vars_scratch[i++] = 0;
   next_index = i;
   // searching to see if this vars list is already in vars_array
   i = 1;
   start = i;
   while (start != next_var) {
      for(k = 0; 
			 (k < next_index) && (vars_scratch[k] == vars_array[i]);
	       k++, i++);
      if (k == next_index)
	      return(start);
      // find next zero in vars_array
      for(m = i; vars_array[m] != 0; m++);
      start = (m + 1);
      i = start;
   }

	if ((next_index + next_var) > max_vars_list_space) {
      printf("Ran out of BDD var list space.\n");
      exit(1);
   }

   for(k = 0; k < next_index; vars_array[i] = vars_scratch[k], i++, k++);
   next_var = i;
   return(start);
}

/*********************************/

int state (int f) {
	if (next_village >= max_smurfs) {
	   fprintf(stderr, "Exceeded allocated space for smurfs.\n");
	   exit(1);
	}
   if (BDD[f].smurf != NULL) {
	   village[next_village] = BDD[f].smurf;
	   next_village++;
	   return 0;
   }
   village[next_village] = (SM+next_sm);
   next_village++;
   return state1(f,0);
}

/*******************************/

int state1 (int f, int level) {
   int t,e,i,j = 0,start,count,cf_and,sm_num_infs,f_save=f;
   struct sm_node *smj;
 
  /* 
   clock_t timer;
	timer = clock();
   */

   if (BDD[f].smurf != NULL)  // check to see if smurf already exists
      return 0;
  
   start = next_sm;
   for (count = 0, i = BDD[f].vars; vars_array[i] != 0; count++, i++);
   next_sm += count; // reserve sm space before recursive calls are made
  
   if (next_sm >= max_sm) {
      fprintf(stderr, "Exceeded max_sm for state machine.\n");
      exit(1);
   }

   // copy vars from BDD to SM
   cf_and = 1; // bdd true
   sm_num_infs = 0;
   for (i = BDD[f].vars, smj = (SM+start); vars_array[i] != 0;
         i++, smj = smj->next) {
      smj->v = vars_array[i];
      t = reduce(f,smj->v,1);
      e = reduce(f,smj->v,0);
      smj->T = (struct sm_node *)t;
      smj->E = (struct sm_node *)e;
      if (t == 0) {
         smj->inference = - smj->v;
         cf_and = ite_wvf_AND(cf_and, ite_wvf_var(- smj->v));
         sm_num_infs++;
      } else if (e == 0) {
         smj->inference = smj->v;
         cf_and = ite_wvf_AND(cf_and, ite_wvf_var(smj->v));
         sm_num_infs++;
      } else
         smj->inference = 0;      
	}

	// cf_and is the ite_wvf_and of all the inferences in this state
   // sm_num_infs is the counts of all inferences in this state

   // sm_num_infs *= 100000; // adjust for integer arithmetic

   for (smj = (SM+start); count>0; smj = smj->next) {
      t = gcf(*(int*)&(smj->T), cf_and); // apply inferences now
      e = gcf(*(int*)&(smj->E), cf_and); // apply inferences now

      if (t == (e ^ 1)) // linear variable experiment
	      smj->pure = 1;
      else
	      smj->pure = 0;

      state1(t, level+1);
      state1(e, level+1);
      smj->T = BDD[t].smurf;
      smj->E = BDD[e].smurf;
      count--;
      score(smj, sm_num_infs, 1 << level);
      // score(smj, sm_num_infs, 1);


      if (smj->score == 0) {
	      fprintf(stderr, "State %d has a score of zero.\n", j);
	      exit (1);
	   }
		
		
      if (smj->T == ZERO)
	      smj->inference = - smj->v;
      else if (smj->E == ZERO)
	      smj->inference = smj->v;
      else
	      smj->inference = 0;
      if (count == 0)
	      smj->next = NULL;

	/*	
		if(((clock() - timer)/(float)CLOCKS_PER_SEC) > STATE_TIME){
			fprintf(stdout, "|%f|\n",((clock() - timer) / (float)CLOCKS_PER_SEC));
//			print_bdd2(f); //BDD[f].vars			
			return 1;
		}
      */
   }
   BDD[f_save].smurf = (SM+start);
	return 0;
}

/**************************/

void score(sm_node *smj, int infs, int level) {
   int i_t,i_e,cnt;
   float total_e=0,total_t=0,quotient;
   struct sm_node *w;
	
   if (smj->T == ZERO) {
	   w = smj->E;
	   for(i_e=0; w != NULL; i_e++, w = w->next) total_e += w->score;
	   quotient = total_e / i_e;
      smj->score_t = 0; // score not needed when we have an inference
      smj->score_e = 0; // score not needed when we have an inference
	   smj->score = quotient + infs;
	   smj->score_level = smj->score * level;
	   smj->level = level;
	   return;
	}
   if (smj->E == ZERO) {
	   w = smj->T;
	   for(i_t=0; w != NULL; i_t++, w = w->next) total_t += w->score;
	   quotient = total_t / i_t;
	   smj->score_t = 0; // score not needed when we have an inference
	   smj->score_e = 0; // score not needed when we have an inference
	   smj->score = quotient + infs;
	   smj->score_level = smj->score * level;
	   smj->level = level;
	   return;
	}
   if (smj->T == ONE) {
	   w = smj->E;
	   cnt=0;
	   for(i_e=0; w != NULL; i_e++, w = w->next) {
		   total_e += w->score;
		   if (w->inference != 0) cnt++;
	   }
	   quotient = (((total_e / i_e) - cnt) / score_const_1) + cnt;
	   smj->score_t = 0;
	   smj->score_e = quotient;
	   quotient = quotient / 2;
	   smj->score = quotient + infs;
	   smj->score_level = smj->score * level;
	   smj->level = level;
	   return;
	}
   if (smj->E == ONE) {
	   w = smj->T;
	   cnt = 0;
	   for(i_t=0; w != NULL; i_t++, w = w->next) {
		   total_t+= w->score;
		   if (w->inference != 0) cnt++;
	   }
	   quotient = (((total_t / i_t) - cnt) / score_const_1) + cnt;
	   smj->score_t = quotient;
	   smj->score_e = 0;
	   quotient = quotient / 2;
	   smj->score = quotient + infs;
	   smj->score_level = smj->score * level;
	   smj->level = level;
	   return;
	}
   w = smj->T;
   cnt = 0;
   for(i_t=0; w != NULL; i_t++, w = w->next) {
	   total_t += w->score;
	   if (w->inference != 0) cnt++;
   }
   smj->score_t = (((total_t / i_t) - cnt) / score_const_2) + cnt;
   w = smj->E;
   cnt = 0;
   for(i_e=0; w != NULL; i_e++, w = w->next) {
	   total_e += w->score;
	   if (w->inference != 0) cnt++;
   }
   smj->score_e = (((total_e / i_e) - cnt) / score_const_1) + cnt;
   quotient = (smj->score_t + smj->score_e) / 2;
   smj->score = quotient + infs;
   smj->score_level = smj->score * level;
   smj->level = level;
   return;
}

/********************************/

int reduce (int f, int V, int val) {
   int T,E;

   if (V < BDD[f].v)	return f;
   if (BDD[f].v == V) {
	   if (val == 1) return (BDD[f].T);
	   return (BDD[f].E);
	}
   T = reduce(BDD[f].T,V,val);
   E = reduce(BDD[f].E,V,val);
   if (T == E)	return T;
   return (find_or_add_node_wvf(BDD[f].v,T,E));
}
  
/***************************/

void wvf_solve () {
   int j = 0, k, start, big_user_var=0;
   struct sm_node *smi;
  
   // to find next_user_var
   for (k = 1; k < next_village; k++) {
      for (smi = village[k]; smi != NULL; smi = smi->next) {
	      if (smi->v > big_user_var)
	         big_user_var = smi->v;
	   }
   }
   next_user_var =(big_user_var + 1);
   if (next_user_var > max_vars) {
      printf("User vars exceeded max allowed variables.\n");
      exit(1);
   }
  
   //to find all user_vars
   for(j = 2; j < next_user_var; j++) {
      start = next_var_to_village;
      //to step through village
      for(k = 1; k < next_village; k++) {
	      //to step through SM variables
	      for (smi = village[k]; smi != NULL; smi = smi->next) {
	         if (j == smi->v) {
		         var_to_village_list[next_var_to_village]=k;
		         next_var_to_village++;
		      }
	      }
	   }
      if (start != next_var_to_village) {
	      var_to_village_list[next_var_to_village] = 0;
	      next_var_to_village++;
	      user_var_to_village_index[j] = start;
	   } else
	      user_var_to_village_index[j] = 0;
   }

   if (next_var_to_village > max_vars_list_space) {
      printf("var to village list space exceeded.\n");
      exit(1);
   }
   brancher();
   return;
}

/******************************/

void village_push() {
   total_pushes++;
   //village_current = array_of_villages[next_array_of_villages];
   for(int i = 1; i < next_village; i++)
		array_of_villages[next_array_of_villages][i] = village[i];
   //village_current[i] = village[i];
   next_array_of_villages++;
}

/*****************************/

void village_pop() {
   // total_assumptions += next_array_of_villages;
   total_assumptions += node_depth;
   total_pops++;
   //empty inference_queue
   next_inference = 0;
   next_array_of_villages--;
   //village_current = array_of_villages[next_array_of_villages];
   for(int i = 1; i < next_village; i++)
		village[i] = array_of_villages[next_array_of_villages][i];
   //village[i] = village_current[i];
}

/***************************/

int enqueue_inference(int x, int flag) {
   int neg_x = - x;
   for(int i=0; i < next_inference; i++) {
	   if (inference_queue[i] == x) // check for duplicate
		   return (1);
	   if (inference_queue[i] == neg_x) // check for contradiction
		   return (0);
	}

   total_inferences++;
   inference_queue[next_inference] = x;
   next_inference++;

   inf_history_var[next_inf_history] = x;
   inf_history_assume[next_inf_history] = next_assumption;
   inf_history_flag[next_inf_history] = flag;
   next_inf_history++;
   if (flag == 1) node_depth++;
   return (1);
}
/*****************************/

int process_inferences() {
   int inf,abs_inf,var_vil_index,vil,n;
   struct sm_node *state_var;
  
   while (next_inference > 0) {
      next_inference--;
      inf = inference_queue[next_inference];
      abs_inf = abs(inf);

      var_vil_index = user_var_to_village_index[abs_inf];

      while ((vil = var_to_village_list[var_vil_index]) != 0) {
	      var_vil_index++;
	      if ((state_var = village[vil]) == ONE) continue;
	      while ((state_var != NULL) && (state_var->v != abs_inf))
	         state_var = state_var->next;
	      if (state_var == NULL) continue;
	      state_var = (inf>0)?state_var->T:state_var->E;
	      // perform state transformation
	      village[vil] = state_var;
	      // it should never be case that state_var is ZERO
	      // if (state_var == ZERO) {printf("!"); return (0);}
	      for (; state_var != NULL; state_var = state_var->next)
	         if ((n = state_var->inference) != 0)
	            if (enqueue_inference(n,0) == 0)
		            return (0);
	   }
	}
   return (1);
}

/*********/
int
heuristic()
{
   int i,j,var_j,max_var,R=1;
   struct sm_node *state_var;
   // long long score_i, max_score;
   float score_i, max_score;

      // find next variable to assume
      for (i = 0; i < next_user_var; i++) {
	      user_score_array[i] = 0;
	
#ifdef VALUE_FLAG
	      user_score_t[i] = 0;
	      user_score_e[i] = 0;
#endif
	
      }
      
      //compute the score for all variables
      for (i = 1; i < next_village; i++)
	      for (state_var = village[i]; state_var != NULL; state_var = state_var->next) {
	         var_j = state_var->v;
	         user_score_array[var_j] += state_var->score_level; // look here
	    
#ifdef VALUE_FLAG
	         user_score_t[var_j] += state_var->score_t;
	         user_score_e[var_j] += state_var->score_e;
#endif

			}

      next_max_stack = 0;
      max_score = 0;
      for (i = 2; i < next_user_var; i++) {
#ifdef VALUE_FLAG
	      score_i = (user_score_t[i] + 1) * ( user_score_e[i] + 1);
#else
	      score_i = user_score_array[i];
#endif	  
	      if (score_i < max_score) continue;
	      if (score_i == max_score) {
	         max_stack[next_max_stack] = i;
	         next_max_stack++;
	         continue;
	      }
	      // new max reset stack
	      max_score = score_i;
	      max_stack[0] = i;
	      next_max_stack = 1;
		}

#ifdef VALUE_FLAG
      if (max_score == 1)
#else
      if (max_score == 0)
#endif
      {
	      R=0;
	      for(i = 1; i < next_village; i++)
	         if (village[i] != ONE) {
	            R=1;
	            break;
	         }
	         if (R == 0) {
	            solution_found(); // look here
	            return -1;
	         }
	         max_stack[0] = next_user_var >> 1; // look here, special heuristic
	         next_max_stack = 1;
	         max_var = max_stack[0];
	         if (node_depth != 1) {
	            printf("Error: max score 0, node depth not 0\n");
	            print_statistics();
	            exit(1);
	         }
		}
      if (top_assumption[node_depth] == 0) {
	      // we have identified the next variable to assume
	      // if midmost - pick middle-most variable, else pick middle position
	      if (mid_most == 1) {
	         j = next_user_var >> 1;
	         for (i=1; i<next_max_stack; i++)
	            if (abs(max_stack[i] - j) < abs(max_stack[0] - j))
	               max_stack[0] = max_stack[i];
	         max_var = max_stack[0];
			} else
	         max_var = max_stack[next_max_stack >> 1];
      } else {
	      max_var = top_assumption[node_depth];
	      // printf("(%d,%d,%d) ",next_assumption,node_depth,max_var);
	      // fflush(stdout);
      }

#ifdef VALUE_FLAG
      if (user_score_t[max_var] < user_score_e[max_var])
         max_var = - max_var;
#endif
      return max_var;
}
     
/*****************************/

void brancher() {
   int i,max_var,R = 1;
   struct sm_node *state_var;
  
	fprintf(stderr, "Branching..........\n");	
   /*
   f = clock();
   //f = get_runtime();
   total_time += (float)(f-s);
   s = f;
   */
  
   // process all ground-0 (initial) inferences if any exist
   for (i = 1; i < next_village; i++)
      for (state_var = village[i]; state_var != NULL;	state_var = state_var->next) {
	      if (state_var->inference == 0) continue;
	      if (enqueue_inference(state_var->inference, 0) == 0) {
	         // contradiction was detected
	         R = 0;
	         break;
	      }
      }

   if ((R == 0) || ((R = process_inferences()) == 0)) {
      /*
      f = clock();
      brancher_time += (float)(f-s);
      s = f;
      */

      printf("Contradiction was detected from initial inferences.\n");
      printf("System is unsatisfiable: %f %d\n",
	              (brancher_time / (float)CLOCKS_PER_SEC), trim_count);
      print_statistics();
            
      return;
   }

   // top of assumption loop
   while (R == 1) {

      max_var = heuristic();
      if (max_var == -1) return;
      assumption_array[next_assumption] = max_var;
      next_assumption++;
      
      // if (next_assumption > max_assumptions)
      // max_assumptions = next_assumption;
      if (node_depth > max_assumptions)
	      max_assumptions = node_depth;

      enqueue_inference(max_var,1);	
#ifdef TRACE_FLAG
      if (set_trace == 1) {
	      for (i=0; i < (total_pushes - total_pops); i++) printf(" ");
	      printf("<%3d>\n", max_var);
      }
#endif

      // is_valid_state();
      village_push();

      // process next assumption
      while ((((R = process_inferences()) == 0)
	          || (node_depth > next_user_var)) // trim
	          && (next_assumption > 0)) {
	      if (node_depth > next_user_var) // trim
		   trim_count++;

	      village_pop();
	      // is_valid_state();

	      for (i = (next_inf_history - 1); 
	           ((i > 0) && (inf_history_assume[i - 1] == next_assumption));
	           i--)
	         if (inf_history_flag[i] == 1)
	            node_depth--;
	      node_depth--;

	      next_inf_history = i;

	      next_assumption--;

	      enqueue_inference(- assumption_array[next_assumption],1);
#ifdef TRACE_FLAG
	      if (set_trace == 1) {
	         for (i=0; i < (total_pushes - total_pops); i++)
	         printf(" ");
	         printf("{%3d}\n", - assumption_array[next_assumption]);
	      }
#endif
		}
      if ((R == 0) && (next_assumption == 0)) {
         /*
	      f = clock();
	      brancher_time += (float)(f-s);
	      s = f;
         */

	      printf("System is unsatisfiable: %f %d\n",
		           (brancher_time / (float)CLOCKS_PER_SEC), trim_count);
         print_statistics();

	      return;
	   }
   }  //inference queue is empty and R = 1, look for next assumption
}

/*****************************/

void print_statistics() {  
   printf("The Statistics:\n");
   printf("Total variables:     %d\n",next_user_var);
   printf("Total pushes:        %d\n",total_pushes);
   printf("Total pops:          %d\n",total_pops);
   printf("Total inferences:    %d\n",total_inferences);
   printf("Max assumptions:     %d\n",max_assumptions);
   printf("BDDs used:           %d\n",total_bdd_used);
   printf("Total BDD variables: %d\n",num_vars);
   printf("States used:         %d\n",next_sm);
   printf("Trim count:          %d\n",trim_count);
   printf("Average number of assumptions per backtrack: %f\n",
	       ((float)total_assumptions / ((float)total_pops + 1)));
   printf("Total brancher time: %f\n",
	       (brancher_time / (float)CLOCKS_PER_SEC));
   printf("Total pre-processing time: %f\n",
	       ((total_time - brancher_time)/ (float)CLOCKS_PER_SEC));
   printf("Total elapsed time:  %f\n",(total_time / (float)CLOCKS_PER_SEC));
}

/******************************/

//generalized co-factor, f,c, and gcf are BDDs, c != 0
int gcf (int f, int c) {
   int v, c0, c1, a, b;

   if ((c < 2) || (f < 2)) return (f);
   if (f == c) return (1);
   v = (BDD[f].v < BDD[c].v) ? BDD[f].v : BDD[c].v;
   c0 = reduceE(c,v);
   c1 = reduceT(c,v);
   if (c0 == 0) return (gcf(reduceT(f,v),c1));
   if (c1 == 0) return (gcf(reduceE(f,v),c0));
   a = gcf(reduceT(f,v),c1);
   b = gcf(reduceE(f,v),c0);
   if (a == b) return (a);
   return (find_or_add_node_wvf(v,a,b));
}

/*****************************/

int add_k_state(int n, int k) {
   // copy the state machine pointed to by n
   // but increment all variables by k

   // initialize add_state memo table
   for (int i = 2; i < next_sm; i++) next_sm_memo[i] = -1;

   return (add_k_state1(n,k));
}

/***************************/

int add_k_state1(int n, int k) {
   int nsm,a,next_sm1;
   struct sm_node *state_var;
  
   if (next_sm_memo[n] >= 0)  // check memo table
      return (next_sm_memo[n]);
   next_sm_memo[n] = next_sm; // update memo table

   // pass 1, all but recursive calls
   nsm = next_sm;
   for (state_var = (SM+n); state_var != NULL;
        state_var = state_var->next) {
      SM[next_sm].v = state_var->v + k; // increment variable by k
      SM[next_sm].pure = 0;
      SM[next_sm].score = state_var->score;
      SM[next_sm].score_t = state_var->score_t;
      SM[next_sm].score_e = state_var->score_e;
      SM[next_sm].score_level = state_var->score_level;
      SM[next_sm].level = state_var->level;
      a = state_var->inference;
      if (a != 0) {
	      if (a < 0) a -= k;
	      else a += k;
		}
      SM[next_sm].inference = a; // update inference variable by k
      if (state_var->next == NULL) SM[next_sm].next = NULL;
      next_sm++;
   }
   // pass 1, recursive calls
   next_sm1 = nsm;
   for (state_var = (SM+n); state_var != NULL;
        state_var = state_var->next) {
      SM[next_sm1].T = (SM+add_k_state1((state_var->T)->index,k));
      SM[next_sm1].E = (SM+add_k_state1((state_var->E)->index,k));
      next_sm1++;
   }
   return (nsm);
}

/***************************/

// v is number of vars in look up table
// x is an array of binary values
int ite_wvf_binary(int v, int x[]) {
   int i,j,n,top,step,half;
   n = 1 << v;
   for (i=0; i<v; i++) { // pass i
      top = v+1-i;
      step = 2 << i;
      half = step >> 1;
      for (j=0; j<n; j = j + step)
         x[j] = ite_wvf(ite_wvf_var(top), x[j + half], x[j]);
   }
   return (x[0]);
}

/***************************/

// vars is number of vars in look up table
// bit is number of bits wide for each look up table entry
// x is an array of look up table entries
int ite_wvf_slice(int vars, int bit, int x[]) {
   int i, y[1024],n;
   n = 1 << vars;
   for (i=0; i<n; i++)
      y[i] = (x[i] >> bit) & 1; // isolate bit slice
   return (ite_wvf_binary(vars,y));
}

/***************************/

// f is a bdd, x is an array of bdds
int f_apply(int f, int x[]) {
   if (f < 2) return (f);
   return(ite_wvf(x[BDD[f].v], f_apply(BDD[f].T,x), f_apply(BDD[f].E,x)));
}

/***************************/

void is_valid_state() {
   int i;
   struct sm_node *state_var;
    
   
   for (i = 1; i < next_village; i++)
	   for (state_var = village[i]; state_var != NULL; state_var = state_var->next) {
		   if (state_var->index < 2) continue;
		   if (((state_var->score_t != 0) || (state_var->score_e != 0)) &&
		       (state_var->inference == 0)) continue;
		   printf("Invalid State found.\n");
		   return;
	   }
      return;
}

/***************************/

void solution_found() { 
   int a, i;
/*
   f = clock();
   brancher_time += (float)(f-s);
   s = f;
   */
    
   for(a = 1; a < next_village; a++)
	   if (village[a] != ONE) {
		   printf("Error: Inconsistent system found.\n");
		   printf("\nSystem is inconsistent:   %f %d\n",
		          (brancher_time / (float)CLOCKS_PER_SEC), trim_count);
		   return;
	   }
      printf("\nSystem is satisifiable:   %f %d\n",
	          (brancher_time / (float)CLOCKS_PER_SEC), trim_count);
      printf("\nCurrent Node Values:\n");

      for (i=0; i<next_inf_history; i++)
	      if (inf_history_flag[i] == 1) {
	         if (i%10 == 0) printf("\n");	    
	         if ((abs(inf_history_var[i]) < 2) ||  // < 20000
		          (abs(inf_history_var[i]) >= next_var_list))
	            printf("%6d ",inf_history_var[i]);  
//	         else
//	            printf("%s(%d) ",int_to_sym(inf_history_var[i]),
//		      inf_history_var[i]);
			}
      printf("\n\nOpen Assumptions:\n");
      for (i=0; i<next_assumption; i++) {
         if (i%10 == 0) printf("\n");
         if ((abs(assumption_array[i]) < 2) ||  // < 20000
	          (abs(assumption_array[i]) >= next_var_list))
	         printf("%6d ",assumption_array[i]);  
//         else
//	         printf("%s(%d) ",int_to_sym(assumption_array[i]),
//	      assumption_array[i]);
		}
      /*
      // place ascending order
      for (x = 0; x < next_inf_history; x++)
         for (y = x; y < next_inf_history; y++) {
            if (abs(inf_history_var[x]) > (abs(inf_history_var[y]))) {
               temp = inf_history_var[x];
               inf_history_var[x] = inf_history_var[y];
               inf_history_var[y] = temp;
				}
			}
      */
      printf("\n");
      printf("\nSolution Values:\n");
      for (i = 0; i < next_inf_history; i++) {
	      if (i%10 == 0) printf("\n");
	      if ((abs(inf_history_var[i]) < 2) ||  // < 20000
		       (abs(inf_history_var[i]) >= next_var_list))
	         printf("%6d ",inf_history_var[i]);  
//	      else
//	         printf("%s(%d) ",int_to_sym(inf_history_var[i]),
//		                       inf_history_var[i]);
		}
      printf("\n");
      print_statistics();
		return;
}

int readBDD_wvf (FILE *fin) {
  char a = fgetc(fin);
  if (a == 'T')
    return 1;
  if (a == 'F')
    return 0;

  int v;
  fscanf(fin, "[%d]", &v);
  int r = readBDD_wvf(fin);
  int e = readBDD_wvf(fin);

  a = fgetc(fin);

  return ite_wvf(ite_wvf_var(v+1), r, e);
  //return find_or_add_node_wvf(v+1, r, e);
}

void readCircuit_wvf () {
  FILE *fin;
  char a[10];
  int num = 0, which_zoom = 0, cant_do = 0;
  fin = fopen("bdd.tmp", "rb");
  for (int i=0 ; i < nmbrFunctions ; i++){
    //      fprintf(stdout, "\nWorking on constraint #%d\n", i);
    if(state(readBDD_wvf(fin)) == 1) {
      fprintf(stderr, "Clause #%d ", i);
      if(functionType[i] == AND) fprintf(stdout, "is an AND= function with Equality variable %d\n", equalityVble[i]);
      else if(functionType[i] == OR) fprintf(stdout, "is an OR= function with Equality variable %d\n", equalityVble[i]);
      else if(functionType[i] == ITE) fprintf(stdout, "is an ITE= function with Equality variable %d\n", equalityVble[i]);
      else {
   fprintf(stdout, "\n");
   cant_do++;
      }
      printBDD(functions[i]);
      fprintf(stdout, "\n");
      printBDDTree(functions[i], &which_zoom);
      flush(cout);
      num++;
    }
    fgets(a, 10, fin);
  }
  fprintf(stdout, "\nThere were %d problem constraints and %d were unparam\n", num, cant_do);
  flush(cout);
  fclose(fin);
  numinp++; //for mark cause he doesn't use 0 or 1 and cheat_replace
  //makes the varibles go from 1 - N
  unlink("bdd.tmp");
}


void
wvfSolve() {

   writeCircuit();
   ite_wvf_initialize();
   sm_initialize();
   readCircuit_wvf();
   wvf_solve();

}

/***************************/
