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

#include "config.h"
#include "params.h"
#include "log.h"

extern int term_width; 
int params_current_src = 0;

#ifndef COPYRIGHT
#define COPYRIGHT "Copyright (C) 1999-2009, University of Cincinnati.  All rights reserved."
#endif

#ifndef AUTHORS
#define AUTHORS "a research team lead by John Franco"
#endif

#ifndef BUGS_EMAIL
#define BUGS_EMAIL "weaversa@gmail.com"
#endif

#ifndef DESCRIPTION
#define DESCRIPTION "SBSAT is a SAT solver."
#endif

void
init_options()
{
  int i;

  for (i=0;!(options[i].p_target==NULL&&options[i].desc_opt[0]==0);i++)
  {
#ifndef HAVE_DES_INITIALIZERS
#ifdef NDEBUG
#define DEBUG_VSF(x) x;
#else
#define DEBUG_VSF(x) { v_sf = x; assert(1==v_sf); }
     int v_sf;
#endif
     int v_i;
     long v_l;
     float v_f;
     switch (options[i].p_type) {
      case P_PRE_CHAR:
      case P_CHAR: options[i].p_defa.c = options[i].p_defa.s[0]; 
                   options[i].p_value.c = options[i].p_value.s[0]; 
                   break;
      case P_PRE_INT:
      case P_INT: 
                   DEBUG_VSF(sscanf(options[i].p_defa.s, "%d", &v_i));
                   options[i].p_defa.i = v_i;
                   DEBUG_VSF(sscanf(options[i].p_value.s, "%d", &v_i));
                   options[i].p_value.i = v_i;
                   break;
      case P_PRE_LONG:
      case P_LONG: 
                   DEBUG_VSF(sscanf(options[i].p_defa.s, "%ld", &v_l));
                   options[i].p_defa.l = v_l;
                   DEBUG_VSF(sscanf(options[i].p_value.s, "%ld", &v_l));
                   options[i].p_value.l = v_l;
                   break;
      case P_PRE_FLOAT:
      case P_FLOAT: 
                   DEBUG_VSF(sscanf(options[i].p_defa.s, "%f", &v_f));
                   options[i].p_defa.f = v_f;
                   DEBUG_VSF(sscanf(options[i].p_value.s, "%f", &v_f));
                   options[i].p_value.f = v_f;
                   break;
      case P_PRE_STRING:
      case P_STRING: 
                   DEBUG_VSF(sscanf(options[i].p_value.s, "%d", &v_i));
                   options[i].p_value.i = v_i;
                   break;
      default: break; /* P_NONE, P_FN, ... */
     }
#endif

     if (options[i].p_target != NULL) 
     {
        switch (options[i].p_type) {
         case P_PRE_CHAR:
         case P_CHAR: *(char*)(options[i].p_target) = options[i].p_defa.c; break;
         case P_PRE_INT:
         case P_INT: *(int*)(options[i].p_target) = options[i].p_defa.i; break;
         case P_PRE_LONG:
         case P_LONG: *(long*)(options[i].p_target) = options[i].p_defa.l; break;
         case P_PRE_FLOAT:
         case P_FLOAT: *(float*)(options[i].p_target) = options[i].p_defa.f; break;
         case P_PRE_STRING:
         case P_STRING: strcpy((char*)(options[i].p_target), options[i].p_defa.s);
                        break;
         default: break; /* P_NONE, P_PRE_... */
        }
     } else {
        /* if p_target is NULL set the pointer to a default value */
        switch (options[i].p_type) {
         case P_CHAR: 
            options[i].p_target = (void*)&(options[i].p_defa.c); 
            break;
         case P_INT: 
            options[i].p_target = (void*)&(options[i].p_defa.i); 
            break;
         case P_LONG: 
            options[i].p_target = (void*)&(options[i].p_defa.l); 
            break;
         case P_FLOAT: 
            options[i].p_target = (void*)&(options[i].p_defa.f); 
            break;
         case P_STRING: 
            options[i].p_target = (void*)&(options[i].p_defa.s); 
            break;
         default: break; /* P_NONE, P_PRE_... */
        }
     }
  }
}

