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
#include "sbsat_preproc.h"

typedef int (*preproc_fn_type)();
typedef int (*preproc_fn_param_type)(char *);

typedef struct {
  char id[5];
  int id_len;
  preproc_fn_type fn;
  preproc_fn_param_type fn_param;
  int *enable;
} preproc_type;

/* Never start with digit */
/* Never use the same prefix -- 'Pr' and 'Pr1' is very bad 
 *    although if the order is right it will get parsed correctly 
 *    so -- at least try to sort it by prefix length in the descending order */
/* Try to start with a capital letter */
/* Try to keep the fixed size */
int Do_Calc();
int Do_Calc_Param(char *param);
int DO_CALC=1;

preproc_type preproc[] = {
  { "Calc", 4, Do_Calc, Do_Calc_Param, &DO_CALC},
  { "St", 2, Do_Strength,    NULL, &DO_STRENGTH },
  { "Sa", 2, Do_SafeAssign,  NULL, &DO_SAFE_ASSIGN },
  { "Ss", 2, Do_SafeSearch,  NULL, &DO_SAFE_SEARCH },
  { "Pr", 2, Do_Pruning,     NULL, &DO_PRUNING },
  { "P1", 2, Do_Pruning_1,   NULL, &DO_PRUNING },
  { "P2", 2, Do_Pruning_2,   NULL, &DO_PRUNING },
  { "Sl", 2, Do_Steal,       NULL, &DO_PRUNING },
  { "PR", 2, Do_Pruning_Restrict, NULL, &DO_PRUNING },
  { "Co", 2, Do_Cofactor,    NULL, &DO_COFACTOR },
  { "Ex", 2, Do_ExQuantify,  NULL, &DO_EXIST_QUANTIFY },
  { "Ea", 2, Do_ExQuantifyAnd, NULL, &DO_EXIST_QUANTIFY_AND },
  { "Es", 2, Do_ExSafeCluster, NULL, &DO_EX_SAFE_CLUSTER },
  { "Pa", 2, Do_PossibleAnding, NULL, &DO_POSSIBLE_ANDING },
  { "Dc", 2, Do_DepCluster, NULL, &DO_DEP_CLUSTER },
  { "Sp", 2, Do_Split, NULL, &DO_SPLIT },	
  { "Rw", 2, Do_Rewind, NULL, &DO_REWIND },
  { "Cf", 2, Do_Clear_FunctionType, NULL, &DO_CLEAR_FUNCTION_TYPE },
  { "Ff", 2, Do_Find_FunctionType, NULL, &DO_FIND_FUNCTION_TYPE },
  { "P3", 2, Do_Prover3, NULL, &DO_PROVER3},
  { "Is", 2, Do_Identify_Same_Structure, NULL, &DO_IDENTIFY_SAME_STRUCTURE},
  { "Er", 2, Do_ExtendRes, NULL, &DO_EXTEND_RES},
  { "Di", 2, Do_Diameter, NULL, &DO_DIAMETER},
  { "Mp", 2, Do_Message_Passing, NULL, &DO_MESSAGE_PASSING},
  { "", 0, NULL, NULL, NULL }
};

int parse_seq(char **p, int parse_only);
int parse_cycle(char **p, int parse_only);

int Preprocessor() {
	char *p;
	int ret = PREP_NO_CHANGE;
	
	if (DO_INFERENCES) {
		ret = Do_Apply_Inferences();
		num_inferences = Pos_replace + Neg_replace + Setting_Pos + Setting_Neg;
		
		d3_printf1("\n");
		if (ret == TRIV_UNSAT || ret == TRIV_SAT || ret == PREP_ERROR)
		  return ret; 
	}
	
	p = preproc_string;
	ret = parse_seq(&p, 0);
	return ret;
}

int
parse_cycle(char **p, int parse_only)
{
  int ret = PREP_NO_CHANGE;
  int limit=0;
  assert(**p=='{');
  (*p)++;
  char *start_p = *p;
  int loop_counter=0;

  /* skip over the inside to find the number of iterations */
  ret = parse_seq(p, 1);

  if (parse_only == 0) 
  {
    if (**p!='}') {
      dE_printf2("Preproc: Format error: 1 missing '}':%s\n", *p);
      return PREP_ERROR;
    }
    (*p)++;

    /* get the no of loops limit */
    if (**p == '*') 
    { 
      limit=INT_MAX;
      (*p)++;
    }
    else
      if (isdigit(**p)) 
      {
        limit = 0;
        while (isdigit(**p)) 
        {
          limit = limit * 10 + (**p-'0');
          (*p)++;
        }
      } 
    /* if no quantifyer is present */
      else limit = INT_MAX; /* ???? */

      if (limit == 0) return PREP_NO_CHANGE;

      do
      {
        *p = start_p;
        ret = parse_seq(p, 0);
        loop_counter++;
      } 
      /* while it has changed and we are still within the limit */
      while (loop_counter<limit && ret == PREP_CHANGED); 

      switch (ret) {
        case TRIV_UNSAT:  /* Unsat */
        case TRIV_SAT:    /* Sat */
        case PREP_ERROR:  /* Error */
          d9_printf2("pc: returning with %d\n", ret);
          return ret; 
        case PREP_CHANGED:   break; /* continuing */
        case PREP_NO_CHANGE: break; /* continuing */
        default: break; /* ?? */
      };
  }

  if (**p!='}') {
    dE_printf2("Preproc: Format error: missing '}':%s\n", *p);
    return PREP_ERROR;
  }
  (*p)++;

  /* skip the limits */
  if (**p == '*') 
  { 
    (*p)++;
  }
  else
    if (isdigit(**p)) 
    {
      while (isdigit(**p)) 
      {
        (*p)++;
      }
    }

  /* if it had to repeat at least once then everything needs to repeat */
	if (ret == PREP_NO_CHANGE && loop_counter>1) 
	  ret=PREP_CHANGED;
	d9_printf2("pc: returning with %d\n", ret);
	return ret;
}


