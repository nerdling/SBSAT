/*********************************************************************
 *  flatten.cc (J. Franco)
 *  Functions normalizeInputTrace and insertModules flatten a 
 *  collection of nested expressions in Trace format and insert
 *  libraries of Trace expressions.  All others support these two.
 *********************************************************************/

#include <ite.h>
#include "formats.h"

char *builtin_ops[] = { "and", "nand", "or", "nor", "xor", "xnor", "equ",
                        "rimp", "rnimp", "limp", "lnimp", "ite", "nite",
                        "not", "are_equal", "new_int_leaf", NULL };

FlattenTrace::FlattenTrace (char *filename) {
   file = new char[strlen(filename)+1];
   strcpy(file, filename);
   flattened_file = new char[strlen(file)+6];
   sprintf(flattened_file,"%s.flt",file);
   macroed_file = new char[strlen(file)+6];
   sprintf(macroed_file,"%s.mac",file);
}
   
FlattenTrace::FlattenTrace (char *filename, int sw) {
   file = new char[strlen(filename)+1];
   strcpy(file, filename);
   flattened_file = new char[strlen(file)+6];
   sprintf(flattened_file,"%s.flt",file);
   macroed_file = new char[strlen(file)+6];
   sprintf(macroed_file,"%s.mac",file);
   if (sw == 1) hash_table = true; else hash_table = false;
}

FlattenTrace::~FlattenTrace() {
   delete [] file;
   delete [] flattened_file;
   delete [] macroed_file;
}
   int flatten_lines=0;
   
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
char *FlattenTrace::breakCompound (char *s, FILE *ft, char **u, int id) {
   StringTokenizer 
	  *t = NULL;
   Trimmer
	  *trim = new Trimmer();
   bool use_temp_var = true,   // false if arg is "a.. = op..(..)" or at top
	  recurse = false;       // becomes true if breakCompound recurses    
   static int cnt = 0;         // for uniqueness of temp vars               
   char *v,
	  o[B_SIZE],             // Space to build current function line     
	  *o_ptr = o,            // Points to next available byte in o[]     
	  *var,                  // Name of temp var - sent up a level       
	  *p=s,                  // Points to beginning of current argument  
	  *end;                  // End of argument                          
   
   if (u == NULL) use_temp_var = false;
   
   var = new char[1024];
   t = new StringTokenizer(s," =,()");
   sprintf(var,"%s",trim->get(t->nextToken()));
   
   // Find first "(", if "=" is seen first then temp var will not be needed
   for ( ; *p != '(' && *p != 0 ; p++) {
      if (*p == '=') use_temp_var = false;
   }
   p++;                        // p points to the beginning of the argument
   memcpy(o_ptr, s, p-s);      // Copy line "s" up to "(" to "o[]"         
   o_ptr += p-s;               // Set o_ptr to first unfilled byte of o[]  
   while (1) {   // Check each argument to see whether it should be parsed 
      for ( ; *p == ' ' || *p == '\t' ; p++);  // cursor to beginning of arg
      char *t=p; // Set t to point to beginning of the argument       
      // Find its end
      for ( ; *t != ',' && *t != '(' && *t != ')' && *t != 0 ; t++);        
      if (*t == '(') {      // If "(" is seen, parse the argument   
         if (u == NULL && !recurse) { fputs("%%begin block\n", ft); 
				flatten_lines++; }
         recurse = true;
         v = breakCompound(p, ft, &end, id);
         memcpy(o_ptr, v, strlen(v)); // Use returned "v" for arg here     
         o_ptr += strlen(v);
         delete v;
         p = end;
         if (*p == ',') {             // If it's a middle argument ...     
            *o_ptr++ = ',';           // add "," and a " "                 
            *o_ptr++ = ' ';
            for (p++; *p==' ' ;p++);  // advance p to start of next arg ...
         } else {                     // If it's the last argument ...     
            *o_ptr++ = ')';           // add ")"...                        
            *o_ptr++ = 0;             // terminate the string ...          
            if (*p == 0) break;       // At end of s so break              
            for (p++; *p==' ' ;p++);  // Advance p to next "," or ")" ...  
            break;                    // and break                         
         }
      } else {                 // Otherwise, no nesting in that argument   
         memcpy(o_ptr, p, t+1-p);     // Hopefully includes the "," or ")" 
         o_ptr += t+1-p;
         if (*t == 0) {               // Reached end without seeing delimiter 
            fprintf(stderr,"Bad line: %s\n", s);
            exit (1);
         } else if (*t == ')') {      // If we hit ')' then the whole arg list
            *o_ptr++ = 0;
            for (p=t+1;*p==' '||*p=='\t';p++);  // Advance p to ",", ")" or "0"
            break;             // is parsed so break                       
         } else {
            *o_ptr++ = ' ';
            for (p=t+1;*p==' ';p++);  // Advance p to start of next arg    
         }
      }
   }
   // Write current line to file 
   if (use_temp_var) {
      sprintf(var,"%c%d_%d",1,id,cnt++);  // Make a temp var, if needed
      fputs(var,ft);   // Build 1st token of line "var = op(...)"
      fputs(" = ",ft); // Add "=" token.
   }
   fputs(trim->get(o), ft); // Add rest of the line
   fputs(";\n",ft); flatten_lines++;
   if (u == NULL && recurse) {    // If at top level, say block is ending
      fputs("%%end block\n", ft); flatten_lines++;
   } else if (u != NULL) {
      *u = p;
   }
   delete trim;
   delete t;
   return var;         // Send the name of the new var up
}

