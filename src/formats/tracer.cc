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
 *  tracer.cc (J. Franco)
 *********************************************************/
// Converts "Trace" input format to "BDD" format                         
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

#include "ite.h"


extern char *builtin_ops[];
//extern BDDNode *true_ptr, *false_ptr;

Tracer::Tracer (char *filename) {
   file = new char[strlen(filename)+1];
   strcpy(file, filename);
   symbols = new Hashtable();
   outputs = new Hashtable();
   inputs  = new Hashtable();
   nots = new Hashtable();
   nsymbols = noutputs = ninputs = 1;
   nsmurfs = 0;
   max_inp_lst_sz = 7;
   hash_table = false;
}
   
Tracer::Tracer (char *filename, int sw) {
   file = new char[strlen(filename)+1];
   strcpy(file, filename);
   symbols = new Hashtable();
   outputs = new Hashtable();
   inputs  = new Hashtable();
   nots = new Hashtable();
   nsymbols = noutputs = ninputs = 1;
   nsmurfs = 0;
   max_inp_lst_sz = 7;
   if (sw == 1) hash_table = true; else hash_table = false;
}

Tracer::~Tracer() {
	delete symbols;
	delete outputs;
	delete inputs;
	delete nots;
	delete [] file;
}

/**/
void Tracer::explore (Node *node, int ns, Hashtable *sym) {
   for (Cell *c=node->getNext() ; c != NULL ; c=c->getNext()) {
      if (c->getNode()->getMark()) continue;
      explore(c->getNode(), -ns, sym);
   }
   //** Make a check to see whether a previous value is the same as
   //** this one.  Previous value should be null.  If not null and 
   //** different then there is an error.
   // if (!sym.get(node.getId().equals(new Integer(ns)))) then error...
   sym->put(node->getId(), new Integer(ns, node->getId()));
   node->setMark();
}
/**/

#define B_SIZE 32000

int intcmp(const void *x, const void *y) {
   if (*(const int *)x < *(const int *)y) return -1;
   if (*(const int *)x > *(const int *)y) return 1;
   return 0;
}