int
parse_seq(char **p, int parse_only)
{
  int ret=PREP_NO_CHANGE; /* unchanged */
  int r;

  while (1) {
     double mytime = get_runtime(); 
     if (max_preproc_time)
     d2_printf5("Checking preproc time: %f - %f = %f > %d\n", mytime, start_prep, mytime - start_prep, max_preproc_time);
     if (max_preproc_time && (mytime - start_prep) > max_preproc_time) {
        DO_STRENGTH = 0;
        DO_PRUNING = 0;
        DO_DEP_CLUSTER = 0;
        //return PREP_ERROR;
     }
  
     if (**p == '{')
     {
        r = parse_cycle(p, parse_only);
     }
     else if (**p == '[')
     {
        (*p)++;
        r = parse_seq(p, parse_only);
		  if(r == PREP_CHANGED || r == PREP_NO_CHANGE) {
			  assert(**p == ']');
			  (*p)++;
			  if (isdigit(**p)) {
				  if ((**p)-'0' == 0) r=PREP_NO_CHANGE;
				  else r=PREP_CHANGED;
				  (*p)++;
			  }
		  }
     }
     else if (**p == ']')
     {
        break;
     }
     else if (**p == '}')
     {
        break;
     }
     else if (**p == 0)
     {
        break;
     }
     else
     { 
        r = PREP_MAX;
        for (int i=0;preproc[i].id[0];i++) {
           if (!strncmp(preproc[i].id, *p, preproc[i].id_len)) 
           {	
              (*p)+=preproc[i].id_len;
              if ((**p)=='(') { // arguments to the function
                 char *start=(*p)+1;
                 while((**p) && (**p)!=')') {
                    (*p)++;
                    if ((**p)=='{' || (**p)=='}' || (**p)=='(') {
                       dE_printf2("Preproc: Format error: illegal character :%s\n", (*p));
                       r = PREP_ERROR;
                       break;
                    }
                    (*p)++;
                 }
                 if ((**p)!=')') {
                    dE_printf2("Preproc: Format error: missing ')':%s\n", start);
                    r = PREP_ERROR;
                    break;
                 }
                 if (parse_only == 0 && *(preproc[i].enable) == 1 && preproc[i].fn_param) {
                    (**p)=0;
                    r = preproc[i].fn_param(start);
                    (**p)=')';
                    if (r==PREP_ERROR) break;
                 }
                 (*p)++;
              }
              if (parse_only == 0) {
                 if (*(preproc[i].enable) == 1) {
                    r = preproc[i].fn();
                    d9_printf3("ps: returning from %s with %d\n", preproc[i].id, r);
                    if (enable_gc) bdd_gc();
                 }
                 else r=PREP_NO_CHANGE;
              }
              else r=PREP_NO_CHANGE;
              break;
           }
        }
        if (r == PREP_MAX) {
           dE_printf2("The preprocessing function at the position %s was not found.\n", *p);
           r = PREP_ERROR;
           break;
        };
     }
     switch (r) {
      case TRIV_UNSAT:  /* Unsat */
      case TRIV_SAT:    /* Sat */
      case PREP_ERROR:  /* Error */
         d9_printf2("ps: returning with %d\n", r);
         ret=r; return ret; 
      case PREP_CHANGED: ret=PREP_CHANGED; break;
      case PREP_NO_CHANGE: break; /* unchanged */
      default: break;
     };
  }
  d9_printf2("ps: returning ret with %d\n", ret);
  return ret;
}

int Do_Calc() {
   fprintf(stderr, "Calc\n");
   return PREP_NO_CHANGE;
}

int Do_Calc_Param(char *param) {
   if (param == NULL) return PREP_NO_CHANGE;
   fprintf(stderr, "Calc(%s)\n", param);
   return PREP_NO_CHANGE;
}
