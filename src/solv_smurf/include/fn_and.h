

// Function structure
typedef struct {
   int nLHSVble;
   int nLHSPolarity;

   IntegerSet_ArrayBased rhsVbles;
   int *arrRHSPolarities;
  
   int nNumRHSUnknowns;
   int nNumRHSUnknownsNew;
   int nNumRHSUnknownsPrev;

   double fSumRHSUnknowns;
   double fSumRHSUnknownsNew;
   double fSumRHSUnknownsPrev;

   int nNumLHSUnknowns;
   int nNumLHSUnknownsNew;
   int nNumLHSUnknownsPrev;

   LemmaBlock **arrShortLemmas;
   LemmaBlock *pLongLemma;

} Function_AND;

// AFS structure
typedef struct {
   int nRHSPos;
} AFS_AND;

// Stack structure
typedef struct {
   int rhs_unknowns;
   int lhs_unknowns;
   double rhs_sum;
} Stack_AND;

extern int arrFnAndTypes[];

ITE_INLINE int AndCreateFunction(int nFnId, BDDNode *bdd, int nFnType, int eqVble);
void AndAffectedVarList(int nFnId, int **arr1, int *num1, int **arr2, int *num2);
void AndCreateAFS(int nFnId, int nVarId, int nAFSIndex);
int AndUpdateAffectedFunction(int nFnId);
int AndUpdateAffectedFunction_Infer(void *oneafs, int x);
int AndSave2Stack(int nFnId, void *one_stack);
int AndRestoreFromStack(void *one_stack);
void AndUpdateFunctionInfEnd(int nFnId);
void LSGBAndUpdateFunctionInfEnd(int nFnId);
void LSGBGetHeurScoresFromAND(int nFnId);
int FnAndInit();
void HrLSGBFnAndInit();
void HrLSGBWFnAndInit();

void ConstructLemmasForAND(int nFnId);