void print_vars_in_line (char *s, FILE *ft) {
   char *ptr = s, *t;
   while (1) {
      // Advance to non-white-space or semi-colon
      for ( ; (*ptr==' ' || *ptr=='\t') && *ptr != ';' && *ptr != 0 ; ptr++);
      if (*ptr == ';') break;
      if (*ptr == '=') { ptr++; continue; }
      // Let ptr point to beginning of token and advance t until delimiter
      for (t=ptr ; 
	   *t != ' ' && *t != '\t' && *t != 0 && 
	   *t != ';' && *t != '(' && *t != ')' && *t != ',' ;
	   t++);
      // Skip operators and end of parameter lists
      if (*t == '(') { ptr = t+1; continue; }
      if (*t == ';' || *t == 0) {
	 *t = 0;
	 fputs(ptr, ft);
	 fputs(" ",ft); flatten_lines++;
	 break;
      }
      // Must have hit a comma after a paren
      if (t == ptr) { ptr++; continue; }
      // Now have some token, looking to see if it is an operator or variable
      *t = 0;
      for (t++ ; (*t == ' ' || *t == '\t') && *t != ';' && *t != 0 ; t++);
      if (*t == ';' || *t == 0) {
	 fputs(ptr, ft);
	 fputs(" ",ft); flatten_lines++;
	 break;
      }
      if (*t == '(') { ptr = t; continue; }
      fputs(ptr, ft);
      fputs(" ",ft); flatten_lines++;
      ptr = t;
   }
   fputs("\n",ft); flatten_lines++;
}

