#ifndef FN_MINMAX_H
#define FN_MINMAX_H

// Function structure
typedef struct {
   int nNumRHSUnknowns;
   int nNumRHSUnknownsNew;
   int nNumRHSUnknownsPrev;

   int min;
   int max;

   IntegerSet_ArrayBased rhsVbles;
   int *arrRHSPolarities;

   int nRHSCounter;
   int nRHSCounterNew;
   int nRHSCounterPrev;
} Function_MinMax;

// AFS structure
typedef struct {
} AFS_MINMAX;

// Stack structure
typedef struct {
   int rhs_unknowns;
   int rhs_counter;
} Stack_MinMax;

extern int arrFnMinMaxTypes[];

int MinMaxCreateFunction(int nFnId, BDDNode *bdd, int nFnType, int eqVble);
void MinMaxAffectedVarList(int nFnId, int **arr1, int *num1, int **arr2, int *num2);
void MinMaxCreateAFS(int nFnId, int nVarId, int nAFSIndex);
int MinMaxUpdateAffectedFunction(void *oneafs, int x);
int MinMaxUpdateAffectedFunction_Infer(void *oneafs, int x);
int MinMaxSave2Stack(int nFnId, void *one_stack);
int MinMaxRestoreFromStack(void *one_stack);
void MinMaxUpdateFunctionInfEnd(int nFnId);
void LSGBMinMaxUpdateFunctionInfEnd(int nFnId);
void LSGBMinMaxGetHeurScores(int nFnId);
int FnMinMaxInit();
void HrLSGBFnMinMaxInit();
void HrLSGBWFnMinMaxInit();

#endif
