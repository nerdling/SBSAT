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
 *  xorbdd.c (S. Weaver)
 *  BDD tools
 *********************************************************/  

#include "ite.h"
#include "formats.h"

char getNextSymbol (char *&, int &intnum, BDDNode * &);

int xorbdd_line;

int intnum;
extern int no_independent;

char getNextSymbol () {
	char integers[20];
	int i = 0;
	int p = 0;
	while (1) {
      p = fgetc(finputfile);
		if (p == EOF) {
			//fprintf(stderr, "\nUnexpected (unsigned int)EOF...exiting\n");
			return 'x';
		}
		if (feof(finputfile)) return 'x';
		if (p == ';') {
			while (p != '\n') {
            p = fgetc(finputfile);
            if (p == EOF) {
					return 'x';
					//fprintf (stderr, "\nUnexpected (unsigned int)EOF...exiting:%d\n", xorbdd_line);
					//exit (0);
				}
			}
         p = fgetc(finputfile);
         if (p == EOF) {
				return 'x';	  
				//fprintf (stderr, "\nUnexpected (unsigned int)EOF...exiting:%d\n", xorbdd_line);
				//exit (0);
			}
			ungetc (p, finputfile);
			continue;
		}
		if (p == '=') {
         p = fgetc(finputfile);
			if (p == EOF) {
				fprintf (stderr, "\nUnexpected (unsigned int)EOF...exiting:%d\n", xorbdd_line);
				exit (0);
			}
			if (p != ' ') {
				fprintf (stderr, "\n' ' expected...exiting:%d\n", xorbdd_line);
				exit (0);
			}
         p = fgetc(finputfile);
			if (p == EOF) {
				fprintf (stderr, "\nUnexpected (unsigned int)EOF...exiting:%d\n", xorbdd_line);
				exit (0);
			}
			char ret = p;
			if ((p != '0') && (p != '1')) {
				fprintf (stderr, "\n0 or 1 expected...exiting:%d\n", xorbdd_line);
				exit (0);
			}
			
			while (p != '\n' && p != ';') {
            p = fgetc(finputfile);
            if (p == EOF) {
					ungetc (p, finputfile);
					return ret;
				}
				if(p!=' ' && p!='\n' && p!=';') {
					fprintf(stderr, "\nTrailing character (%c) found...exiting:%d\n", p, xorbdd_line);
					exit(0);
				}
			}
			
			ungetc (p, finputfile);
			return ret;
		}
		if (p == ' ') {
			return ' ';
		}
		if (p == 'x') {
         p = fgetc(finputfile);
         if (p == EOF) {
				fprintf (stderr, "\nUnexpected (unsigned int)EOF...exiting:%d\n", xorbdd_line);
				exit (0);
			}
			if ((p >= '1') && (p <= '9')) {
				i = 0;
				while ((p >= '0') && (p <= '9')) {
					integers[i] = p;
					i++;
               p = fgetc(finputfile);
               if (p == EOF) {
						fprintf (stderr, "\nUnexpected (unsigned int)EOF...exiting:%d\n", xorbdd_line);
						exit (0);
					}
				}
				ungetc (p, finputfile);
				integers[i] = 0;
				intnum = atoi (integers);
				if (intnum > numinp) {
					fprintf (stderr, "\nVariable %d is larger than allowed (%ld)...exiting:%d\n",	intnum, numinp - 2, xorbdd_line);
					exit (0);
				}
				return 'i';
			} else {
				fprintf (stderr, "\nInteger expected after x...exiting:%d\n", xorbdd_line);
				exit (0);
			}
		} if(p == '-') {
			fprintf (stderr, "\nNegative variables not supported...exiting:%d\n", xorbdd_line);
			exit (0);
		}
		if (p == 'i' || p == 'I'){
			int i = 0;
			char macros[14];
			while ((((p >= 'a') && (p <= 'z')) || ((p >= 'A') && (p <= 'Z')))
					 && (i < 13)) {
				macros[i] = p;
				i++;
            p = fgetc(finputfile);
            if (p == EOF) {
					fprintf (stderr, "\nUnexpected (unsigned int)EOF (%s)...exiting:%d\n", macros, xorbdd_line);
					exit (1);
				}
			}
			macros[i] = 0;
			if(strcasecmp (macros, "initialbranch")) {
				fprintf (stderr, "\nUnknown keyword (%s) found while looking for initialbranch...exiting:%d\n", macros, xorbdd_line);
				exit (1);
			}
			no_independent = 0;
			int p = 0;
			char integers[10];
			int secondnum = 0;
			i = 0;
			int openbracket_found = 0;
			int stop_openbracket = 0;
			Initialbranch:;
			while ((p != '\n')	&& !((p == ')') && (openbracket_found))) {
            p = fgetc(finputfile);
				if (p == EOF)	{
					fprintf (stderr, " %d %d ", openbracket_found, stop_openbracket);
					fprintf (stderr, "\nUnexpected (unsigned int)EOF (%s)...exiting:%d\n",	macros, xorbdd_line);
					exit (1);
				}
				if (p == '(') {
					if (!stop_openbracket)
					  openbracket_found = 1;
					continue;
				}
				if ((p >= '0') && (p <= '9')) {
					i = 0;
					while ((p >= '0') && (p <= '9')) {
						integers[i] = p;
						i++;
                  p = fgetc(finputfile);
                  if (p == EOF) {
							fprintf (stderr, "\nUnexpected (unsigned int)EOF (%s)...exiting:%d\n", macros, xorbdd_line);
							exit (1);
						}
					}
					integers[i] = 0;
					intnum = atoi (integers);
					if (intnum > numinp) {
						fprintf (stderr, "\nVariable %d is larger than allowed(%ld)...exiting:%d\n", intnum, numinp - 2, xorbdd_line);
						exit (1);
					}
					if (p == '.') {
                  p = fgetc(finputfile);
						if (p == EOF) {
							fprintf (stderr, "\nUnexpected (unsigned int)EOF (%s)...exiting:%d\n",	macros, xorbdd_line);
							exit (1);
						}
						if (p == '.') {
                     p = fgetc(finputfile);
                     if (p == EOF) {
								fprintf (stderr, "\nUnexpected (unsigned int)EOF (%s)...exiting:%d\n", macros, xorbdd_line);
								exit (1);
							}
							if ((p >= '0') && (p <= '9')) {
								i = 0;
								while ((p >= '0') && (p <= '9')) {
									integers[i] = p;
									i++;
                           p = fgetc(finputfile);
                           if (p == EOF) {
										fprintf (stderr, "\nUnexpected (unsigned int)EOF (%s)...exiting:%d\n",	macros, xorbdd_line);
										exit (1);
									}
								}
								ungetc (p, finputfile);
								integers[i] = 0;
								secondnum = atoi (integers);
								if (secondnum > numinp) {
									fprintf (stderr, "\nVariable %d is larger than allowed (%ld)...exiting:%d\n",	secondnum, numinp - 2, xorbdd_line);
									exit (1);
								}
								for (int x = intnum; x <= secondnum; x++)
								  independantVars[x] = 1;
								stop_openbracket = 1;
							} else {
								fprintf (stderr, "\nNumber expected after '%d..'  exiting:%d\n", intnum, xorbdd_line);
								exit (1);
							}
						} else {
							ungetc (p, finputfile);
							stop_openbracket = 1;
							independantVars[intnum] = 1;
						}
					} else {
						ungetc (p, finputfile);
						stop_openbracket = 1;
						independantVars[intnum] = 1;
					}
				}
			}
			if ((p == '\n') && (openbracket_found == 1)) {
				p = ' ';
				goto Initialbranch;
			}
			ungetc (p, finputfile);
			return 'b';
		}
		if(p!='\n') {
			fprintf(stderr, "\nUnexpected character (%c)...exiting:%d\n", p, xorbdd_line);
			exit(0);
		}
	}
}

