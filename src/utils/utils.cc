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
#include <sys/time.h>
#include <sys/resource.h>

long numinp, numout;

BDDNode *false_ptr = NULL;
BDDNode *true_ptr = NULL;
extern char temp_dir[256];

void *_ite_calloc(unsigned int x, unsigned int y, int dbg_lvl, const char *for_what) {
   //fprintf(stderr, "ite_calloc(x=%d, y=%d, dbg_lvl=%d, for_what=%s)\n", x, y, dbg_lvl, for_what);
   void *p = NULL;
   LONG64 r = x;
   r *= y;
   if (r >= INT_MAX) {
         fprintf(stderr, "ERROR: Unable to allocate %u (%u * %u) bytes for %s\n", x*y, x, y, for_what); 
         exit(1); 
   }
   if (x==0 || y==0) {
      dm2_printf2("WARNING: 0 bytes allocation for %s\n", for_what); 
   } else {
      p=calloc(x,y);
      if (!p) {
         fprintf(stderr, "ERROR: Unable to allocate %u (%u * %u) bytes for %s\n", x*y, x, y, for_what); 
         exit(1); 
      } 
   }
   DM_2(
         if ((DEBUG_LVL&15) >= dbg_lvl) 
         fprintf(stddbg, "Allocated %ld bytes for %s\n", (long)(x*y), for_what); 
      );
   return p;
}

void _ite_free(void **ptr) {
  if (*ptr != NULL) {
    free(*ptr);
    *ptr = NULL;
  }
}


void *_ite_recalloc(void *ptr, unsigned int oldx, unsigned int x, unsigned int y, int dbg_lvl, const char *for_what) {
   //fprintf(stderr, "ite_recalloc(ptr, oldx=%d, x=%d, y=%d, dbg_lvl=%d, for_what=%s)\n", oldx, x, y, dbg_lvl, for_what);
   void *p = NULL;
   LONG64 r = x;
   r *= y;
   if (r >= INT_MAX) {
         fprintf(stderr, "ERROR: Unable to allocate %u (%u * %u) bytes for %s\n", x*y, x, y, for_what); 
         exit(1); 
   }
   assert(oldx<x);
   if (x==0 || y==0) {
      dm2_printf2("WARNING: 0 bytes allocation for %s\n", for_what); 
   } else {
      p=realloc(ptr, x*y);
      if (!p) {
         fprintf(stderr, "ERROR: Unable to allocate %d bytes for %s\n", x*y, for_what); 
         exit(1); 
      } 
      void *pp = (char*)p+oldx*y;
      memset(pp, 0, (x-oldx)*y);
   }
   DM_2(
         if ((DEBUG_LVL&15) >= dbg_lvl) 
         fprintf(stddbg, "ReAllocated %d bytes for %s\n", x*y, for_what); 
      );
   return p;
}

void *_ite_realloc(void *ptr, unsigned int oldx, unsigned int x, unsigned int y, int dbg_lvl, const char *for_what) {
   //fprintf(stderr, "ite_realloc(ptr, oldx=%d, x=%d, y=%d, dbg_lvl=%d, for_what=%s)\n", oldx, x, y, dbg_lvl, for_what);
   void *p = NULL;
   LONG64 r = x;
   r *= y;
   if (r >= INT_MAX) {
         fprintf(stderr, "ERROR: Unable to allocate %u (%u * %u) bytes for %s\n", x*y, x, y, for_what); 
         exit(1); 
   }
   assert(oldx<x);
   if (x==0 || y==0) {
      dm2_printf2("WARNING: 0 bytes allocation for %s\n", for_what); 
   } else {
      p=realloc(ptr, x*y);
      if (!p) {
         fprintf(stderr, "ERROR: Unable to allocate %d bytes for %s\n", x*y, for_what); 
         exit(1); 
      } 
      //void *pp = (char*)p+oldx*y;
      //memset(pp, 0, (x-oldx)*y);
   }
   DM_2(
         if ((DEBUG_LVL&15) >= dbg_lvl) 
         fprintf(stddbg, "ReAllocated %d bytes for %s\n", x*y, for_what); 
      );
   return p;
}

//Returns run time in seconds.
double
get_runtime ()
{
  struct rusage ru;
  getrusage (RUSAGE_SELF, &ru);
  return ((double) ru.ru_utime.tv_sec * 1000 + ru.ru_utime.tv_usec / 1000 +
           ru.ru_stime.tv_sec * 1000 + ru.ru_stime.tv_usec / 1000) / 1000;
}

//#define LINUX_GET_MEMUSAGE
#ifdef LINUX_GET_MEMUSAGE
long
get_memusage ()
{
   int pid = getpid();
   char stat_buf[256];
   char buffer[2001];
   sprintf(stat_buf, "/proc/%d/status", pid);
   FILE *fin=fopen (stat_buf, "r");
   if (!fin) return 0;

   /* read the file in one swing */
   fread(buffer, 2000, 1, fin);
   fclose(fin);

   buffer[2000] = 0;

   char *c=buffer;

   /* find the end of the filename */
   //while (*c!=0 && *c!=')') c++;
   //if (*c == 0) return 0;

   /* find the start of the memory info */
   int space_counter=0;
   while (space_counter < 9) 
    {
      if (*c == ' ') space_counter++;
      c++;
    }

   *c='*';

   long memalloc=0;
   if (1!=sscanf(c+1, "%ld", &memalloc)) 
	  memalloc=0;
   //fprintf(stderr, "stat(%s):\n%s\nreturn: %ld", stat_buf, buffer, memalloc);
   return memalloc;
}
#else
long
get_memusage ()
{
  struct rusage ru;
  getrusage (RUSAGE_SELF, &ru);
  return ru.ru_maxrss/*+ru.ru_ixrss+ru.ru_idrss+ru.ru_isrss*/;
}
#endif