t_opt *
lookup_keyword(char *key)
{
   int i;

   for (i=0;!(options[i].p_target==NULL&&options[i].desc_opt[0]==0);i++)
   {
      if (options[i].w_opt[0] && !strcmp(key, options[i].w_opt))
         return &(options[i]);
   }
   return NULL;
}

t_opt *
lookup_short_keyword(char *key)
{
  int i;
  
  for (i=0;!(options[i].p_target==NULL&&options[i].desc_opt[0]==0);i++)
    {
      if (options[i].l_opt[0] && !strcmp(key, options[i].l_opt))
	return &(options[i]);
    }
  return NULL;
}

void
set_param_int(char *param, int value)
{
   t_opt *p_opt = lookup_keyword(param);
   if (p_opt == NULL) return;
   assert(p_opt->p_type == P_INT);
   if (p_opt->p_src <= params_current_src)
   {
      *(int*)(p_opt->p_target) = value;
      p_opt->p_src = params_current_src;
   }
}

void
change_defa_param_int(char *param, int value)
{
   t_opt *p_opt = lookup_keyword(param);
   if (p_opt == NULL) return;
   assert(p_opt->p_type == P_INT);
   p_opt->p_defa.i = value;
   *(int*)(p_opt->p_target) = value;
}

void
skip_eol(FILE *fini)
{
  char c=fgetc(fini);
  while (!feof(fini) && c!='\n') c = fgetc(fini);
}