int
Tracer::parseInput () {
   Object **obj = NULL;
   StringTokenizer
      *func = new StringTokenizer("",""),
      *r = new StringTokenizer("new"," "),
      *t = new StringTokenizer("new"," ");
   Substring 
      *substring = new Substring();
   Trimmer
      *trim = new Trimmer();
   char
      *l = NULL,
      *s = NULL,
      *v = NULL,
      *mer = NULL,
      *first_op = NULL,
      *out = NULL,
      *sym = NULL,
      *rem = NULL,
      *token = NULL;
   char 
      sb[2048], 
      op[128], 
      first[128], 
      path[512], 
      line[4100],          // buffer for a line of a module file
      line_xlate[4100],    // output for translation of line of smurf module
      arg_lst[100][100],   // For translating actual parms to formal    
      fun_lst[100][100];   // For translating actual parms to formal    
   int 
      lat_lst[100],        // permutation of 1,2,3... showing order
      srt_lst[100],        // For mapping variables to their increasing order
      map_lst[100],        // For mapping variables to their increasing order
      state=0, 
      group=1, 
      group_cnt=0, 
      temp_group=0;
   bool 
      in_group = false, 
      group_bdd_done = false,
      in_block = false,
      single_line = false;
   
   // Pass 1: Find input variables and "not"ed literals and    
   //         construct a symbol table.                        
   //         Count the numbers of Smurfs.                     
   //         All vars and smurf count are found in the first  
   //         two passes in order to write the preamble in the 
   //         output file.                                     
 
   int lineno=0; 
   int lineno2=0; 
   FILE *fd, *fc;
   if ((fd = fopen(file, "rb")) != NULL) {
      while ((s = fgets(sb, 2047, fd)) != NULL) {
	 if (++lineno % 100 == 0) 
          {
            extern int flatten_lines; /* from flatten */
            d2_printf3("\rPass 1/3: %d of %d   ", lineno, flatten_lines);
          }
	 if  (s[0] == '%' && s[1] != '%') continue;
	 switch (state) {
	 case 0:  // Startup - MODULE section 
	    t->renewTokenizer(s, " ");
	    token = trim->get(t->nextToken());
	    if (strcmp(token, "MODULE")) {
	       fprintf(stderr, "Unexpected first line - want MODULE - got [%s]\n", token); 
	       unlink(file);         
	       goto finished;
	    }
	    state = 1;
	    break;
	 case 1:  // Looking for input variables - INPUT section 
	    // All inputs are on same line which may be    
	    // shared with the keyword "INPUT".            
	    t->renewTokenizer(s, " ,;");
	    token = trim->get(t->nextToken());
	    if (!strcmp(token, "INPUT")) {
	       if (!t->hasMoreTokens()) {
		  if ((s = fgets(sb, 2047, fd)) == NULL) {
		     fprintf(stderr, "Premature end of file\n");
		     exit(1);
		  }
		  if (!strncmp(s,"OUTPUT",6)) {
		     state = 2;
		     goto output_sec;
		  }
		  t->renewTokenizer(s, " ,;");
	       } else {
		  rem = substring->get(s, indexOf(s,' '), strlen(s));
		  t->renewTokenizer(rem, " ,;");
	       }
	       while (t->hasMoreTokens()) {
		  sym = trim->get(t->nextToken());
		  if (symbols->get(sym) == NULL)
		     symbols->put(sym, new Integer(nsymbols++, sym));
		  if (inputs->get(sym) == NULL)
		     inputs->put(sym, new Integer(ninputs++, sym));
	       }
	       //ins = inputs->entrySet(); // Array of objects in the table 
	       state = 2;
	    }
	    break;
	 case 2:  // Looking for output variables - OUTPUT section 
	    // All inputs are on same line which may be      
	    // shared with the keyword "OUTPUT".             
output_sec:
            t->renewTokenizer(s, " ");
	    token = trim->get(t->nextToken());
	    if (!strcmp(token, "OUTPUT")) {
	       if (!t->hasMoreTokens()) {
		  if ((s = fgets(sb, 2048, fd)) == NULL) {
		     fprintf(stderr, "Premature end of file\n");
		     exit(1);
		  }
		  t->renewTokenizer(s, " ,;");
	       } else {
		  rem = substring->get(s, indexOf(s, ' '), strlen(s));
		  t->renewTokenizer(rem, " ,;");
	       }
	       while (t->hasMoreTokens()) {
		  sym = trim->get(t->nextToken());
	       }
	       state = 3;
	    }
	    break;
	 case 3: 
	    if (s[0] == 'S') 
	    if (!strncmp(s,"STRUCTURE",9)) break;

	    if (s[0] == '&' || s[0] == '%')  
	    if (!strncmp(s,"&&begingroup",12)) {
	       in_group = true;
	       single_line = true;
	       group_bdd_done = false;
	       break;
	    } else if (!strncmp(s,"&&endgroup",10)) {
	       in_group = false;
	       break;
	    } else if (!strncmp(s,"%%begin",7)) {
	       in_block = true;
	       single_line = false;
	       break;
	    } else if (!strncmp(s,"%%end",5)) {
	       in_block = false;
	       if (in_group) group_bdd_done = true;
	       break;
	    }

	    if (in_group && group_bdd_done) {
	       nsmurfs++;
	       break;
	    }
		 
	    if (single_line) {
	       single_line = false;
	       group_bdd_done = true;
	    }

	    // Get all variables which show up in STRUCTURE   
	    // section are part of "not" functions.           
	    t->renewTokenizer(s, " (");
	    strncpy(first, trim->get(t->nextToken()), 127);

	    if (first[0]=='E')
	    if (!strncmp(first,"ENDMODULE",9)) goto we1;
	    // Look for special operators such as "are_equal".  
	    // Parse the argument list (between parens) and add 
	    // previously unseen literals to the hash table.    
	    if (indexOf(s, '(') >= 0) {
	       if ((first_op = trim->get(substring->get(s, 0, indexOf(s, '(')))) == NULL) {
		  fprintf(stderr, "Operator expected in \"%s\"\n", s);
		  exit (1);
	       }
	       if (!strcmp(first_op, "are_equal")) {
		  nsmurfs++;
		  rem = substring->get(s, indexOf(s, '(')+1, indexOf(s, ')')+1);
		  t->renewTokenizer(rem, " ,)");
		  while (t->hasMoreTokens()) {
		     v = trim->get(t->nextToken());
		  }
		  break;
	       }
	    }
	    
	    // Look for all equivalence statements (with operator "=")  
	    token = trim->get(t->nextToken());
	    if (token[0] == '=' && token[1] == 0) {
	       strncpy(op,trim->get(substring->get(s, indexOf(s, '=')+1, indexOf(s, '('))),127);
	       // In the case of "not" merely keep track of which named  
	       // literals (such as "_temp_1256, ID_JUMP_0") are         
	       // complementary. The number of smurfs remains unchanged. 
	       if (!strcmp(op, "not")) {
		  v = trim->get(substring->get(s, indexOf(s, '(')+1, indexOf(s, ')')));
		  /**/
		  if (nots->get(first) == NULL) nots->put(first, new Node(first));
		  if (nots->get(v) == NULL) nots->put(v, new Node(v));
		  Node *src = (Node *)nots->get(v);
		  Node *dst = (Node *)nots->get(first);
		  src->setNext(new Cell(dst, src->getNext()));
		  dst->setUp(src);
		  /**/
	       } else { 
		  nsmurfs++;
	       }
	    } else {
	       // If the STRUCTURE line is not an equivalence op and not 
	       // one of the recognized relations such as "are_equal"    
	       // then give this error message.                          
	       fprintf(stderr, "Do not recognize (1) [%s]\n", s);
               fflush(stderr);
	       unlink(file);         
	       goto finished;
	    }
	    break;
	 }
      }
      fprintf(stderr, "Premature end of file\n");
      exit(1);
we1:
      fclose(fd);
   } else {
      fprintf(stderr, "File not found\n");
      goto finished;
   }

   /**/
   obj = nots->entrySet();
   if (obj != NULL) {
      for (int i=0 ; obj[i] != NULL ; i++) {
			Node *noted = (Node *)obj[i];
			if (noted->getMark() || noted->getUp() != NULL) continue;
			Integer *number = (Integer *)symbols->get(noted->getId());
			if (number == NULL) {
				explore(noted, nsymbols++, symbols);
			} else {
				explore(noted, number->intValue(), symbols);
			}
      }
		delete [] obj;
	}
   /**/
   state = 0;
   in_group = false;
   group_bdd_done = false;
   in_block = false;
   single_line = false;
   lineno2=0;
   // Pass 2: Associate literal identities with output variables 
   //         and STRUCTURE functions excluding "not" functions. 
   if ((fd = fopen(file, "rb")) != NULL) {
      while ((s = fgets(sb, 2047, fd)) != NULL) {
	 if (++lineno2 % 100 == 0) 
         {
	   d2_printf3("\rPass 2/3: %d out of %d          ", lineno2, lineno);
         }
	 //fprintf(stderr, "\n2:QQQ: %s\n", s); fflush(stderr);
	 if (s[0] == '%' && s[1] != '%') continue;
	 switch (state) {
	 case 0:  // Startup - MODULE section 
	    t->renewTokenizer(s, " ");
	    token = trim->get(t->nextToken());
	    if (strcmp(token, "MODULE")) {
	       fprintf(stderr, "Unexpected first line - want MODULE\n");
	       unlink(file);         
	       goto finished;
	    }
	    state = 1;
	    break;
	 case 1:  // Looking for input variables - INPUT section 
	    // All inputs are on same line which may be    
	    // shared with the keyword "INPUT".            
	    t->renewTokenizer(s, " ");
	    token = trim->get(t->nextToken());
	    if (!strcmp(token, "INPUT")) {
	       if (!t->hasMoreTokens()) {
		  s = fgets(sb, 2047, fd);
		  t->renewTokenizer(s, " ,;");
	       } else {
		  rem = substring->get(s, indexOf(s,' '), strlen(s));
		  t->renewTokenizer(rem, " ,;");
	       }
	       while (t->hasMoreTokens()) {
		  sym = trim->get(t->nextToken());
	       }
	       state = 2;
	    }
	    break;
	 case 2:  // Looking for output variables - OUTPUT section 
	    // All inputs are on same line which may be      
	    // shared with the keyword "OUTPUT".             
	    t->renewTokenizer(s, " ");
	    token = trim->get(t->nextToken());
	    if (!strcmp(token, "OUTPUT")) {
	       if (!t->hasMoreTokens()) {
		  s = fgets(sb, 2047, fd);
		  t->renewTokenizer(s, " ,;");
	       } else {
		  rem = substring->get(s, indexOf(s,' '),strlen(s));
		  t->renewTokenizer(rem, " ,;");
	       }
	       while (t->hasMoreTokens()) {
		  sym = trim->get(t->nextToken());
		  if (!strcmp(sym,"STRUCTURE")) break;
		  if (symbols->get(sym) == NULL)
		     symbols->put(sym, new Integer(nsymbols++, sym));
		  if (outputs->get(sym) == NULL)
		     outputs->put(sym, new Integer(noutputs++, sym));
	       }
	       //outs = outputs->entrySet();
	       state = 3;
	    }
	    break;
	 case 3: // Get all variables which show up in STRUCTURE   
	    // section.                                       
	    if (s[0] == 'S' || s[0] == 'E' || s[0]=='&' || s[0] == '%')
	    if (!strncmp(s,"STRUCTURE",9)) break;
	    else if (!strncmp(s,"ENDMODULE",9)) break;
	    else if (!strncmp(s,"&&begingroup",12)) {
	       in_group = true;
	       single_line = true;
	       group_bdd_done = false;
	       break;
	    } else if (!strncmp(s,"&&endgroup",10)) {
	       in_group = false;
	       break;
	    } else if (!strncmp(s,"%%begin",7)) {
	       in_block = true;
	       single_line = false;
	       break;
	    } else if (!strncmp(s,"%%end",5)) {
	       in_block = false;
	       if (in_group) group_bdd_done = true;
	       break;
	    }

	    if (in_group && group_bdd_done) {
	       t->renewTokenizer(s," ");
	       while (t->hasMoreTokens()) {
		  v = trim->get(t->nextToken());
		  if (symbols->get(v) == NULL)
		     symbols->put(v, new Integer(nsymbols++, v));
	       }
	       break;
	    }

	    if (single_line) {
	       single_line = false;
	       group_bdd_done = true;
	    }

	    t->renewTokenizer(s, " (");
	    strncpy(first, trim->get(t->nextToken()), 127);
	    // Look for special operators such as "are_equal".  
	    // Parse the argument list (between parens) and add 
	    // previously unseen literals to the hash table.    
	    if (indexOf(s, '(') >= 0) {
	       if ((first_op = trim->get(substring->get(s, 0, indexOf(s,'(')))) == NULL) {
		  fprintf(stderr, "Operator expected in \"%s\"\n", s);
		  exit (1);
	       }
	       if (!strcmp(first_op, "are_equal")) {
		  rem = substring->get(s, indexOf(s,'(')+1, indexOf(s,')')+1);
		  t->renewTokenizer(rem, " ,)");
		  while (t->hasMoreTokens()) {
		     v = trim->get(t->nextToken());
		     if (symbols->get(v) == NULL)
			symbols->put(v, new Integer(nsymbols++, v));
		  }
		  break;
	       }
	    }
	    
	    // Look for all equivalence statements (with operator "=")  
	    token = trim->get(t->nextToken());
	    if (token[0]=='=' && token[1]==0) {
	       strncpy(op, trim->get(substring->get(s, indexOf(s,'=')+1, indexOf(s,'('))), 127);
	       // In the case of "not" merely keep track of which named  
	       // literals (such as "_temp_1256, ID_JUMP_0") are         
	       // complementary. The number of smurfs remains unchanged. 
	       if (!strcmp(op,"not")) {
		  v = trim->get(substring->get(s, indexOf(s,'(')+1,indexOf(s,')')));
	       } else { 
		  // In other cases, such as "and", "or", "ite", or        
		  // "new_int_leaf", record new literals and increase the  
		  // number of smurfs.                                     
		  if (symbols->get(first) == NULL)
		     symbols->put(first, new Integer(nsymbols++, first));
		  // Operator "new_int_leaf" is expected to hold a constant 
		  // 0 or 1 as argument so we do not parse the argument     
		  // list for that operator here.  Argument lists of all    
		  // other operators are parsed.                            
		  bool special_op = true;
		  for (int nop=0 ; builtin_ops[nop] != NULL ; nop++) {
		     if (!strcmp(builtin_ops[nop],op)) {
			special_op = false;
			break;
		     }
		  }
		  if (strcmp(op,"new_int_leaf")) {
		     rem = substring->get(s, indexOf(s,'(')+1,indexOf(s,')'));
		     t->renewTokenizer(rem, " ,)");
		     while (t->hasMoreTokens()) {
			v = trim->get(t->nextToken());
			if (special_op) 
			   special_op = false;
			else if (symbols->get(v) == NULL)
			   symbols->put(v, new Integer(nsymbols++, v));
		     }
		  }
	       }
	    } else if (!strncmp(s,"&&begingroup",12)) {
	       break;
	    } else if (!strncmp(s,"&&endgroup",10)) {
	       break;
	    } else {
	       // If the STRUCTURE line is not an equivalence op and not 
	       // one of the recognized relations such as "are_equal"    
	       // then give this error message.                          
	       fprintf(stderr, "Do not recognize (2) [%s]\n", s);fflush(stderr);
	       unlink(file);         
	       goto finished;
	    }
	    break;
	 }
      }
      fclose(fd);
   } else {
      fprintf(stderr, "File not found\n");
      goto finished;
   }
   
   if (hash_table) { 
      printHashTable(symbols);
      unlink(file);         
      goto finished; 
   }
   
   // Pass 3: Build Smurfs (ignore not)
   
   bdd_circuit_init(nsymbols+2, nsmurfs+2);

   group = 1;
   group_cnt = 0;
   in_group = false;
   group_bdd_done = false;
   in_block = false;
   single_line = false;

   lineno2 = 0;	
   // Open the file a third time for building smurfs 
   if ((fd = fopen(file, "rb")) != NULL) {
      int *var_list = new int[100]; //100 should be max_inp_lst_sz
      int *uns_list = new int[100]; //100 should be max_inp_lst_sz
	while ((s = fgets(sb, 2047, fd)) != NULL) {
	   if (++lineno2 % 100 == 0) 
	   {
             d2_printf3("\rPass 3/3: %d out of %d        ", lineno2, lineno);
           }
	   if (s[0] == '%' && s[1] != '%') continue;
	   if (!strncmp(s,"&&begingroup",12)) {

	      temp_group = nmbrFunctions;

	      in_group = true;
	      in_block = false;
	      group_bdd_done = false;
	      single_line = true;
	      continue;
	   } else if (!strncmp(s,"&&endgroup",10)) {
	      in_group = false;
	      in_block = false;
	      group += group_cnt;
	      continue;
	   } else if (!strncmp(s,"%%begin",7)) {
	      if (in_group && !in_block) group_cnt = 0;
	      in_block = true;
	      single_line = false;
	      continue;
	   } else if (!strncmp(s,"%%end",5)) {
	      in_block = false;
	      if (in_group) { 
		group_bdd_done = true;
	        for(int x = temp_group+1; x < nmbrFunctions; x++) {
	          functions[temp_group] = ite_and(functions[temp_group], functions[x]);
		  fprintf(stderr, "&");
		}
		  nmbrFunctions = temp_group;
	      }
	      continue;
	   }

	   Integer *number;
	   if (in_group && group_bdd_done) {
	      if(single_line) nmbrFunctions = temp_group;
	      single_line = false;
	     
	      t->renewTokenizer(s, " ;");
	      int cnt = t->countTokens();

	      int *num_list = (int *)calloc(1,(cnt+1)*sizeof(int));
	      num_list[0] = cnt;
	      for (int i=1 ; i < cnt+1 ; i++) {
		 v = trim->get(t->nextToken());

		 if ((number = (Integer *)symbols->get(v)) == NULL) {
		    fprintf(stderr,"Something wrong at A\n");
		    exit(1);
		 }
		 num_list[i] = number->intValue();
	      }
	     
              functionType[nmbrFunctions] = UNSURE;
	      parameterGroup[nmbrFunctions] = 
	      parameterGroup[temp_group];	      
	     
	      parameterizedVars[nmbrFunctions] = num_list;
              
	      functions[nmbrFunctions++] = 
	        mitosis(functions[temp_group], 
			parameterizedVars[temp_group],
			num_list);
	      continue;
	   }

	   if (single_line) {
	      group_bdd_done = true;
	   }

	   //	   fprintf(stderr,"\rDoing %d/%d", nmbrFunctions, nsmurfs);
	   t->renewTokenizer(s, " (");
	   strncpy(first, trim->get(t->nextToken()), 127);
	   // Look for special operators such as "are_equal".       
	   // If found, parse the argument list (between parens).   
	   // Construct an ordered var_list from the argument list. 
	   // Output a "#", smurf number, and argument list on      
	   //   separate lines.                                     
	   // Output the truth table values.                        

	   if (indexOf(s,'(') >= 0) {
	      if ((first_op = trim->get(substring->get(s, 0, indexOf(s,'(')))) == NULL) {
		 fprintf(stderr, "Operator expected in \"%s\"\n", s);
		 exit (1);
	      }

	      // Handle the "are_equal" relation 
	      if (!strcmp(first_op, "are_equal")) {
		 // Get the list of arguments (between the parens)...
		 rem = substring->get(s, indexOf(s,'(')+1,indexOf(s,')')+1);
		 r->renewTokenizer(rem, " ,)");
		 int vcount = 0;
		 // ... and put them into var_list, in order of hash values 
		 while (r->hasMoreTokens()) {
		    v = trim->get(r->nextToken());
		    int va = ((Integer *)symbols->get(v))->intValue();
		    int i;
		    for (i=0 ; i < vcount ; i++) {
		       if (va == var_list[i]) break;
		       if (va == -var_list[i]) {
			  fprintf(stderr, "Trivially unsatisfiable: are_equal(...a,!a...)\n");
			  unlink(file);         
			  goto finished;
		       }
		    }
		    if (i == vcount) {
		       for (i=vcount ; i > 0 ; i--) {
			  if (abs(var_list[i-1]) < abs(va)) break;
			  var_list[i] = var_list[i-1];
		       }
		       var_list[i] = va;
		       vcount++;
		    }            
		 }
	       
		 // Build the "are_equal" BDD
		 if (vcount <= 1) {
		    fprintf(stderr, "Error: are_equal has fewer than two arguments\n");
		    continue;
		 }
		 int i=0;
		 BDDNode *nots = true_ptr;
		 BDDNode *vars = true_ptr;
		 for (i=0; i < vcount ; i++) {
		    int vthis = (var_list[i]);
		    nots = ite_and(ite_not(ite_var(vthis)), nots);
		    vars = ite_and(ite_var(vthis), vars);
		 }
		 if (in_group) parameterGroup[nmbrFunctions] = group+group_cnt++;
		 functions[nmbrFunctions++] = ite_or(nots, vars);
		 continue;
	      }
	   }
	   if (!t->hasMoreTokens()) continue;
	   token = trim->get(t->nextToken());
	   // Handle all equivalence statements here 
	   if (!strcmp(token, "=")) {
	      bool pair_is_comp = false;
	      bool equiv_in_args = false;
	      bool equiv_neg_in_args = false;
	      strncpy(op, trim->get(substring->get(s, indexOf(s,'=')+1,indexOf(s,'('))), 127);
	      // In cases other than the "not" operator, build a smurf 
	      if (strcmp(op,"not")) {
		 // Put the equivalence variable (equiv) into the var_list. 
		 int equiv_index;
		 int vcount = 0;
		 int equiv = ((Integer *)symbols->get(first))->intValue();
	   	 
		 equiv_index = vcount; //equiv is always the 0th element of uns_list
		 uns_list[vcount] = equiv;
	       
                 bool special_op = true;
		 for (int nop=0 ; builtin_ops[nop] != NULL ; nop++) {
		    if (!strcmp(builtin_ops[nop],op)) {
		       special_op = false;
		       break;
		    }
		 }
		 var_list[vcount++] = equiv;
		 // If the operator is not "new_int_leaf", arguments are 
		 // literals which must be parsed.  Parse the argument   
		 // list and put the remaining literals into var_list,   
		 // keeping var_list in increasing order.                
		 // Rules:                                               
		 //    1.                                                
		 if (strcmp(op, "new_int_leaf")) {
		    rem = substring->get(s, indexOf(s,'(')+1,indexOf(s,')')+1);
		    r->renewTokenizer(rem, " ,)");
		    while (r->hasMoreTokens()) {
		       v = trim->get(r->nextToken());
		       if (special_op) { special_op = false; continue; }
		       int va = ((Integer *)symbols->get(v))->intValue();
		       int i;
		       for (i=0 ; i < vcount ; i++) {
			  if (va == var_list[i]) {
			     if (i == equiv_index && equiv_neg_in_args)
				pair_is_comp = true;
			     break;
			  }
			  if (va == -var_list[i]) {
			     if (i != equiv_index || equiv_in_args)
				pair_is_comp = true;
			     break;
			  }
		       }
		       if (i == vcount) { //if va is not in var_list
			  for ( ; i > 0 ; i--) {
			     if (abs(var_list[i-1]) < abs(va)) break;
			     var_list[i] = var_list[i-1];
			     if (i-1 == equiv_index) equiv_index++;
			  }
			  var_list[i] = va;
		       }
		       uns_list[vcount++] = va;
		     
		       if (va == equiv) {
			  equiv_in_args = true;
		       } else if (va == -equiv) {
			  equiv_neg_in_args = true;
		       }
		    }
		 }

		 // Construct the BDDs for each Boolean function 
		 if (!strcmp(op,"new_int_leaf")) {
		    // Process "new_int_leaf" operator 
		    
		    // Truth table for "new_int_leaf" 
		    //  equ = arg   f  
		    //  ----------|--- 
		    //   0     0  | 1  
		    //   0     1  | 0  
		    //   1     0  | 0  
		    //   1     1  | 1  
		    
		    // Determine whether arg=0 or 1 and output the truth table
		    rem = trim->get(substring->get(s, indexOf(s,'(')+1,indexOf(s,')')));
		    BDDNode *int_leaf;
		    if (!strcmp(rem,"1")) {
		       int_leaf = ite_var(equiv);
		    } else if (!strcmp(rem,"0")) {
		       int_leaf = ite_var(-equiv);
		    } else {
		       fprintf(stderr, "New_int_leaf must have 0 or 1 argument\n");
		       exit (1);
		    }
		    if (in_group) parameterGroup[nmbrFunctions] = group+group_cnt++;
		    functions[nmbrFunctions++] = int_leaf;
		 } else if (!strcmp(op,"")) {
		    fprintf(stderr, "Null operator?\n");
		 } else if (!strcmp(op,"and")) {
		    // Process "and" operator       
		    
		    // Truth table for AND          
		    //  equ =  a,   b,   c    f     
		    //  --------------------|---    
		    //   0     0    0    0  | 1     
		    //   0     0    0    1  | 1     
		    //   0     0    1    0  | 1     
		    //   0     0    1    1  | 1     
		    //   0     1    0    0  | 1     
		    //   0     1    0    1  | 1     
		    //   0     1    1    0  | 1     
		    //   0     1    1    1  | 0     
		    //   1     0    0    0  | 0     
		    //   1     0    0    1  | 0     
		    //   1     0    1    0  | 0     
		    //   1     0    1    1  | 0     
		    //   1     1    0    0  | 0     
		    //   1     1    0    1  | 0     
		    //   1     1    1    0  | 0     
		    //   1     1    1    1  | 1     
		  
		    int i = 0;
		  
		    BDDNode *vars = true_ptr;
		    for (i = 1 ; i < vcount ; i++) {
		       int vthis = uns_list[i];
		       vars = ite_and( ite_var( vthis ), vars);
		    }
		  
			 functionType[nmbrFunctions] = AND;

		    equalityVble[nmbrFunctions] = equiv;
		    independantVars[equiv] = 0;
		    if (in_group) parameterGroup[nmbrFunctions] = group+group_cnt++;

		    functions[nmbrFunctions++] = ite_equ( vars, ite_var( equiv ));

		    ///////////////
		    //                     if(uflist[x]->tail != NULL){
		    //                        uflist[x]->tail->next = new flat;
		    //                        uflist[x]->tail = uflist[x]->tail->next;
		    //                     } else {
		    //                        uflist[x]->head = uflist[x]->tail = new flat;
		    //                     }
		    //                     uflist[x]->tail->next = NULL;                     
		    //                     uflist[x]->tail->op = AND;
		    //                     uflist[x]->tail->bdd = ite_equ( vars, ite_var( equiv ));
		    ///////////////
		 } else if (!strcmp(op,"nand")) {
		    int i = 0;
		  
		    BDDNode *vars = true_ptr;
		    for (i = 1 ; i < vcount ; i++) {
		       int vthis = uns_list[i];
		       vars = ite_and( ite_var( vthis ), vars);
		    }
		  
		    functionType[nmbrFunctions] = NAND;
		    equalityVble[nmbrFunctions] = equiv;
		    
		    independantVars[equiv] = 0;
		    if (in_group) parameterGroup[nmbrFunctions] = group+group_cnt++;

		    functions[nmbrFunctions++] = ite_equ( ite_not(vars), ite_var( equiv ));
		 } else if (!strcmp(op,"or")) {
		    // Process "or" operator        
		  
		    // Truth table for OR           
		    //  equ =  a,   b,   c    f     
		    //  --------------------|---    
		    //   0     0    0    0  | 1     
		    //   0     0    0    1  | 0     
		    //   0     0    1    0  | 0     
		    //   0     0    1    1  | 0     
		    //   0     1    0    0  | 0     
		    //   0     1    0    1  | 0     
		    //   0     1    1    0  | 0     
		    //   0     1    1    1  | 0     
		    //   1     0    0    0  | 0     
		    //   1     0    0    1  | 1     
		    //   1     0    1    0  | 1     
		    //   1     0    1    1  | 1     
		    //   1     1    0    0  | 1     
		    //   1     1    0    1  | 1     
		    //   1     1    1    0  | 1     
		    //   1     1    1    1  | 1     
		    int i = 0;
		  
		    BDDNode *vars = false_ptr;
		    for (i = 1 ; i < vcount ; i++) {
		       int vthis = uns_list[i];
		       vars = ite_or( ite_var( vthis ), vars);
		    }
			 functionType[nmbrFunctions] = OR;
				 
		    equalityVble[nmbrFunctions] = equiv;
		    independantVars[equiv] = 0;
		    if (in_group) parameterGroup[nmbrFunctions] = group+group_cnt++;

		    functions[nmbrFunctions++] = ite_equ( vars, ite_var( equiv ));
		 } else if (!strcmp(op,"nor")) {
		    int i = 0;
		  
		    BDDNode *vars = false_ptr;
		    for (i = 1 ; i < vcount ; i++) {
		       int vthis = uns_list[i];
		       vars = ite_or( ite_var( vthis ), vars);
		    }
		    
		    functionType[nmbrFunctions] = NOR;
		    equalityVble[nmbrFunctions] = equiv;

		    independantVars[equiv] = 0;
		    if (in_group) parameterGroup[nmbrFunctions] = group+group_cnt++;

		    functions[nmbrFunctions++] = ite_equ( ite_not(vars), ite_var( equiv ));
		 } else if (!strcmp(op,"xor")) {
		    int i = 0;
		  
		    BDDNode *vars = false_ptr;
		    for (i = 1 ; i < vcount ; i++) {
		       int vthis = uns_list[i];
		       vars = ite_xor( ite_var( vthis ), vars);
		    }
		  
		    functionType[nmbrFunctions] = XOR;
		    equalityVble[nmbrFunctions] = equiv;
		    
		    independantVars[equiv] = 0;
		    if (in_group) parameterGroup[nmbrFunctions] = group+group_cnt++;

		    functions[nmbrFunctions++] = ite_equ( vars, ite_var( equiv ));
		 } else if (!strcmp(op,"equ") || !strcmp(op, "xnor")) {
		    int i = 0;
		    
		    BDDNode *vars = false_ptr;
		    for (i = 1 ; i < vcount ; i++) {
		       int vthis = uns_list[i];
		       vars = ite_xor( ite_var( vthis ), vars);
		    }
		    
		    functionType[nmbrFunctions] = EQU;
		    equalityVble[nmbrFunctions] = equiv;
		    
		    independantVars[equiv] = 0;
		    if (in_group) parameterGroup[nmbrFunctions] = group+group_cnt++;

		    functions[nmbrFunctions++] = ite_equ( ite_not( vars ), ite_var( equiv ));
		 } else if (!strcmp(op,"limp")) {
		    int i = 0;
		    
		    BDDNode *vars = ite_var( uns_list[1] );
		    for (i = 2 ; i < vcount ; i++) {
		       int vthis = uns_list[i];
		       vars = ite_imp( vars, ite_var( vthis ));
		    }
		    
		    functionType[nmbrFunctions] = LIMP;
		    equalityVble[nmbrFunctions] = equiv;
		    
		    independantVars[equiv] = 0;
		    if (in_group) parameterGroup[nmbrFunctions] = group+group_cnt++;

		    functions[nmbrFunctions++] = ite_equ( vars, ite_var( equiv ));
		 } else if (!strcmp(op,"lnimp")) {
		    int i = 0;
		    
		    BDDNode *vars = ite_var( uns_list[1] );
		    for (i = 2 ; i < vcount ; i++) {
		       int vthis = uns_list[i];
		       vars = ite_imp( vars, ite_var( vthis ));
		    }
		    
		    functionType[nmbrFunctions] = LNIMP;
		    equalityVble[nmbrFunctions] = equiv;
		    
		    independantVars[equiv] = 0;
		    if (in_group) parameterGroup[nmbrFunctions] = group+group_cnt++;

		    functions[nmbrFunctions++] = ite_equ( ite_not( vars ), ite_var( equiv ));
		 } else if (!strcmp(op,"rimp")) {
		    int i = 0;
		    
		    BDDNode *vars = ite_var( uns_list[vcount-1] );
		    for (i = vcount-2 ; i > 0 ; i--) {
		       int vthis = uns_list[i];
		       vars = ite_imp( ite_var( vthis ), vars);
		    }
		    
		    functionType[nmbrFunctions] = RIMP;
		    equalityVble[nmbrFunctions] = equiv;
		    
		    independantVars[equiv] = 0;
		    if (in_group) parameterGroup[nmbrFunctions] = group+group_cnt++;

		    functions[nmbrFunctions++] = ite_equ( vars, ite_var( equiv ));
		 } else if (!strcmp(op,"rnimp")) {
		    int i = 0;
		    
		    BDDNode *vars = ite_var( uns_list[vcount-1] );
		    for (i = vcount-2 ; i > 0 ; i--) {
		       int vthis = uns_list[i];
		       vars = ite_imp( ite_var( vthis ), vars);
		    }
		    
		    functionType[nmbrFunctions] = RNIMP;
		    equalityVble[nmbrFunctions] = equiv;
		    
		    independantVars[equiv] = 0;
		    if (in_group) parameterGroup[nmbrFunctions] = group+group_cnt++;

		    functions[nmbrFunctions++] = ite_equ( ite_not( vars ), ite_var( equiv ));
		 } else if (!strcmp(op,"ite")) {
		    // Process the if-then-else operator 
		    
		    // If Then Else :                                        
		    //   This operator involves exactly four variables:      
		    //   equiv = if ? then : else                            
		    //   var_list contains the variables in increasing order 
		    //   equ = index into var_list of equiv variable (first) 
		    //   ife = index into var_list of if variable (second)   
		    //   the = index into var_list of then variable (third)  
		    //   els = index into var_list of else variable (fourth) 
		    //                                                       
		    // Truth table for If-Then-Else 
		    //  equ = ife, the, els   f     
		    //  --------------------|---    
		    //   0     0    0    0  | 1     
		    //   0     0    0    1  | 0     
		    //   0     0    1    0  | 1     
		    //   0     0    1    1  | 0     
		    //   0     1    0    0  | 1     
		    //   0     1    0    1  | 1     
		    //   0     1    1    0  | 0     
		    //   0     1    1    1  | 0     
		    //   1     0    0    0  | 0     
		    //   1     0    0    1  | 1     
		    //   1     0    1    0  | 0     
		    //   1     0    1    1  | 1     
		    //   1     1    0    0  | 0     
		    //   1     1    0    1  | 0     
		    //   1     1    1    0  | 1     
		    //   1     1    1    1  | 1     
		    
		    // Output error message if number of literals != 4 
		    if (vcount != 4) {
		       fprintf(stderr, "ITE operator requires 4 vars\n");
		    } else {
		       int equ=-1, ife=-1, the=-1, els=-1;
		       // Map "equ" to index of var_list containing var 
		       // which is left of the "="                      
		       int va = ((Integer *)symbols->get(first))->intValue();
		       for (int i=0 ; i < vcount ; i++) {
			  if (var_list[i] == va) {
			     equ = i;
			     break;
			  }
		       }
		       mer = substring->get(s, indexOf(s,'(')+1,indexOf(s,')')+1);
		       t->renewTokenizer(mer, " ,)");
		       v = trim->get(t->nextToken());
		       // Map "ife" to index of var_list containing var 
		       // which is first in argument list               
		       va = ((Integer *)symbols->get(v))->intValue();
		       for (int i=0 ; i < vcount ; i++) {
			  if (var_list[i] == va) {
			     ife = i;
			     break;
			  }
		       }
		       v = trim->get(t->nextToken());
		       // Map "the" to index of var_list containing var 
		       // which is second in argument list              
		       va = ((Integer *)symbols->get(v))->intValue();
		       for (int i=0 ; i < vcount ; i++) {
			  if (var_list[i] == va) {
			     the = i;
			     break;
			  }
		       }
		       v = trim->get(t->nextToken());
		       // Map "els" to index of var_list containing var 
		       // which is third in argument list               
		       va = ((Integer *)symbols->get(v))->intValue();
		       for (int i=0 ; i < vcount ; i++) {
			  if (var_list[i] == va) {
			     els = i;
			     break;
			  }
		       }
		       BDDNode *var_equ = ite_var (var_list[equ]);
		       BDDNode *var_ife = ite_var (var_list[ife]);
		       BDDNode *var_the = ite_var (var_list[the]);
		       BDDNode *var_els = ite_var (var_list[els]);
		       BDDNode *var_ite = ite (var_ife, var_the, var_els);
		       
		       functionType[nmbrFunctions] = ITE;
		       equalityVble[nmbrFunctions] = var_list[equ];
		       independantVars[var_list[equ]] = 0;
		       if (in_group) parameterGroup[nmbrFunctions] = group+group_cnt++;

		       functions[nmbrFunctions++] = 
			  ite_equ (var_equ, var_ite);
		    }
		 } else if (!strcmp(op,"nite")) {
		    if (vcount != 4) {
		       fprintf(stderr, "NITE operator requires 4 vars\n");
		    } else {
		       int equ=-1, ife=-1, the=-1, els=-1;
		       // Map "equ" to index of var_list containing var 
		       // which is left of the "="                      
		       int va = ((Integer *)symbols->get(first))->intValue();
		       for (int i=0 ; i < vcount ; i++) {
			  if (var_list[i] == va) {
			     equ = i;
			     break;
			  }
		       }
		       mer = substring->get(s, indexOf(s,'(')+1,indexOf(s,')')+1);
		       t->renewTokenizer(mer, " ,)");
		       v = trim->get(t->nextToken());
		       // Map "ife" to index of var_list containing var 
		       // which is first in argument list               
		       va = ((Integer *)symbols->get(v))->intValue();
		       for (int i=0 ; i < vcount ; i++) {
			  if (var_list[i] == va) {
			     ife = i;
			     break;
			  }
		       }
		       v = trim->get(t->nextToken());
		       // Map "the" to index of var_list containing var 
		       // which is second in argument list              
		       va = ((Integer *)symbols->get(v))->intValue();
		       for (int i=0 ; i < vcount ; i++) {
			  if (var_list[i] == va) {
			     the = i;
			     break;
			  }
		       }
		       v = trim->get(t->nextToken());
		       // Map "els" to index of var_list containing var 
		       // which is third in argument list               
		       va = ((Integer *)symbols->get(v))->intValue();
		       for (int i=0 ; i < vcount ; i++) {
			  if (var_list[i] == va) {
			     els = i;
			     break;
			  }
		       }
		       BDDNode *var_equ = ite_var (var_list[equ]);
		       BDDNode *var_ife = ite_var (var_list[ife]);
		       BDDNode *var_the = ite_var (var_list[the]);
		       BDDNode *var_els = ite_var (var_list[els]);
		       BDDNode *var_ite = ite (var_ife, var_the, var_els);
		       
		       functionType[nmbrFunctions] = NITE;
		       equalityVble[nmbrFunctions] = var_list[equ];
		       
		       independantVars[var_list[equ]] = 0;
		       if (in_group) parameterGroup[nmbrFunctions] = group+group_cnt++;
		       
		       functions[nmbrFunctions++] =
			  ite_equ (var_equ, ite_not(var_ite));
		    }
		 } else { // Add the truth tabled module
		    // Format of compiled module file:                     
		    //  Several sections each beginning with "FUNCTION ..."
		    //  FUNCTION line has space separated parameters       
		    //  first one of which is the output parameter         
		    //  Our requested output (first param of call) must    
		    //  match one FUNCTION output parameter - first one it 
		    //  Remaining lines in section are to be substituted   
		    //-----------------------------------------------------
		    // Find the output variable (call it out)              
		    int begarg = indexOf(s, '(');
		    int endarg = indexOf(s, ')');
		    if (endarg < 0 || begarg < 0) {
		       fprintf(stderr,"Syntax error:%s parens not match\n",s);
		       exit (1);
		    }
					
		    t->renewTokenizer(substring->get(s,begarg+1,endarg),",) ");
		    out = trim->get(t->nextToken());
		    int cnt = t->countTokens();
		    /*** At some point set a maximum count for # inputs ***/
		    /*** Be sure to coordinate with the max byte count  ***/
		    /*** of fgets(line...                               ***/
		    // Find text to substitute - 
                    // check if the module file exists
		    sprintf(path,"%s/%s.tab",module_root,op);
		    if ((fc = fopen(path, "rb")) != NULL) {
		       // Look for the section containing the requested output 
		       while ((l = fgets(line, 4097, fc)) != NULL) {
			  if (strncmp(l,"FUNCTION",8)) continue;
			  if (!strncmp(&l[9],out,strlen(out))) {
			     // Check arg counts of call and module for match 
			     func->renewTokenizer(&l[9], " ");
			     func->nextToken();
			     if (func->countTokens() != cnt) {
				fprintf(stderr,"Module %s needs %d arguments for output %s.  You gave it %d.\n", path, func->countTokens(), out, cnt);
				exit (1);
			     }
			     // At this point we have found a section to 
                             // insert. Increment mod counter and make 
                             // arg substitution list.
			     for (int i=0 ; i < cnt ; i++) {
				strcpy(arg_lst[i], trim->get(t->nextToken()));
				strcpy(fun_lst[i], trim->get(func->nextToken()));
			     }
								
			     if ((l = fgets(line, 4096, fc)) == NULL) {
				fprintf(stderr,"Smurf module %s has no truth table line for %s\n", path, out);
				exit (1);
			     }

                             // Input the truth table from module
                             // map_lst: numbers of variables stored from the
                             //          hash table in the order they are
                             //          parsed and translated in the module
                             // srt_lst: same numbers as in map_lst except
                             //          will be placed in increasing order
                             for (int i=0 ; i < cnt ; i++) {
				int vap = ((Integer *)symbols->get(arg_lst[i]))->intValue();
				map_lst[i] = vap;
				srt_lst[i] = vap;
                             } 

			     // Sort srt_lst and produce the permutation
			     // from map_lst to it.  For example, if 
			     // map_lst={3,6,4} then lat_lst={0,2,1}
			     // which means the first element of map_list
			     // remains the first element, the second element
			     // of map_lst becomes the third element and the
			     // third element of map_lst becomes the second
			     // element.
			     qsort(srt_lst, cnt, sizeof(int), intcmp);
			     for (int i=0 ; i < cnt ; i++) {
				for (int j=0 ; j < cnt ; j++) {
				   if (srt_lst[j] == map_lst[i]) {
				      lat_lst[i] = j;
				      break;
				   }
				}
			     }

                             // Assemble the truth table for the increasing
                             // order permutation of the original module
                             // function.  The truth table is a sequence
                             // of '1' or '0' ascii characters in line_xlate
                             // array.  The corresponding variables are in
                             // srt_lst;
                             long incr = 0; 
                             for (int i=1 ; i <= (1 << cnt) ; i++) {
				line_xlate[i-1] = l[incr];
				for (int j=0 ; j < cnt ; j++) {
				   if ((i % (1 << lat_lst[j])) == 0) 
				      incr ^= (1 << j);
				}
                             } 
			     line_xlate[(1 << cnt)] = 0;
			     /************

			     cout << "Variables:";
			     for (int i=0 ; i < cnt ; i++) 
				cout << srt_lst[i] << " "; 
			     cout << "\nLine:" << line_xlate << "\n";

			     ************/
			    
			     int y = 0;
			     int level = 0;

			     functionType[nmbrFunctions] = UNSURE;
		             equalityVble[nmbrFunctions] = equiv;

                   independantVars[equiv] = 0;
                   if (in_group) parameterGroup[nmbrFunctions] = group+group_cnt++;
                   BDDNode *v = ReadSmurf(&y, line_xlate, level, srt_lst, cnt);
                   functions[nmbrFunctions++] = ite_equ(v, ite_var( equiv ));

                   goto done;
			  }
		       }
done:                  fclose(fc);
		    } else {
		       fprintf(stderr,"Module %s not in library\n", path);
		       exit (1);
		    }
		 } // construct BDD for each function
	      } // 'not' token
	   } // '=' token
	} // pass 3 while fgets
	delete [] var_list;
	delete [] uns_list;
	fclose(fd);
   } else { 
      fprintf(stderr, "File not found\n");
finished:
      delete r;
      delete t;
      delete func;
      delete substring;
      delete trim;
      return 1;
   }
   d2_printf1("\n");
   unlink(file);         
   delete r;
   delete t;
	delete func;
   delete substring;
   delete trim;
   return 0;
}

