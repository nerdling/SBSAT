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
#include "sbsat_formats.h"

#define MAXFILE_LENGTH 4000000
#define LINE_LENGTH 1024

/*=================================================================*/
#define OPS_NUM 2

int
look_up (char *s)
{
  char *ops[OPS_NUM] = { "-out", "-in" };
  int i;

  for (i = 0; i < OPS_NUM; i++)
    if (strcmp (ops[i], s) == 0)
      return i;

  return -1;
}

/*=================================================================*/
void
print_err (char *s, int stop)
{
  fprintf (stderr, "%s\n", s);
  if (stop)
    exit (-1);
}

/*=================================================================*/
char *Ws;			/* work string */
char *Acr;			/* Actual character in reading */
char *Acw;			/* Actual character in writing */
int C;

/*=================================================================*/
void
SAT_to_CNF ()
{
  int bracket = 0, d;
  char c;
  char *tmp;

  tmp = (char *) calloc (100, sizeof (char));
  Ws = (char *) calloc (MAXFILE_LENGTH, sizeof (char));
  if (Ws == NULL)
    print_err ("Not enough memory", 1);

  Acr = Acw = Ws;
  readfile ();
  Acw = Ws;

  while ((c = mygetc ()) != '\0')
    {
      switch (c)
	{
	case 'c':
	  myputc (c);
	  while ((c = mygetc ()) != '\n' && c != '\0')
	    myputc (c);
	  myputc ('\n');
	  break;

	case 'p':
	  for (; *(Acw - 1) != '\n'; Acw--);
	  myputc (c);
	  sscanf (Acr, "%d\n", &d);
	  while ((c = mygetc ()) != '\n' && c != '\0');
	  Acw += sprintf (Acw, " cnf %d %d\n", d, C);
	  break;

	case '-':
	  if (*(Acw - 1) != ' ')
	    myputc (' ');
	  myputc (c);
	  while ((c = mygetc ()) == '(' || c == ' ' || c == '\n')
	    if (c == '(')
	      bracket++;
	  myungetc (c);
	  break;

	case '\n':
	case ' ':
	case '+':
	case '*':
	  myputc (' ');
	  break;

	case '(':
	  bracket++;
	  myputc (' ');
	  break;

	case ')':
	  bracket--;
	  myputc (' ');
	  if (bracket == 2)
	    {
	      myputc ('0');
	      myputc ('\n');
	    }
	  break;

	default:
	  myputc (c);
	  break;
	}
    }

  myputc ('\n');
  myputc ('\0');

  writestring (Ws);

  free (Ws);
  free (tmp);
}

/*=================================================================*/
void
myputc (char c)
{
  if (Acw - Ws == MAXFILE_LENGTH)
    print_err ("File is longer than allocated space", 1);

  if (c == ' ' && (*(Acw - 1) == ' ' || *(Acw - 1) == '\n'))
    return;

  *Acw = c;
  Acw++;
}

/*=================================================================*/
char
mygetc (void)
{
  char c = *Acr;

  Acr++;
  return (c);
}

/*=================================================================*/
void
myungetc (char c)
{
  Acr--;
  *Acr = c;
}

/*=================================================================*/
void
readfile ()
{
  int c;
  int nl = 0, i, bracket = 0;

  C = 0;
  while (1)
    {
       c = fgetc(finputfile);
       switch (c)
       {
        case EOF: break;
        case '\n':
           nl = 1;
        case ' ':
        case '\t':

           c = fgetc(finputfile);
           while (c == ' ' || c == '\t' || c == '\n') {
              if (c == '\n')
                 nl = 1;
              c = fgetc(finputfile);
           }

           (nl == 1) ? myputc ('\n') : myputc (' ');
           nl = 0;
           ungetc (c, finputfile);
           break;

        case 'c':
           myputc ('c');
           c = fgetc(finputfile);
           for (i = 0; c != '\n' && i < LINE_LENGTH - 1 && c != EOF;
                 i++) {
              myputc (c);
              c = fgetc(finputfile);
           }
       

           myputc ('\n');
           if ((i == LINE_LENGTH - 1 && c != '\n') || c == EOF)
           {
              ungetc (c, finputfile);
              ungetc (' ', finputfile);
              ungetc ('c', finputfile);
           }
           break;

        case 'p':
           myputc (c);
           c = fgetc(finputfile);
           while (c != '\n' && c != EOF) {
              myputc (c);
              c = fgetc(finputfile);
           }
           for (i = 0; i < 20; i++, *Acw = ' ', Acw++);
           myputc ('\n');
           break;

        case '(':
           bracket++;
           myputc (c);
           break;

        case ')':
           bracket--;
           if (bracket == 2)
              C++;
           myputc (c);
           break;

        default:
           myputc (c);
           break;
       }
    }
  myputc ('\0');
  if (bracket != 0)
     print_err ("Error in input: Mismatching brackets.", 0);
}

//========================================================================
void
writestring (char *s)		/* writes a string to f cutting after
				   LINE_LENGTH characters */
{
  int i, nl;
  char *t, *p;
  int len = strlen (s);
  char *ls = (char *) calloc (LINE_LENGTH + 1, sizeof (char));

  if (ls == NULL)
    print_err ("Not enough memory", 1);

  for (i = 0; i < len;)
    {
      nl = (t = strchr (s, '\n')) ? t - s + 1 : len - i;

      if (nl <= LINE_LENGTH)
	{
	  strncpy (ls, s, nl);
	  *(ls + nl) = '\0';
	  s += nl;
	  i += nl;
	  fputs (ls, foutputfile);
	}
      else
	{
	  nl = LINE_LENGTH;
	  for (p = s + nl; *p != ' ' && p != s; p--, nl--);
	  if (p == s)
	    print_err ("Too short line_length.", 1);

	  strncpy (ls, s, nl);
	  *(ls + nl) = '\0';
	  s += nl;
	  i += nl;
	  fputs (ls, foutputfile);
	  fputc ('\n', foutputfile);
	}
    }

  free (ls);
}