void xorloop () {
	fscanf (finputfile, "%ld %ld\n", &numinp, &numout);
	numinp += 2;
	xorbdd_line = 1;
	no_independent = 1;

	vars_alloc((numinp*4)+2);
	functions_alloc((numout*4)+2);

	int temp_vars = 1;
	
	//int *keep = new int[numout + 2];
	BDDNode **bdds = new BDDNode*[1000];
	int p = 0;

	for (int x = 0; x < numinp + 1; x++) {
		independantVars[x] = 0;
	}
	
	while (1) {				//(p = fgetc(finputfile))!=(unsigned int)EOF) 
		xorbdd_line++;
      d2_printf3("\rReading XOR %d/%ld", xorbdd_line, (long)numout);
		if(p == ';') {
			while (p != '\n') {
            p = fgetc(finputfile);
            if (p == EOF) {
					goto Exit;
				}
			}
         p = fgetc(finputfile);
         if(p == EOF) {
				goto Exit;
			}
			if(p!=';') {
				ungetc(p, finputfile);
			}
			continue;
		}
		if(p=='\n') {
			while (p != '\n') {
            p = fgetc(finputfile);
            if (p == EOF) {
					goto Exit;
				}
			}
			if(p==';') continue;
			ungetc(p, finputfile);
		}
      p = fgetc(finputfile);
      if(p == EOF) {
			goto Exit;
		}
		ungetc(p, finputfile);
		p = getNextSymbol();
		if(p == 'x') goto Exit;
		if(p == 'b') continue; //InitialBranch condition
		else if(p != 'i') {
			fprintf (stderr, "\nx# expected...exiting:%d\n", xorbdd_line);
			exit (0);	
		}
		
		int nmbrxors = 0;
		int num_spaces = 0;
		int *vars = (int *)calloc(numinp, sizeof(int));
		while((p != '0') && (p != '1')) {
			//Get it!
			bdds[nmbrxors] = ite_var(intnum);
			int num_ands = 0;
			while(p == 'i') {
				bdds[nmbrxors] = ite_and(bdds[nmbrxors], ite_var(intnum));
				int intnum_prev = intnum;
				p = getNextSymbol();
				if(p == 'i') { vars[intnum] = 1; vars[intnum_prev] = 1;}
				if(p == 'x') {
					fprintf (stderr, "\nUnexpected (unsigned int)EOF, '=' expected...exiting:%d\n", xorbdd_line);
					exit (0);	
				}
				num_spaces = 0;
				num_ands++;
			}
			if(p!=' ') {
				fprintf (stderr, "\n' ' expected...exiting:%d\n", xorbdd_line);
				exit (0);	
			}
			num_spaces++;
			if(num_spaces > 1) {
				fprintf(stderr, "\nOnly one space between characters is allowed...exiting:%d\n", xorbdd_line);
				exit(0);
			}
			nmbrxors++;
			p = getNextSymbol();
			if(p == 'x') {
				fprintf (stderr, "\nUnexpected (unsigned int)EOF, '=' expected...exiting:%d\n", xorbdd_line);
				exit (0);
			}
		}
	
		/*BDDNode *linear = false_ptr;*/
		BDDNode *excess = false_ptr;
		for(int i = 0; i < nmbrxors; i++) {
			  excess = ite_xor(excess, bdds[i]);
		}
		
		free(vars);

      if(p=='0')			  
         excess = ite_equ(excess, false_ptr);
      else if(p=='1')
         excess = ite_equ(excess, true_ptr);
      else {
         fprintf (stderr, "\n0 or 1 expected...exiting:%d\n", xorbdd_line);
         exit (0);	
      }
      functionType[nmbrFunctions] = UNSURE;
      functions[nmbrFunctions] = excess;
      d4_printf2("BDD $%d: ", nmbrFunctions);
      D_4(printBDDfile(functions[nmbrFunctions], stddbg);)
         d4_printf1("\n");
      nmbrFunctions++;
		
		if (nmbrFunctions > (numout * 2)) {
			fprintf (stderr, "Too many functions, increase to larger than %ld...exiting:%d",	numout, xorbdd_line);
			exit (0);
		}
		
		p = fgetc (finputfile);
		if (p != '\n') {
         p = fgetc(finputfile);
         while (p != EOF && p!= '\n')
            p = fgetc(finputfile);
			if (p != '\n')
			  goto Exit;
		} else ungetc (p, finputfile);
	}
	//d4_printf1("\n");
	Exit:;

	numinp = numinp+temp_vars;
	if (no_independent) {
		for (int x = 0; x < numinp + 1; x++) {
         if (independantVars[x] == 0)
            independantVars[x] = 1;
		}
	}
	
   d2_printf1("\rReading XOR ... Done\n");
	delete [] bdds;
}
