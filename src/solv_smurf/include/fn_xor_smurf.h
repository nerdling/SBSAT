
#ifndef FN_XOR_SMURF_H
#define FN_XOR_SMURF_H

extern int arrFnXorSmurfTypes[];

void LSGBXorSmurfUpdateFunctionInfEnd(int nFnId);
void LSGBXorSmurfGetHeurScores(int nFnId);
void LSGBWXorSmurfUpdateFunctionInfEnd(int nFnId);
void LSGBWXorSmurfGetHeurScores(int nFnId);
int FnXorSmurfInit();
void HrLSGBFnXorSmurfInit();
void HrLSGBWFnXorSmurfInit();

#endif