int FlattenTrace::normalizeInputTrace () {   // Returns neg for error, 0 for OK
   StringTokenizer *t = new StringTokenizer("","");
   Trimmer *trim = new Trimmer();
   char sb[B_SIZE], *s = NULL, *current = NULL, *token = NULL;
   char comment = '%';
   long mark, t_mark;
   int state=0;
   int id=0;
   int cp;
   bool group_on = false, group_bdd_done = false;
   int lineno=0;
   FILE *fd, *ft;
   if ((fd = fopen(file, "rb")) != NULL) {
      if ((ft = fopen(flattened_file, "wb")) != NULL) {
         while (1) {
            mark = 0;
            //Collect successive lines into single buffer  
            //Strip comments here (comment character is %) 
            if (feof(fd)) goto err;
            while ((fgets(&sb[mark], B_SIZE-mark-2, fd)) != NULL) {
	       if (++lineno %100 == 0) 
		{
		  d2_printf3("\rReading tracer 0/3: %d/%d", lineno, flatten_lines);
                }
               cp = indexOf(&sb[mark], comment);
               if (cp >= 0) { sb[mark+cp] = '\n'; sb[mark+cp+1] = 0; }
               if (state == 0) break;
               t_mark = mark;
               mark += strlen(&sb[mark]);
               if (mark >= B_SIZE-2) {
                  fprintf(stderr,"Buffer overrun\n");
                  exit(1);
               }
               if (mark == 0) goto err;
               if (sb[mark-1] == '\n') sb[mark-1] = ' ';
               sb[mark] = 0;
               if (indexFrom(&sb[t_mark], &sb[mark], ';') != -1) break;
            }
            // Find first token of buffer
	    current = sb; 

            t->renewTokenizer(current, " ");
continue_parse:
            token = trim->get(t->nextToken());
            if (token == NULL) continue;
            switch (state) {
            case 0:  // Startup - MODULE section - Assume: "MODULE name"
               if (strcmp(token, "MODULE") || !t->hasMoreTokens()) {
                  fprintf(stderr,"Unexpected first line: want: MODULE <module_name>, got: %s\n", s);
                  unlink(flattened_file); 
		  fclose(ft); 
		  fclose(fd);
                  exit (1);
               }
               fputs(current, ft); flatten_lines++;
               state = 1;
               break;
            case 1:  // Looking for input variables - INPUT section 
               // All inputs are on same line which may be    
               // shared with the keyword "INPUT".            
               if (strcmp(token, "INPUT")) {
                  fprintf(stderr,"Unexpected second line: want: INPUT [inp_var ...], got: %s\n", s);
                  unlink(flattened_file); 
		  fclose(fd); 
		  fclose(ft);
                  exit (1);
               }
               fputs(current, ft); flatten_lines++;
               fputs("\n",ft);
               state = 2;
               break;
            case 2:  // Still in the input state looking for inputs
               if (strcmp(token,"OUTPUT")) {
                  fputs(current, ft); flatten_lines++;
                  break;
               }
               fputs(current, ft); flatten_lines++;
               fputs("\n", ft);
               state = 3;
               break;
            case 3:  // Still in output vars state looking for vars  
               // Go to STRUCTURE section if found - Assume    
               // the keyword is the only thing on the line.   
               if (strcmp(token,"STRUCTURE")) {
                  fputs(current, ft); flatten_lines++;
                  break;
               } else {
                  fputs("STRUCTURE\n", ft); flatten_lines++;
                  state = 4;
		  if (t->hasMoreTokens()) {
		     t->renewTokenizer(current = &sb[9], " ");
		     goto continue_parse;
		  }
                  break;
               }
            case 4:  // In STRUCTURE section parsing compund expressions
               if (strcmp(token,"ENDMODULE")) {
		  if (!strncmp(token,"&&begingroup",12)) {
		     if (t->hasMoreTokens()) {
			fprintf(stderr,"Semicolon missing after &&begingroup\n");
			exit(1);
		     }
		     if (group_on) {
			fprintf(stderr,"Overlapping groups\n");
			exit(1);
		     }
		     group_on = true;
		     group_bdd_done = false;
		     fputs("&&begingroup;\n",ft); flatten_lines++;
		     break;
		  } else if (!strncmp(token,"&&endgroup",10)) {
		     if (t->hasMoreTokens()) {
			fprintf(stderr,"Semicolon missing after &&endgroup\n");
			exit(1);
		     }
		     if (!group_on) {
			fprintf(stderr,"No group to terminate\n");
			exit(1);
		     }
		     group_on = false;
		     fputs("&&endgroup;\n", ft); flatten_lines++;
		     break;
		  } else {
		     if (!group_on || !group_bdd_done) {
				  char *v = breakCompound(current, ft, NULL, id++);
				  delete [] v;
			if (group_on && !group_bdd_done) {
			   group_bdd_done = true;
			   print_vars_in_line(current, ft); 
			}
		     } else {
			print_vars_in_line(current, ft); 
		     }
		  }
		  break;
	       }
	       fputs(current, ft); flatten_lines++;
	       fputs("\n", ft);
	       goto out;
	    }
	 }
err:     fprintf(stderr,"File ended prematurely, ");
	 switch (state) {
          case 0: fprintf(stderr,"missing MODULE line\n"); break;
          case 1: fprintf(stderr,"missing INPUTS section\n"); break;
          case 2: fprintf(stderr,"missing OUTPUTS section\n"); break;
          case 3: fprintf(stderr,"missing STRUCTURE section\n"); break;
          case 4: fprintf(stderr,"missing ENDMODULE line\n"); break;
         }
         exit (1);
out:     fclose(ft);
      } else {
         fprintf(stderr,"Cannot write to %s\n", flattened_file);
         fclose(fd);
         exit (1);
      }
      fclose(fd);
   } else {
      fprintf(stderr,"Cannot open %s\n", file);
      exit (1);
   }
   delete trim;
   delete t;
   return 0;
}

bool FlattenTrace::builtin (char *op) {
   for (int i=0 ; builtin_ops[i] != NULL ; i++) {
      if (!strcmp(builtin_ops[i], op)) return true;
   }
   return false;
}

