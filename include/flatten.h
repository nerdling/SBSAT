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
/*********************************************************************
 *  flatten.h (J. Franco)
 *  Functions normalizeInputTrace and insertModules flatten a 
 *  collection of nested expressions in Trace format and insert
 *  libraries of Trace expressions.  All others support these two.
 *********************************************************************/
#ifndef FLATTEN_H
#define FLATTEN_H

#define min(x,y) ((x)<(y))?(x):(y)
#define B_SIZE 32000

// Structure of the module data base is as follows:                      
//  .../Modules -> the root of the data base                             
//  .../Modules/BDD -> files containing ascii-based BDD representations  
//  .../Modules/Trace -> files containing ascii-based Trace format reps  
// Action is to search Trace first, then BDD (this may change - also     
//  new subdirectories may be added - for example for CNF and other fmts 
class FlattenTrace {
   char *file, *flattened_file, *macroed_file;
   bool hash_table;
   
 public:
   FlattenTrace (char *);
   FlattenTrace (char *, int);
   ~FlattenTrace();

   // Flattens an arbitrary nested propositional expression.  Example:        
   //  3 = or(not(tr), or(qw, and(1, 2, 3), 12), and(23, z = and(2, 1), 45)); 
   // becomes, in module ww:                                                  
   //  5_0 = not(tr);                                                       
   //  5_1 = and(1, 2, 3);                                                  
   //  5_2 = or(qw, 5_1, 12);                                             
   //  z = and(2, 1);                                                         
   //  5_3 = and(23, z, 45);                                                
   //  3 = or(5_0, 5_2, 5_3);                                           
   //                                                                         
   // Assume lines are less than 16000 bytes long (seems pretty safe).        
   // Assume an op exists for each line.  In other words, can't do "x = y;"   
   // Returns the made-up temp var.  Pointer u returns end of argument in s   
   //-------------------------------------------------------------------------
   // Inputs:                                                                 
   //   s  = the line to break: a = op(b, f(...), x = g(...), ...)            
   //   ft = the file which the result is output to                           
   //   u  = pointer to char string which is the var name to go on left of "="
   //   id = number which gives uniqueness to new variables                   
   char *breakCompound (char *, FILE *, char **, int);
   int normalizeInputTrace ();
   bool builtin (char *);

   // Goes through the STRUCTURE section of a flattened circuit, line by 
   // line, looking for functions which are not builtin.  Upon finding   
   // such, makes an appropriate substitution from a file in the module  
   // data base or reports no such module exists.  The substitution is   
   // itself a flattened circuit - thus, module information is stored as 
   // editable ascii text and in a separate file in "compiled" form which
   // includes sections showing lines supporting each output of the      
   // module.                                                            
   char *insertModules ();
};
#endif