void
read_ini(char *filename)
{
   char c=0;
   FILE *fini = fopen(filename, "r");
   char keyword[256]="";
   char *p_keyword=NULL;
   t_opt *p_opt;

   params_current_src = 1;

   if (!fini) 
   { 
      d3_printf2("warning: ini file not found %s\n", filename); 
      return; 
   } else {
      d2_printf2("using ini file %s\n", filename); 
   };

   while (!feof(fini))
   {
      c=fgetc(fini);
      while (!feof(fini) && !isalpha(c) && !isdigit(c) && c!='#' && c!='\n') 
      {
         c = fgetc(fini);
      }

      if (c=='\n' || feof(fini)) {
         continue;
      }
      if (c=='#') {
         skip_eol(fini);
         continue;
      }
      ungetc(c, fini);
      fscanf(fini, "%s", keyword);
      p_keyword=strchr(keyword, '='); 
      if (p_keyword) {
         *p_keyword=0;
         p_keyword++;
      }

      //set_param_value(keyword, p_keyword);

      if ((p_opt = lookup_keyword(keyword))==NULL || 
            (p_opt->var_type&(VAR_INI+VAR_CHECK))==0)
      {
         printf("error ini file: unknown keyword %s\n", keyword);
         exit(1);
         skip_eol(fini);
         continue;
      }
      if (p_opt->p_src > params_current_src) 
      { 
         /* already set from cmd line */
         skip_eol(fini);
         continue;
      }

      /* set src for all vars with the same target var location before this one variable */
      if (p_opt > options) {
         t_opt *x_opt = p_opt;
         while ((--x_opt)->p_target == p_opt->p_target) x_opt->p_src = params_current_src;
      }
      /* and after this one */
      {
         t_opt *x_opt = p_opt;
         while ((++x_opt)->p_target == p_opt->p_target) x_opt->p_src = params_current_src;
      }
      /* set src = ini file */
      p_opt->p_src = params_current_src;

      if (p_opt->p_type <= P_NONE || p_opt->p_type == P_FN) 
      {
         if (p_keyword) printf("warning ini file: extra characters after %s\n", keyword);
         switch (p_opt->p_type) {
          case P_PRE_CHAR: *(char*)(p_opt->p_target) = p_opt->p_value.c; break;
          case P_PRE_INT: *(int*)(p_opt->p_target) = p_opt->p_value.i; break;
          case P_PRE_LONG: *(long*)(p_opt->p_target) = p_opt->p_value.l; break;
          case P_PRE_FLOAT: *(float*)(p_opt->p_target) = p_opt->p_value.f; break;
          case P_PRE_STRING: strcpy((char*)(p_opt->p_target), p_opt->p_value.s); 
                             break;
          case P_FN: ((p_fn)(p_opt->p_target))(); break;
          default: break; /* P_NONE */
         }
      } else {
         if (!p_keyword) 
         {
            printf("error ini file: missing value for %s\n",  keyword);
            exit(1);
            skip_eol(fini);
            continue;
         }
         switch (p_opt->p_type) {
          case P_CHAR: 
             if (sscanf(p_keyword, "%c", (char*)(p_opt->p_target))!=1) {
                printf("error ini file: parameter is not char for %s\n", keyword);
                exit(1);
                skip_eol(fini);
                continue;
             };
             break;
          case P_INT: 
             if (sscanf(p_keyword, "%d", (int*)(p_opt->p_target))!=1) {
                printf("error ini file: parameter is not integer for %s\n", keyword);
                exit(1);
                skip_eol(fini);
                continue;
             };
             break;
          case P_LONG: 
             if (sscanf(p_keyword, "%ld", (long*)(p_opt->p_target))!=1) {
                printf("error ini file: parameter is not long integer for %s\n", keyword);
                exit(1);
                skip_eol(fini);
                continue;
             };
             break;
          case P_FLOAT: 
             if (sscanf(p_keyword, "%f", (float*)(p_opt->p_target))!=1) {
                printf("error ini file: parameter is not float for %s\n", keyword);
                exit(1);
                skip_eol(fini);
                continue;
             };
             break;
          case P_STRING: {
             char *p_str=NULL;
             if (*p_keyword != '"') {
                printf("error ini file: missing initial quotes for %s\n", keyword);
                exit(1);
                skip_eol(fini);
                continue;
             }
             if ((p_str = strchr(p_keyword+1, '"'))!=NULL) *p_str=0;
             else {
                /* get the rest of the string from the file */
                p_str = p_keyword+strlen(p_keyword);
                *p_str = ' '; p_str++;
                c = fgetc(fini);
                while (!feof(fini)) {
                   c = fgetc(fini);
                   if (c=='"' || c=='\n') break;
                   *p_str = c;
                   p_str++;
                }
                *p_str=0; 
                if (c!= '"') {
                   printf("error ini file: missing end quotes for %s\n", keyword);
                   exit(1);
                   skip_eol(fini);
                   continue;
                }
             }
             strncpy((char*)(p_opt->p_target), p_keyword+1, p_opt->p_value.i-1);
             ((char*)(p_opt->p_target))[p_opt->p_value.i-1]=0;
          } break;
          case P_FN_INT: 
             int arg;
             if (sscanf(p_keyword, "%d", &arg)!=1) {
                printf("error ini file: parameter is not integer for %s\n", keyword);
                exit(1);
                skip_eol(fini);
                continue;
             };
             ((p_fn_int)(p_opt->p_target))(arg/*, p_keyword*/);
             break;
          default: break;
         }
      }
      skip_eol(fini);
   }

   fclose(fini);
}

