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
#include "solver.h"

ITE_INLINE void DisplayBacktrackInfo(double &fPrevEndTime, double &fStartTime);
char *cl_string, *cm_string;
int height;
int width;
int auto_wrap;

static char term_buffer[2048];
struct termios  newtty, origtty;            /* tty modes          */

static char *CM, *SO, *SE, *CL;
static char *tv_stype;
char *ptr;

/*
 * Get a required termcap string or exit with a message.
 */
static char * qgetstr(char *ref)
{
   char *tmp;

   if ((tmp = tgetstr(ref, &ptr)) == NULL) {
      printf("/etc/termcap terminal %s must have a %s= entry\n",
            tv_stype, ref);
   }
   return (tmp);
}


void
init_terminal_data()
{
   char *termtype = getenv("TERM");
   int success;
   extern char *getenv(), *realloc();
   char *tcapbuf;

   if (termtype == 0)
      perror ("Specify a terminal type with `setenv TERM <yourtype>'.\n");

   success = tgetent(term_buffer, termtype);
   if (success < 0)
      perror("Could not access the termcap data base.\n");
   if (success == 0)
      perror("Terminal type `...' is not defined.\n"/*, termtype*/);

   /* get far too much and shrink later */
   if ((ptr = tcapbuf = (char*)malloc(1024)) == NULL) {
      printf("out of space\n");
      exit(1);
   }

   CM = qgetstr("cm"); /* this string used by tgoto() */
   CL = qgetstr("cl"); /* this string used to clear screen */
   SO = qgetstr("so"); /* this string used to set standout */
   SE = qgetstr("se"); /* this string used by clear standout */
}

/*
 * output char function.
 */
int ttputc(int c)
{
   fputc(c, stdout);
   return 0;
}

/*
 * output command string, set padding to one line affected.
 * use ttputc as character output function. Use only for
 * termcap created data not your own strings.
 */
void putpad(char *str)
{
   tputs(str, 1, ttputc);
}

/*
 * Move cursor.
 */
void
move(int col, int row)
{
   putpad(tgoto(CM, col, row));
}

void
crtwin_draw() //double fPrevEndTime, double fStartTime)
{
   double fPrevEndTime=0, fStartTime=0;
   putpad(CL);/* clear the screen */

   move(30, 5);
   putpad(SO);/* standout mode */
   printf("SBSAT %s", VERSION);
   putpad(SE);/* end standout mode */

   move(0, 7);

   {
      FILE *tmpstd = stddbg;
      stddbg = stdout;
      DisplayBacktrackInfo(fPrevEndTime, fStartTime);
      stddbg = tmpstd;
   }
   height = tgetnum("li");
   width = tgetnum("co");
   printf("co=%d, li=%d \r", height, width);
   fflush(stdout);
}

void
dump_lemmas(char *_filename)
{
   char filename[128];
   get_freefile(_filename, NULL, filename, 128);
   DisplayAllBrancherLemmasToFile(filename);
}

void
crtwin_cmd(char c)
{
   switch(c) {
    case 'L': if (*lemma_out_file) dump_lemmas(lemma_out_file); break;
    case '+': BACKTRACKS_PER_STAT_REPORT *= 2; break;
    case '-': BACKTRACKS_PER_STAT_REPORT /= 2; break;
    default: break;
   }
   crtwin_draw();
}

void
crtwin(void) {
   fd_set rfds;
   struct timeval tv;
   int retval = 0;
   int draw = 0;

   do {
      /* Watch stdin (fd 0) to see when it has input. */
      FD_ZERO(&rfds);
      FD_SET(0, &rfds);

      tv.tv_sec = 0;
      tv.tv_usec = 0;
      retval = select(1, &rfds, NULL, NULL, &tv);
      /* Don't rely on the value of tv now! */

      if (retval) {
         char c = getchar();
         printf("getchar: %x\n", c);
         draw = 1;
         crtwin_cmd(c);
      }
   } while (retval != 0);

   if (draw == 0) crtwin_draw();
}

void
crtwin_init()
{
   if (isatty(0/*fileno(stdin)*/)) {
      if (tcgetattr(0, &origtty) < 0) {
         perror("tcgetattr: stdin");
         exit(1);
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
         perror("tcsetattr: stdin");
         exit(1);
      }
   }
   init_terminal_data ();
}


