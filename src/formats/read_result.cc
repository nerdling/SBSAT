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
#include "ite.h"

extern char *input_result_filename;

int 
setValue(char *s, int value, Tracer *tracer, varinfo * variablelist)
{
 d9_printf3("Setting %s to %d\n", s, value);
 if (formatin == 't') {
    int i = tracer->intValue(s);
    if (i==0) {
      d1_printf2("Unknown symbol: %s\n", s);
      return 1;
    }
    if (value != -1) {
        if (i<0) { i=-i; value=value==0?1:0; };
        d1_printf4("setting %d (%s) to %d\n", i, s, value);
	if (variablelist[i].true_false != -1) {
	   if (variablelist[i].true_false != value) {
	     d1_printf2("Conflict of the imported values for %s\n", s);
	     exit(1);
  	   };
        } else
	variablelist[i].true_false = value;
    }
 } else {
  /* add other formats */
 }
 return 0;
}

int
read_input_result(FILE *fin, Tracer *tracer, varinfo *variablelist)
{
char c;
char *p_str=NULL;
int value;
char s[256];
while (!feof(fin)) 
{
  c=fgetc(fin);
  while (!feof(fin) && 
         !(isalpha(c) || isdigit(c) || c=='-' || c=='*' || c=='+' || c=='_')) 
         c=fgetc(fin);
  if (feof(fin)) break;
  switch (c) {
  case '-': value=0; break;
  case '+': value=1; break;
  case '*': value=-1; break;
  default: { value=1; s[0]=c; };
           break;
  }
  if (c=='-' || c=='+' || c=='*') fgets(s, 255, fin);
  else fgets(s+1, 254, fin);
  if ((p_str=strchr(s, '\n'))!=NULL) *p_str = 0;
  if (setValue(s, value, tracer, variablelist) != 0)
  {
    dE_printf2("unknown variable in the input file: %s\n", s);
    exit(1); 
  };
}

return 0;
}