void
read_cmd(int argc, char *argv[])
{
   int i;
   t_opt *p_opt;
   int params=0; 

   params_current_src = 2;

   for (i=1;i<argc;i++)
   {
      d9_printf3("parameter(%d): %s\n", i, argv[i]);
      if (argv[i][0]!='-') 
      {
         /* fixed parameters - inputfile, outputfile */
         char tmp_str[32];
         params++;
         switch (params) {
          case 1: p_opt = lookup_keyword(strcpy(tmp_str,"input-file")); break;
          case 2: p_opt = lookup_keyword(strcpy(tmp_str,"output-file")); break;
          default: p_opt = NULL; break;
         }
         if (p_opt == NULL ) {
            printf("error: Option error %s\n", argv[i]);
            exit(1);
            continue;
         }
         i--;
      } else
         if (argv[i][1]!='-')
            p_opt = lookup_short_keyword(argv[i]+1);
         else
            p_opt = lookup_keyword(argv[i]+2);
      if (!p_opt || (p_opt->var_type&(VAR_CMD+VAR_CHECK))==0) { 
         printf("error: Unknown option %s\n", argv[i]);
         exit(1);
         continue;
      }
      if (p_opt->p_src > params_current_src) 
      {
         /* already set from higher src */
         // skip = 1;
         if (p_opt->p_type <= P_NONE || p_opt->p_type == P_FN) {
            continue;
         } else {
            i++;
            if (i == argc) {
               printf("error: missing parameter for %s\n", argv[i-1]);
               exit(1);
               continue;
            }
            continue;
         }
      }
      if (p_opt > options) {
         t_opt *x_opt = p_opt;
         while ((--x_opt)->p_target == p_opt->p_target) x_opt->p_src = params_current_src;
      }
      {
         t_opt *x_opt = p_opt;
         while ((++x_opt)->p_target == p_opt->p_target) x_opt->p_src = params_current_src;
      }
      p_opt->p_src = params_current_src; /* cmd line source */

      if (p_opt->p_type <= P_NONE || p_opt->p_type == P_FN) {
         d9_printf1("found predefined keyword\n");
         if (p_opt->p_target) {
            switch (p_opt->p_type) {
             case P_PRE_CHAR: *(char*)(p_opt->p_target) = p_opt->p_value.c; break;
             case P_PRE_INT: *(int*)(p_opt->p_target) = p_opt->p_value.i; break;
             case P_PRE_LONG: *(long*)(p_opt->p_target) = p_opt->p_value.l; break;
             case P_PRE_FLOAT: *(float*)(p_opt->p_target) = p_opt->p_value.f; break;
             case P_PRE_STRING: strcpy((char*)(p_opt->p_target), p_opt->p_value.s);
                                break;
             case P_FN: ((p_fn)(p_opt->p_target))(); break;
             default: break; /* P_NONE */
            }
         }
      } else {
         d9_printf1("found keyword with parameter\n");
         i++;
         if (i == argc) {
            printf("error: missing parameter for %s\n", argv[i-1]);
            exit(1);
            continue;
         }
         switch (p_opt->p_type) {
          case P_CHAR: 
             if (sscanf(argv[i], "%c", (char*)(p_opt->p_target)) != 1) {
                printf("error: parameter is not char for %s\n", argv[i-1]); 
                exit(1);
                continue;
             }; 
             break;
          case P_INT: 
             if (sscanf(argv[i], "%d", (int*)(p_opt->p_target)) != 1) {
                printf("error: parameter is not integer for %s\n", argv[i-1]); 
                exit(1);
                continue;
             }; 
             break;
          case P_LONG: 
             if (sscanf(argv[i], "%ld", (long*)(p_opt->p_target)) != 1) {
                printf("error: parameter is not long integer for %s\n", argv[i-1]); 
                exit(1);
                continue;
             }; 
             break;
          case P_FLOAT: 
             if (sscanf(argv[i], "%f", (float*)(p_opt->p_target)) != 1) {
                printf("error: parameter is not float for %s\n", argv[i-1]);
                exit(1);
                continue;
             }; 
             break;
          case P_STRING: 
             strncpy((char*)(p_opt->p_target), argv[i], p_opt->p_value.i-1);
             ((char*)(p_opt->p_target))[p_opt->p_value.i-1]=0;
             break;
          case P_FN_INT: 
             int arg;
             if (sscanf(argv[i], "%d", &arg) != 1) {
                printf("error ini file: parameter is not integer for %s\n", argv[i-1]);
                exit(1);
                continue;
             };
             ((p_fn_int)(p_opt->p_target))(arg/*, argv[i]*/);
             break;
          case P_FN_STRING: 
             ((p_fn_string)(p_opt->p_target))(argv[i]);
             break;
          default: break;
         };
      };
      continue;
   }
}

