
#ifndef FN_XOR_H
#define FN_XOR_H

// Function structure
typedef struct {
   int nNumRHSUnknowns;
   int nNumRHSUnknownsNew;
   int nNumRHSUnknownsPrev;

   IntegerSet_ArrayBased rhsVbles;
   int *arrRHSPolarities;

   LemmaBlock *pLongLemma;
} Function_XOR;

// AFS structure
typedef struct {
} AFS_XOR;

// Stack structure
typedef struct {
   int rhs_unknowns;
//   int rhssum;
} Stack_XOR;
   
extern int arrFnXorTypes[];

int XorCreateFunction(int nFnId, BDDNode *bdd, int nFnType, int eqVble);
void XorAffectedVarList(int nFnId, int **arr1, int *num1, int **arr2, int *num2);
void XorCreateAFS(int nFnId, int nVarId, int nAFSIndex);
int XorUpdateAffectedFunction(int nFnId);
int XorUpdateAffectedFunction_Infer(void *oneafs, int x);
int XorSave2Stack(int nFnId, void *one_stack);
int XorRestoreFromStack(void *one_stack);
void XorUpdateFunctionInfEnd(int nFnId);
void LSGBXorInitHeuristicTables();
void LSGBXorUpdateFunctionInfEnd(int nFnId);
void LSGBXorGetHeurScores(int nFnId);
void LSGBWXorUpdateFunctionInfEnd(int nFnId);
void LSGBWXorGetHeurScores(int nFnId);
int FnXorInit();
void HrLSGBFnXorInit();
void HrLSGBWFnXorInit();

#endif
