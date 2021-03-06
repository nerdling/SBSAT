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

#ifndef LOG_H
#define LOG_H
/*
 * What level of debugging gets compiled in.
 * Change this to a lower number for better performance
 */
#define _DEBUG_LVL_COMPILE 9

extern FILE* stddbg;
extern int DEBUG_LVL;
int dX_printf (int , const char* , ...);

/* -- finish this -- *
#ifdef GCC
#define ITE_DEBUG(fmt, args...)  ITE_ERR("ite(%s): "fmt, __FUNCTION__, ## args)
#endif
 * ----------------- */

#if _DEBUG_LVL_COMPILE >= 2
# define D_2(x) if ((DEBUG_LVL&15) >= 2) { x }
# define D_2E(x) if ((DEBUG_LVL&15) == 2) { x }
# define DM_2(x) D_2(if(DEBUG_LVL&16) { x } )
#else
# define D_2(x) {}
# define D_2E(x) {}
# define DM_2(x) {}
#endif

#if _DEBUG_LVL_COMPILE >= 9
# define D_9(x) if ((DEBUG_LVL&15) >= 9) { x }
# define TB_9(x) D_9(if(ite_counters[NUM_BACKTRACKS]>=TRACE_START) { x }) 
#else
# define D_9(x) {}
# define TB_9(x) {}
#endif

#if _DEBUG_LVL_COMPILE >= 0
# define D_C(x) if (competition_enable) { x }
#else
# define D_C(x) {}
#endif

#define dC_printf1(x)        D_C(printf(x);)
#define dC_printf2(x1,x2)    D_C(printf(x1, x2);)
#define dC_printf3(x1,x2,x3) D_C(printf(x1, x2, x3);)
#define dC_printf4(x1,x2,x3,x4) D_C(printf(x1, x2, x3,x4);)

#define d2_printf1(x)        D_2(fprintf(stddbg, x);)
#define d2_printf2(x1,x2)    D_2(fprintf(stddbg, x1, x2);)
#define d2_printf3(x1,x2,x3) D_2(fprintf(stddbg, x1, x2, x3);)
#define d2_printf4(x1,x2,x3,x4) D_2(fprintf(stddbg, x1, x2, x3,x4);)
#define d2_printf5(x1,x2,x3,x4,x5) D_2(fprintf(stddbg, x1, x2, x3,x4,x5);)
#define d2_printf6(x1,x2,x3,x4,x5,x6) D_2(fprintf(stddbg, x1, x2, x3,x4,x5,x6);)
#define d2_printf7(x1,x2,x3,x4,x5,x6,x7) D_2(fprintf(stddbg, x1, x2, x3,x4,x5,x6,x7);)
#define d2_printf8(x1,x2,x3,x4,x5,x6,x7,x8) D_2(fprintf(stddbg, x1, x2, x3,x4,x5,x6,x7,x8);)
#define d2_printf9(x1,x2,x3,x4,x5,x6,x7,x8,x9) D_2(fprintf(stddbg, x1, x2, x3,x4,x5,x6,x7,x8,x9);)

#define d2e_printf1(x)        D_2E(fprintf(stddbg, x);)
#define d2e_printf2(x1,x2)    D_2E(fprintf(stddbg, x1, x2);)
#define d2e_printf3(x1,x2,x3) D_2E(fprintf(stddbg, x1, x2, x3);)
#define d2e_printf4(x1,x2,x3,x4) D_2E(fprintf(stddbg, x1, x2, x3,x4);)
#define d2e_printf5(x1,x2,x3,x4,x5) D_2E(fprintf(stddbg, x1, x2, x3,x4,x5);)

#define dm2_printf2(x1,x2)    DM_2(fprintf(stddbg, x1, x2);)

#define d9_printf1(x)        D_9(fprintf(stddbg, x);)
#define d9_printf2(x1,x2)    D_9(fprintf(stddbg, x1, x2);)
#define d9_printf3(x1,x2,x3) D_9(fprintf(stddbg, x1, x2, x3);)
#define d9_printf4(x1,x2,x3,x4) D_9(fprintf(stddbg, x1, x2, x3,x4);)
#define d9_printf5(x1,x2,x3,x4,x5) D_9(fprintf(stddbg, x1, x2, x3,x4,x5);)
#define d9_printf6(x1,x2,x3,x4,x5,x6) D_9(fprintf(stddbg, x1, x2, x3,x4,x5,x6);)
#define d9_printf7(x1,x2,x3,x4,x5,x6,x7) D_9(fprintf(stddbg, x1, x2, x3,x4,x5,x6,x7);)
#define d9_printf8(x1,x2,x3,x4,x5,x6,x7,x8) D_9(fprintf(stddbg, x1, x2, x3,x4,x5,x6,x7,x8);)
#define d9_printf9(x1,x2,x3,x4,x5,x6,x7,x8,x9) D_9(fprintf(stddbg, x1, x2, x3,x4,x5,x6,x7,x8,x9);)
#define d9_printf10(x1,x2,x3,x4,x5,x6,x7,x8,x9,x10) D_9(fprintf(stddbg, x1, x2, x3,x4,x5,x6,x7,x8,x9,x10);)

#endif