void
dump_params()
{
   int i;

   for (i=0;!(options[i].p_target==NULL&&options[i].desc_opt[0]==0);i++)
   {
      if ((options[i].var_type&VAR_DUMP) == 0) continue;
      if (options[i].p_target == NULL && options[i].p_type == P_NONE) {
         printf("%s\n", options[i].desc_opt);
         continue;	
      } 
      if (options[i].w_opt[0]) printf("%s", options[i].w_opt);
      else
         if (options[i].l_opt[0]) printf("OPT_%s", options[i].l_opt);

      if (options[i].p_target) {
         switch (options[i].p_type) {
          case P_PRE_CHAR: 
             if (options[i].p_value.c==*(char*)(options[i].p_target))
                printf(" is set "); else printf(" is not set ");
             break;
          case P_PRE_INT:
             if (options[i].p_value.i==*(int*)(options[i].p_target))
                printf(" is set "); else printf(" is not set ");
             break;
          case P_PRE_LONG:
             if (options[i].p_value.l==*(long*)(options[i].p_target))
                printf(" is set "); else printf(" is not set ");
             break;
          case P_PRE_FLOAT:
             if (options[i].p_value.f==*(float*)(options[i].p_target))
                printf(" is set "); else printf(" is not set ");
             break;
          case P_PRE_STRING:
             if (!strcmp(options[i].p_value.s, (char*)(options[i].p_target)))
                printf(" is set "); else printf(" is not set ");
             break;
          default: break; /* P_NONE, P_type */
         }

         switch (options[i].p_type) {
          case P_PRE_CHAR: 
          case P_CHAR: printf("=%c", *(char*)(options[i].p_target)); break;
          case P_PRE_INT:
          case P_INT: printf("=%d", *(int*)(options[i].p_target)); break;
          case P_PRE_LONG:
          case P_LONG: printf("=%ld", *(long*)(options[i].p_target)); break;
          case P_PRE_FLOAT:
          case P_FLOAT: printf("=%f", *(float*)(options[i].p_target)); break;
          case P_PRE_STRING:
          case P_STRING: printf("=\"%s\"", (char*)(options[i].p_target)); break;
          default: break; /* P_NONE */
         }
      }
      printf("\n");
   }
}

void 
fprintf_desc(FILE *fout, char *desc, char *first_line, char *new_line)
{
   char *prev_line=desc;
   char *p_line=NULL;
   int line_no=0;
   int line_len=0;
   if (first_line == NULL) first_line = new_line;
   while ((p_line=strchr(prev_line, '\n'))!=NULL) {
      *p_line = 0;
      if (strlen(prev_line)) fprintf(fout, "%s%s\n", (line_no++?new_line:first_line), prev_line);
      else fprintf(fout, "%s\n", (line_no++?new_line:first_line));
      *p_line = '\n';
      prev_line = p_line+1;	
   };
   line_len=strlen((line_no?new_line:first_line));
   //fprintf(stdout, "%d(%d,%d)",line_len,line_no,term_width);
   while ((int)(line_len + strlen(prev_line)) > (int)(term_width-2)) {
      char tmp_char;
      char *p_line2;
      p_line = prev_line + (term_width-2 - line_len);
      tmp_char = *p_line; *p_line = 0;
      p_line2 = strrchr(prev_line, ' ');
      if (p_line2 == NULL) p_line2 = strchr(p_line+1, ' ');
      *p_line = tmp_char;
      if (p_line2 != NULL) {
         tmp_char = *p_line2; *p_line2 = 0;
      }
      fprintf(fout, "%s%s\n", (line_no++?new_line:first_line), 
            prev_line);
      if (p_line2 != NULL) {
         *p_line2 = tmp_char;
         prev_line = p_line2+1;
      } else {
         prev_line = NULL;
         break;
      }
      line_len = strlen(new_line);
   }
   if (prev_line)
      fprintf(fout, "%s%s\n", (line_no++?new_line:first_line), prev_line);
}

