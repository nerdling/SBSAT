#ifndef SBSAT_UTILS_H
#define SBSAT_UTILS_H
/*
#define ite_free(n) free(*(n))
#define ite_calloc(x,y,dbglvl,forwhat) calloc(x,y)
#define ite_recalloc(ptr,oldx,x,y,dbglvl,forwhat) _ite_recalloc(ptr,oldx,x,y,dbglvl,forwhat)
#define ite_realloc(ptr,oldx,x,y,dbglvl,forwhat) realloc(ptr,x*y) 
*/

#define ite_free(n) _ite_free(n)
#define ite_calloc(x,y,dbglvl,forwhat) _ite_calloc(x,y,dbglvl,forwhat)
#define ite_recalloc(ptr,oldx,x,y,dbglvl,forwhat) _ite_recalloc(ptr,oldx,x,y,dbglvl,forwhat)
#define ite_realloc(ptr,oldx,x,y,dbglvl,forwhat) _ite_realloc(ptr,oldx,x,y,dbglvl,forwhat)


void _ite_free(void **ptr);
void *_ite_calloc(unsigned int x, unsigned int y, int dbg_lvl, const char *for_what);
void *_ite_recalloc(void *ptr, unsigned int oldx, unsigned int x, unsigned int y, int dbg_lvl, const char *for_what);
void *_ite_realloc(void *ptr, unsigned int oldx, unsigned int x, unsigned int y, int dbg_lvl, const char *for_what);

char *ite_basename(char *filename);
void ite_strncpy(char *str_dst, char *str_src, int len);


#define ITE_NEW_CATCH(x, desc) \
 try { x; } catch(...) { \
  cerr << "ERROR: Unable to allocate memory for " << desc; \
  cerr << "IN FILE " << __FILE__ << " AT " << __LINE__ << endl; \
  exit(1); };

#endif
