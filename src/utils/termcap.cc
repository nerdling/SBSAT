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

char *cl_string, *cm_string;
int term_height=25;
int term_width=80;
int auto_wrap;

char term_buffer[2048];

char *CM, *SO, *SE, *CL;
char *tv_stype;
char *ptr;

/*
 * Get a required termcap string or exit with a message.
 */
char * qgetstr(char *ref)
{
   char *tmp=NULL;
#ifdef TERMCAP_H

   if ((tmp = tgetstr(ref, &ptr)) == NULL) {
      printf("/etc/termcap terminal %s must have a %s= entry\n",
            tv_stype, ref);
   }
#endif
   return (tmp);
}


int
init_terminal_out()
{
#ifdef TERMCAP_H
   char *termtype = getenv("TERM");
   int success;
   extern char *getenv(), *realloc();
   char *tcapbuf;

   if (termtype == 0) {
      fprintf(stderr, "Specify a terminal type with `setenv TERM <yourtype>'.\n");
      return 1;
   }

   success = tgetent(term_buffer, termtype);
   if (success < 0) {
      fprintf(stderr, "Could not access the termcap data base.\n");
      return 1;
   }
   if (success == 0) {
      fprintf(stderr, "Terminal type `...' is not defined.\n"/*, termtype*/);
      return 1;
   }

   /* get far too much and shrink later */
   ptr = tcapbuf = (char*)calloc(1024, sizeof(char));
   if (!ptr) {
      fprintf(stderr, "Can't allocate tcapbuf\n");
      exit(1); // memory problems -- can quit
   }
   CM = qgetstr("cm"); /* this string used by tgoto() */
   CL = qgetstr("cl"); /* this string used to clear screen */
   SO = qgetstr("so"); /* this string used to set standout */
   SE = qgetstr("se"); /* this string used by clear standout */
   term_height = tgetnum("li");
   term_width = tgetnum("co");
//   printf("co=%d, li=%d \r", height, width);
#endif
   return 0;
}

/*
 * output char function.
 */
int ttputc(int c)
{
#ifdef TERMCAP_H
   fputc(c, stdout);
#endif
   return 0;
}

/*
 * output command string, set padding to one line affected.
 * use ttputc as character output function. Use only for
 * termcap created data not your own strings.
 */
void putpad(char *str)
{
#ifdef TERMCAP_H
   tputs(str, 1, ttputc);
#endif
}

/*
 * Move cursor.
 */
void
move(int col, int row)
{
#ifdef TERMCAP_H
   putpad(tgoto(CM, col, row));
#endif
}


struct termios  newtty, origtty;            /* tty modes          */

int
init_terminal_in()
{
#ifdef TERMCAP_H
   if (isatty(0/*fileno(stdin)*/)) {
      if (tcgetattr(0, &origtty) < 0) {
         fprintf(stderr, "tcgetattr: stdin");
         return 1;
      }

      newtty = origtty;
      newtty.c_lflag &= ~(ICANON);
      newtty.c_cc[VMIN] = 1;

//      newtty.c_cc[VMIN] = 1;
//      newtty.c_cc[VTIME] = 0;
//      newtty.c_oflag &= ~OPOST;
//      newtty.c_lflag &= ~(ICANON|ISIG|ECHO);
//      newtty.c_iflag &= ~(INLCR|IGNCR|ICRNL|IUCLC|IXON);

      if (tcsetattr(0, TCSANOW, &newtty) < 0) {
         fprintf(stderr, "tcgetattr: stdin");
         return 1;
      }
   }
#endif
   return 0;
}

int
term_getchar()
{
#ifdef TERMCAP_H
   fd_set rfds;
   struct timeval tv;
   int retval = 0;

   /* Watch stdin (fd 0) to see when it has input. */
   FD_ZERO(&rfds);
   FD_SET(0, &rfds);

   tv.tv_sec = 0;
   tv.tv_usec = 0;
   retval = select(1, &rfds, NULL, NULL, &tv);
   /* Don't rely on the value of tv now! */

   if (retval) return getchar();
#endif
   return 0;
}