void
show_ini()
{
   int i;
   FILE *stdini=stdout;

   fprintf(stdini, "# \n");
   fprintf(stdini, "# %s Version %s %s\n", PACKAGE, VERSION, COPYRIGHT);
   fprintf(stdini, "# \n");
   for (i=0;!(options[i].p_target==NULL&&options[i].desc_opt[0]==0);i++)
   {
      if ((options[i].var_type&VAR_INI) == 0) continue;

      if (options[i].p_target==NULL && options[i].p_type==P_NONE)  {
         char tmp_str[5];
         fprintf_desc(stdini, options[i].desc_opt, NULL, strcpy(tmp_str, "# "));
         fprintf(stdini, "#\n");
         continue;
      }
      if (options[i].w_opt[0]==0) continue;
      if (options[i].desc_opt[0])  {
         char tmp_str[5];
         fprintf_desc(stdini, options[i].desc_opt, NULL, strcpy(tmp_str, "# "));
      }

      switch (options[i].p_type) {
       case P_PRE_CHAR: 
          if (options[i].p_value.c != options[i].p_defa.c) 
             fprintf(stdini, "# ");
          break;
       case P_PRE_INT: 
          if (options[i].p_value.i != options[i].p_defa.i) 
             fprintf(stdini, "# ");
          break;
       case P_PRE_LONG: 
          if (options[i].p_value.l != options[i].p_defa.l) 
             fprintf(stdini, "# ");
          break;
       case P_PRE_FLOAT: 
          if (options[i].p_value.f != options[i].p_defa.f) 
             fprintf(stdini, "# ");
          break;
       case P_PRE_STRING: 
          if (!strcmp(options[i].p_value.s, options[i].p_defa.s)) 
             fprintf(stdini, "# ");
          break;
       default: break; /* P_NONE, P_type */
      }

      fprintf(stdini, "%s", options[i].w_opt);
      switch (options[i].p_type) {
       case P_CHAR: fprintf(stdini, "=%c", options[i].p_defa.c); break;
       case P_INT: fprintf(stdini, "=%d", options[i].p_defa.i); break;
       case P_LONG: fprintf(stdini, "=%ld", options[i].p_defa.l); break;
       case P_FLOAT: fprintf(stdini, "=%f", options[i].p_defa.f); break;
       case P_STRING: fprintf(stdini, "=\"%s\"", options[i].p_defa.s); break;
       default: break;
      } 
      fprintf(stdini, "\n\n");
   }
   exit(0);
}


