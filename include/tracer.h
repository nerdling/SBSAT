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
/*********************************************************
 *  tracer.h (J. Franco)
 *********************************************************/
// Converts "Trace" input format to "Smurf" format                       
//                                                                       
// Usage: trace_out <trace-input-file>                                   
//                                                                       
// Assume the following definition of "Trace" format:                    
//   Possible Sections:                                                  
//     MODULE                                                            
//     INPUT                                                             
//     OUTPUT                                                            
//     STRUCTURE                                                         
//                                                                       
//   Format by section (all keywords are assumed upper case):            
//     MODULE:    None - we delete this line                             
//     INPUT:     List of comma separated variables terminating in ";"   
//                The list is on one line and possibly on the same line  
//                as the keyword "INPUT".                                
//     OUTPUT:    Same as "INPUT".                                       
//     STRUCTURE: The following types of expressions are recognized:     
//                1. <evar> = ite(<var>,<var>,<var>,<var>);              
//                2. <evar> = and(<var>,<var>,...,<var>);                
//                3. <evar> = or(<var>,<var>,...,<var>);                 
//                4. <evar> = new_int_leaf(<zero-or-one>);               
//                5. <evar> = not(<var>);                                
//                6. are_equal(<var>,<var>,...,<var>);                   
//                There may be white space between commas.               
//                Each expression takes exactly one line in the file.    
//                There must be white space on both sides of "=".        
//                There can be nothing else, such as the keyword         
//                   "STRUCTURE", on the same line as 1.-6.              
//                                                                       
// Makes three passes through the input file.  On the first pass, input  
// variables and "not"ed literals are given identities.  In the case of  
// the "not" operator, new literals are given identities as follows: if  
// <var> and <evar> are encountered for the first time, var is assigned  
// the next available number n in sequence and evar is assigned -n; else 
// if <var> is encountered for the first time, <var> is assigned the     
// negative of <evar>'s identity; else if <evar> is encountered for the  
// first time, <evar> is assigned the negative of <var>'s identity; else 
// if the identities of <var> and <evar> are not complementary, an error 
// is printed.  On the second pass, output variables and literals not    
// previously seen are assigned positive integer identities in increa-   
// sing order.  On the third pass, lines matching 1.-4. and 6. above are 
// processed and a new truth table section is built for each processed   
// line.  A truth table section contains the symbol # followed by a      
// smurf number (order of truth table in the output) followed by a list  
// of variables contained in the line (not literals) terminated by -1,   
// followed by either a line with 1's and 0's matching the truth table   
// mapping for the function expressed by the line or, if the number of   
// function variables is greater than "max_inp_lst_sz", an operator      
// followed by 1's and 0's representing the polarity of the literals on  
// the right side of "=" and a 2 or 3 representing the polarity of the   
// literal on the left side of "=".  The following shows a sample input  
// fragment and output.                                                  
//                                                                       
// Input fragment:                                                       
//   MODULE dlx1_c                                                       
//   INPUT                                                               
//     ID_EX_RegWrite, ID_EX_MemToReg, _Taken_Branch_1_1, EX_MEM_Jump, MEM_WB_RegWrite, EX_MEM_RegWrite, IF_ID_RegWrite, ID_EX_Jump, ID_EX_Branch, TakeBranchALU_0, IF_ID_Flush, e_1_1, IF_ID_UseData2, IF_ID_Branch, IF_ID_MemWrite, IF_ID_MemToReg, e_2_1, e_2_2, e_3_2, EX_MEM_MemToReg, MEM_WB_MemToReg, e_3_1, e_1_3, e_1_4, UseData2_0, e_2_3, e_2_4, e_5_2, e_1_2, e_4_2, IF_ID_Jump, TakeBranchALU_1, RegWrite_0, e_5_5, e_3_4, e_4_1, e_5_1, e_3_3, EX_MEM_MemWrite, ID_EX_MemWrite, e_4_3, e_4_4, e_5_4, MemWrite_0, e_5_3, Jump_0, Branch_0, TakeBranchALU_2, MemToReg_0, TakeBranchALU_3, TakeBranchALU_4;
//   OUTPUT  _temp_1252;                                                 
//   STRUCTURE                                                           
//     _squash_1_1 = or(_Taken_Branch_1_1, EX_MEM_Jump);                 
//     _Taken_Branch_9_1 = and(_squash_bar_1_1, ID_EX_Branch, TakeBranchALU_0);
//     _temp_976 = ite(_temp_969, IF_ID_Jump, Jump_0);                   
//     _temp_1000 = and(EX_MEM_RegWrite, _temp_279, e_2_4, _temp_984, _temp_988, _temp_993, _temp_997);
//     _temp_1056 = or(_temp_989, _temp_998, _temp_999, _temp_1000, _temp_1001, _temp_1006, _temp_1055);
//     true_value = new_int_leaf(1);                                     
//     are_equal(_temp_1252, true_value); % 1                            
//     ...                                                               
//   ENDMODULE                                                           
//                                                                       
// Output fragment:                                                      
//   295 # Number Input Variables                                        
//   245 # Number Output Variables                                       
//   111111111111111111111111111111111...1111111111111111 # Output Vector
//   #                                                                   
//   0                                                                   
//   2 3 52 -1                                                           
//   10000111                                                            
//   #                                                                   
//   1                                                                   
//   8 9 52 54 -1                                                        
//   1110111100010000                                                    
//   #                                                                   
//   2                                                                   
//   30 45 59 64 -1                                                      
//   1100101000110101                                                    
//   #                                                                   
//   3                                                                   
//   5 19 26 70 74 75 77 80 -1                                           
//   and= 10100103                                                       
//   #                                                                   
//   4                                                                   
//   71 78 79 80 81 84 125 126 -1                                        
//   or= 11111113                                                        
//   #                                                                   
//   5                                                                   
//   294 -1                                                              
//   01                                                                  
//   #                                                                   
//   6                                                                   
//   51 294 -1                                                           
//   1001                                                                
//   ...                                                                 
//   @                                                                   

#ifndef TRACER_H
#define TRACER_H

#include "parse.h"

typedef struct flat{
   struct flat *next;
   int op;
   BDDNode *bdd;
   BDDNode *args;
} Flat;

typedef struct unflat{
   struct flat *head;
   struct flat *tail;
} Unflat;

typedef struct charList {
	struct charList *next;
	char *id;
} NodeList;

class Tracer {
   Hashtable *symbols, *outputs, *inputs, *nots;
   int nsymbols, ninputs, noutputs, npos, nsmurfs;
   bool hash_table;
   Object **ins, **outs;
   int max_inp_lst_sz;

 public:
   char *file;
   Tracer (char *);
   Tracer (char *, int);
	~Tracer();
   Hashtable *getHashTable () { return symbols; }
   void explore (Node *, int, Hashtable *);
   int  parseInput ();
   void display ();
   void getSymbols (int *vars, int size);
   int intValue(char *);
   // For maintenance - must take Integer objects 
   void printHashTable (Hashtable *);
};

#endif