int compfunc (const void *x, const void *y)
{
  int pp, qq;

  pp = *(const int *) x;
  qq = *(const int *) y;
  if (pp < qq)
    return -1;
  if (pp == qq)
#ifndef FORCE_STABLE_QSORT
    return 0;
#else
    {
    if (x < y) return -1;
    else if (x > y) return 1;
    else return 0;
    }
#endif
  return 1;
}

int revcompfunc (const void *x, const void *y)
{
  int pp, qq;

  pp = *(const int *) x;
  qq = *(const int *) y;
  if (pp > qq)
    return -1;
  if (pp == qq)
#ifndef FORCE_STABLE_QSORT
    return 0;
#else
    {
    if (x > y) return -1;
    else if (x < y) return 1;
    else return 0;
    }
#endif
  return 1;
}


int abscompfunc (const void *x, const void *y)
{
  int pp, qq;

  pp = abs(*(const int *) x);
  qq = abs(*(const int *) y);
  if (pp < qq)
    return -1;
  if (pp == qq)
#ifndef FORCE_STABLE_QSORT
    return 0;
#else
    {
    if (x < y) return -1;
    else if (x > y) return 1;
    else return 0;
    }
#endif
  return 1;
}

int absrevcompfunc (const void *x, const void *y)
{
  int pp, qq;

  pp = abs(*(const int *) x);
  qq = abs(*(const int *) y);
  if (pp > qq)
    return -1;
  if (pp == qq)
#ifndef FORCE_STABLE_QSORT
    return 0;
#else
    {
    if (x < y) return -1;
    else if (x > y) return 1;
    else return 0;
    }
#endif
  return 1;
}

ITE_INLINE
void memcpy_ite( void* d, void* s, int size ) {

memcpy(d,s,size);
return;

#ifdef MK_NULL
 for(int i=size>>2;--i>=0;) {
                *(unsigned int*)d=*(const unsigned int*)s;
                ((unsigned char*)d)+=4;
                ((unsigned char*)s)+=4;
        }

 for(int i=size&3;i>0;i--) {
                *(unsigned char*)d=*(unsigned char* )s;
                (unsigned char*)d+=1;
                (unsigned char*)s+=1;
        }
return; 

void *_d = (void*)d;
void *_s = (void*)s;
int _size  =   (size>>2)<<2;
int _size2 =  _size&15;
	d += _size;
	s += _size;

       asm ("shr $4, %%ecx;"\
            "lx10:"\
            "movq  (%%esi),%%mm0;"\
            "movq 8(%%esi),%%mm1;"\
            "lea 16(%%esi),%%esi;"\
            /*"add $16,%%esi;"*/  \
            "movntq %%mm0, (%%edi);"\
            "prefetcht0 2048(%%esi);"\
            "movntq %%mm1,8(%%edi);"\
            "lea 16(%%edi),%%edi;"\
            /*"add $16,%%edi;"*/  \
            "dec %%ecx;"\
            "jnz lx10;" \
             :   : "D" (_d),
                   "S" (_s),
                   "c" (_size) : "mm0","mm1" );

        for(int i=_size2;i>0;i--) {
                *(unsigned char*)d=*(unsigned char* )s;
                d+=1;
                s+=1;
        }
#endif
}

void
get_freefile(char *basename, char *file_dir, char *filename, int filename_max)
{
  struct stat buf;
  int filename_len=0;

  if (file_dir != NULL && *file_dir != 0) 
	  strncpy(filename, file_dir, filename_max);	
  else
	  strcpy(filename, ".");

  strncat(filename, "/", filename_max-strlen(filename)-5);
  strncat(filename, basename, filename_max-strlen(filename)-5);	
  filename_len = strlen(filename);

  /* if it does not exist => it is good and it returns */
  if (stat(filename, &buf)!=0) return;

  /* otherwise we just append a number */
  {
  int i = 1;
  while (i<1000000) {
     sprintf(filename+filename_len, "%d", i);
     if (stat(filename, &buf)==0) i++;
     else return;
  }
  }
  printf("Please delete old files from %s\n", filename);
  exit (1);
}

char *
ite_basename(char *filename)
{
   char *ptr = strrchr(filename, '/');
   if (ptr == NULL) ptr = filename;
   else ptr = ptr+1;
   return ptr;
}

void
ite_strncpy(char *str_dst, char *str_src, int len)
{
   strncpy(str_dst, str_src, len);
   str_dst[len] = 0;
}

int signs_idx = 0;
//#define signs_max 6
//char signs[signs_max] = "/|\\- ";
#define signs_max 4
char signs[signs_max+1] = "/|\\-";
int last_print_roller = 0;

void
print_roller()
{
   if (last_print_roller) {
      fprintf(stddbg, "\b%c", signs[signs_idx++]);
   } else {
      fprintf(stddbg, "%c", signs[signs_idx++]);
      last_print_roller = 1; 
   }
   if (signs[signs_idx] == 0) signs_idx = 0;
}

void
print_nonroller()
{
   last_print_roller = 0; 
}

void
print_roller_init()
{
   last_print_roller = 0; 
   signs_idx = 0;
}