// Goes through the STRUCTURE section of a flattened _ircuit, line by 
// line, looking for functions which are not builtin.  Upon finding   
// such, makes an appropriate substitution from a file in the module  
// data base or reports no such module exists.  The substitution is   
// itself a flattened _ircuit - thus, module information is stored as 
// editable ascii text and in a separate file in "compiled" form which
// includes sections showing lines supporting each output of the      
// module.                                                            
char *FlattenTrace::insertModules () {
   StringTokenizer
      *func = new StringTokenizer("",""),
      *args = new StringTokenizer("",""),
      *t = new StringTokenizer("","");   // Tokenizer for given string  
   Substring
      *substring  = new Substring(),
      *substring1 = new Substring();
   Trimmer
      *trim  = new Trimmer(),
      *trim1 = new Trimmer();
   
   char sb[B_SIZE],        // Buffer for reading from an actual file    
      path[512],           // Path to a module                          
      line[1028],          // Buffer for reading a module line          
      mod_var[1028],       // Buffer for constructing unique module var 
      *s,                  // Line of an actual file                    
      *l,                  // Line of a compiled module file            
      token[1024],         // Picks up tokens parsed from a line        
      first[1024],         // First token of an actual line             
      *out = NULL,
      *nxt = NULL,
      op[1024],
      arg_lst[100][100],   // For translating actual parms to formal    
      fun_lst[100][100],   // For translating actual parms to formal    
      outline[B_SIZE];     // Buffer for writing module lines to file   
   bool have_equal = false, // Whether the current actual line has an =  
      in_block = false,     // true iff we are currently in a block      
      in_group = false,
      group_bdd_done = false;
   int state = 0;           // 0 = start, 1 = in STRUCTURE section       
   int linecnt = 0,         // Line number in actual list                
      funcnt = 0;           // Module count for uniqueness of int. vars  
   
   FILE *ft, *fm, *fc;
   if ((ft = fopen(flattened_file, "rb")) != NULL) {
      if ((fm = fopen(macroed_file, "wb")) != NULL) {
         while ((s = fgets(sb, B_SIZE-2, ft)) != NULL) {
            t->renewTokenizer(s, " ");
            strcpy(first,trim->get(t->nextToken()));
	    if (!strncmp(first, "&&begingroup",12)) {
               fputs(s,fm);
	       
	       in_block = false;
	       in_group = true;
	       group_bdd_done = false;
	       continue;
	    } else if (!strncmp(first,"&&endgroup",10)) {
               fputs(s,fm);
	       
	       in_block = false;
	       in_group = false;
	       if (!group_bdd_done) {
		  fprintf(stderr,"Group BDD not built, cannot continue\n");
		  exit(1);
	       }
	       continue;
	    } else if (!strncmp(first,"%%begin",7)) {
               fputs(s,fm);
      	       
               in_block = true;
               continue;
            } else if (!strncmp(first,"%%end",5)) {
               fputs(s,fm);
	       
               in_block = false;
	       if (in_group) group_bdd_done = true;
               continue;
            }
            linecnt++;
            if (!strncmp(first,"ENDMODULE",9)) {
	       fputs(s,fm);
	       
               break; 
            }

	    if (in_group && group_bdd_done) {
	       fputs(s,fm);
	       
	       continue;
	    }
            // Assume "STRUCTURE" keyword is on a line of its own 
            if (!strncmp(first,"STRUCTURE",9)) {
               fputs(s,fm);
	       
               state = 1;
               continue;
            }
            if (state == 0) {
               fputs(s,fm);
	       
               continue;
            }

            // Below: considering lines from the STRUCTURE section
            // First: find the operator
            strcpy(token,trim->get(t->nextToken()));
            if (!strcmp(token, "=")) {
               strcpy(op,trim->get(substring->get(s, indexOf(s, '=')+1, indexOf(s, '('))));
               have_equal = true;
            } else {
               strcpy(op,trim->get(substring->get(s, 0, indexOf(s, '('))));
               have_equal = false;
            }
            
            // Check if op is builtin - if not, subst. module for the line
            if (!builtin(op)) {
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
                  fprintf(stderr, "Syntax error:%s @ line %d: parens do not match\n", flattened_file, linecnt);
                  exit (1);
               }
					
               args->renewTokenizer(substring->get(s, begarg+1, endarg),",) ");
               out = trim->get(args->nextToken());
               int cnt = args->countTokens();
               // Find text to substitute - check if the module file exists
               sprintf(path,"%s/%s.moc",module_root,op);
               if ((fc = fopen(path, "rb")) != NULL) {
                  // Look for the section containing the requested output 
                  while ((l = fgets(line, 1026, fc)) != NULL) {
                     if (strncmp(l,"FUNCTION",8)) continue;
                     if (!strncmp(&l[9],out,strlen(out))) {
                        // Check arg counts of call and module for match 
                        func->renewTokenizer(&l[9], " ");
                        func->nextToken();
                        if (func->countTokens() != cnt) {
                           fprintf(stderr,"Module %s needs %d arguments for output %s.  You gave it %d.\n", path, func->countTokens(), out, cnt);
                           exit (1);
                        }
                        // At this point we have found a section to insert 
                        // Increment mod counter and make arg subst. list  
                        funcnt++;
                        for (int i=0 ; i < cnt ; i++) {
                           strcpy(arg_lst[i], trim->get(args->nextToken()));
                           strcpy(fun_lst[i], trim->get(func->nextToken()));
                        }
								
                        if (!in_block) fputs("%%begin block\n", fm);
                        // Substitute the section for the line             
                        // Assume module inputs never show up on left side 
                        // of an = in the module                           
                        while ((l = fgets(line, 1026, fc)) != NULL) {
                           if (l[0] == 0 || !strncmp(l,"FUNCTION",8)) break;
			   if (substring->get(l, indexOf(l, '(')+1, indexOf(l, ')')) == NULL) break;
                           func->renewTokenizer(substring->get(l, indexOf(l, '(')+1, indexOf(l, ')')), " ,)");
                           if (l[0] == 1) {          // not output line     
                              if (indexOf(l, '=') < 0) {  // must have an = 
                                 fprintf(stderr,"Module %s: must have = in \"%s\"\n", path, l);
                              }
                              sprintf(mod_var,"%c%s_%d = %s",1,trim->get(substring->get(l, 1, indexOf(l, '='))),funcnt,trim1->get(substring1->get(l, indexOf(l, '=')+1, indexOf(l, '('))));
                              strcat(outline,mod_var);
                           } else if (!have_equal) { // output line has no =
                              strcpy(outline, trim->get(substring->get(l, indexOf(l, '=')+1, indexOf(l, '('))));
                           } else {                  // output line has =
                              sprintf(outline,"%s = %s",first,trim->get(substring->get(l, indexOf(l, '=')+1, indexOf(l, '('))));
                           }
                           strcat(outline,"(");
                           bool first_time = true;
                           // Make variable substitutions 
                           while ((nxt = trim->get(func->nextToken())) != NULL) {
                              if (first_time) first_time = false;
                              else strcat(outline,", ");
                              int j;
                              // If the variable is in the input list 
                              for (j=0 ; j < cnt ; j++) {
                                 if (!strcmp(fun_lst[j], nxt)) {
                                    strcat(outline, arg_lst[j]);
                                    break;
                                 }
                              }
                              // If the variable is internal to the module 
                              if (j == cnt) {
                                 sprintf(mod_var,"%c%s_%d",1,trim->get(&nxt[1]),funcnt);
                                 strcat(outline,mod_var);
                              }
                           }
                           strcat(outline,");\n");
                           fputs(outline,fm);
			   
                        }
                        goto done;
                     }
                  }
						
                  if (l == NULL) {
                     fprintf(stderr,"Module %s has no output %s\n", path, out);
                     exit (1);
                  }
               done:                  fclose(fc);
                  if (!in_block) fputs ("%%end block\n", fm);
               } else {
		  //fprintf(stderr,"Assume %s in Smurf form\n", path);
		  fputs(s, fm);
                  // fprintf(stderr,"Module %s does not exist\n", path);
                  // exit (1);
               }
            } else {  // if so, the line is untouched
               fputs(s,fm);
	       
            }
	    if (in_group && !in_block && !group_bdd_done) 
	       group_bdd_done = true;
         }
         fclose(fm);
      } else {
         fprintf(stderr,"Cannot open %s in current directory\n", macroed_file);
         unlink(flattened_file);
         fclose(ft);
         delete t;
         delete args;
         delete func;
         delete substring;
         delete substring1;
         delete trim;
         delete trim1;
         return NULL;
      }
      fclose(ft);
      unlink(flattened_file);
   } else {
      fprintf(stderr,"Cannot open %s in current directory\n", flattened_file);
      delete t;
      delete args;
      delete func;
      delete substring;
      delete substring1;
      delete trim;
      delete trim1;
      return NULL;
   }
   delete t;
   delete args;
   delete func;
   delete substring;
   delete substring1;
   delete trim;
   delete trim1;
   return NULL;
}

