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


int
check_gzip (char *filename)
{
  int is_gzip=0;
  FILE *fin;
  if (!strcmp(filename, "-")) fin = stdin;
  else  fin = fopen (filename, "rb");
  if (fin)
    {
      unsigned char x1 = (unsigned char)fgetc (fin);
      unsigned char x2 = (unsigned char)fgetc (fin);
      if (x1 == (unsigned char)'\037' && 
          x2 == (unsigned char)'\213') is_gzip=1;
      ungetc(x2, fin);
      ungetc(x1, fin);
      if (fin != stdin) fclose (fin);
    }
   return is_gzip;
}

FILE*
zread(char *filename)
{
    FILE *infile;
    char cmd[256];

    if (!strcmp(filename, "-")) {
       fprintf(stderr, "Can not accept gzipped data on the standard input\n");
       fprintf(stderr, "Please use the filename.gz as a parameter or gzip -dc for the stdin instead\n");
       fprintf(stderr, "Example: cat filename.gz | gzip -dc | ite\n");
       exit(1);
    }

    strcpy(cmd, "gzip -dc ");  /* use "gzip -c" for zwrite */
    strncat(cmd, filename, sizeof(cmd)-strlen(cmd));
    infile = popen(cmd, "r");  /* use "w" for zwrite */
    if (infile == NULL) {
	fprintf(stderr, "popen('%s', 'r') failed\n", cmd);
	exit(1);
    }

    return infile;
}