void
show_help()
{
   int i;
   FILE *stdhelp=stdout;
   char left_str[81];
   int left_size = (int)(term_width-2)/3;
   if (left_size < 26) left_size = 26;
   if (left_size > 80) left_size = 80;
   strncpy(left_str, "                                                                                  ", left_size);
   left_str[left_size] = 0;

   fprintf (stdhelp, "%s\n", DESCRIPTION);
   fprintf (stdhelp, 
         "Usage: sbsat [OPTIONS]... [inputfile [outputfile]]\n\n");
   fprintf (stdhelp, 
         "Options:\n");

   for (i=0;!(options[i].p_target==NULL&&options[i].desc_opt[0]==0);i++)
   {
      char line[256];
      char default_line[256];
      int line_len=0;

      if ((options[i].var_type&VAR_CMD) == 0) continue;
      if (options[i].p_target==NULL && options[i].p_type==P_NONE)  {
         fprintf(stdhelp, "%s\n", options[i].desc_opt);
         continue;
      }

      if (options[i].w_opt[0]) {
         sprintf(line, "  --%s", options[i].w_opt);
         if (options[i].p_type==P_INT) 
            strcat(line, " <number>");
         else if (options[i].p_type==P_FLOAT) 
            strcat(line, " <number>");
         else if (options[i].p_type==P_STRING) 
            strcat(line, " <string>");
         else if (options[i].p_type==P_CHAR) 
            strcat(line, " <char>");
      } else  sprintf(line, "  ");
      if (options[i].l_opt[0]) {
         if (options[i].w_opt[0]) strcat(line, ", "); //else strcat(line, "  ");
         strcat(line, "-"); 
         strcat(line, options[i].l_opt);
         if (options[i].p_type==P_INT) 
            strcat(line, " <number>");
         else if (options[i].p_type==P_FLOAT) 
            strcat(line, " <number>");
         else if (options[i].p_type==P_STRING) 
            strcat(line, " <string>");
         else if (options[i].p_type==P_CHAR) 
            strcat(line, " <char>");
      }
      line_len = strlen(line);
      if (line_len < left_size) 
         strncat(line, left_str, left_size-line_len);
      else {
         strcat(line, "\n");
         fprintf(stdhelp, "%s", line);
         strncpy(line, left_str, left_size);
         line[left_size]=0;
      }

      default_line[0]=0;
      switch (options[i].p_type) {
       case P_PRE_CHAR:
          if (options[i].p_value.c == options[i].p_defa.c)
             sprintf(default_line, "[default]");
          break;
       case P_PRE_INT:
          if (options[i].p_value.i == options[i].p_defa.i)
             sprintf(default_line, "[default]");
          break;
       case P_PRE_LONG:
          if (options[i].p_value.l == options[i].p_defa.l)
             sprintf(default_line, "[default]");
          break;
       case P_PRE_FLOAT:
          if (options[i].p_value.f == options[i].p_defa.f)
             sprintf(default_line, "[default]");
          break;
       case P_PRE_STRING:
          if (!strcmp(options[i].p_value.s, options[i].p_defa.s))
             sprintf(default_line, "[default]");
          break;
       case P_CHAR:
          sprintf(default_line, "[default=\'%c\']", options[i].p_defa.c); 
          break;
       case P_INT:
          sprintf(default_line, "[default=%d]", options[i].p_defa.i); 
          break;
       case P_LONG:
          sprintf(default_line, "[default=%ld]", options[i].p_defa.l); 
          break;
       case P_FLOAT:
          sprintf(default_line, "[default=%f]", options[i].p_defa.f); 
          break;
       case P_STRING:
          sprintf(default_line, "[default=\"%s\"]", options[i].p_defa.s); 
          break;
       default: break; /* P_NONE */
      }

      if (strchr(options[i].desc_opt, '\n') == NULL &&
            (int)(strlen(line)+strlen(options[i].desc_opt)+strlen(default_line)) <= (int)(term_width-2))
         fprintf(stdhelp, "%s%s %s\n",  line, options[i].desc_opt, default_line);
      else 
      {
         if ((int)(strlen(line)+strlen(options[i].desc_opt)) <= (int)(term_width-2))
            fprintf(stdhelp, "%s%s\n%s%s\n",  
                  line, options[i].desc_opt, left_str, default_line);
         else 
         {
            fprintf_desc(stdhelp, options[i].desc_opt, line, left_str);
            fprintf(stdhelp, "%s%s\n", left_str, default_line);
         }
      }
   }
   fprintf(stdhelp, "\nPlease report bugs to %s.\n", BUGS_EMAIL);
   exit(0);  
}

void
show_version()
{
   fprintf(stdout, "%s %s\n\n%s\n\nWritten by %s.\n", 
         PACKAGE, VERSION, COPYRIGHT, AUTHORS);
   exit(0);
}

void
show_competition_version()
{
   fprintf(stdout, "c %s %s\nc \nc %s\nc \nc Written by %s.\n", 
         PACKAGE, VERSION, COPYRIGHT, AUTHORS);
}

void
fix_ini_filename()
{
   char tmp_str[5];
   t_opt *p_opt = lookup_keyword(strcpy(tmp_str, "ini"));
   if (!p_opt) return;
   if (((char*)(p_opt->p_target))[0] == '~') {
      char *env = getenv("HOME");
      if (env)
      {
         char temp_str[256];
         sprintf(temp_str, "%s%s",
               env, ((char*)(p_opt->p_target))+1);
         strncpy((char*)(p_opt->p_target), temp_str,
               p_opt->p_value.i-1);
         ((char*)(p_opt->p_target))[p_opt->p_value.i-1]=0;
      }
   }
}