void Tracer::display () {  cout << file << endl;  }

int Tracer::intValue (char *v)
{
  Integer *obj = (Integer*)symbols->get(v);
  if (obj != NULL) 
  {
    int i = obj->intValue();
    return i;
  }
  /* nonexisting symbol */
  return 0;
}

void Tracer::getSymbols (int *vars, int size) {
   Object **obj = symbols->entrySet();
   for (int i=0 ; obj[i] != NULL ; i++) {
      int vv = ((Integer *)obj[i])->intValue();
      bool negative = (vv < 0) ? true : false;
      vv = abs(vv);
      if (vv < size) {
			char *ret = ((Integer *)obj[i])->getId();
			if (ret[0] != 1) {
				ShowResultLine(foutputfile, ret, vv, negative, vars[vv]);
			}	
      }
   }
	delete [] obj;
}

// For maintenance - must take Integer objects 
void Tracer::printHashTable (Hashtable *ht) {
   Object **obj = ht->entrySet();
   for (int i=0 ; obj[i] != NULL ; i++) {
      int vv = ((Integer *)obj[i])->intValue();
      bool negative = (vv < 0) ? true : false;
      vv = abs(vv);
      cout << ((Integer *)obj[i])->getId() << " (";
      if (negative) cout << "-" << (vv-1) << ")\n";
      else cout << (vv-1) << ")\n";
   }
}
