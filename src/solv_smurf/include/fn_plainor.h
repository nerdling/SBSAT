

// Function structure
typedef struct {
   int nNumRHSUnknowns;
   int nNumRHSUnknownsNew;
   int nNumRHSUnknownsPrev;

   double fSumRHSUnknowns;
   double fSumRHSUnknownsNew;
   double fSumRHSUnknownsPrev;
} Function_PlainOR;

// AFS structure
typedef struct {
} AFS_PlainOR;

// Stack structure
typedef struct {
   int value;
   int prev;
   int rhscounter;
} Stack_PlainOR;


