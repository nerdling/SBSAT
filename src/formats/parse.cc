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
 *  parse.cc (J. Franco)
 *  Functions used by flatten.cc and tracer.cc to parse
 *  input lines.
 *********************************************************/

#include "ite.h"
#include "formats.h"

int indexOf(char *str, char c) {
   if (str == NULL) return -1;
   int len = strlen(str);
   int i=0;
   for (char *ptr=str ; *ptr != c && i < len ; ptr++, i++);
   if (i == len) return -1;
   return i;
}

int indexFrom(char *fnt, char *str, char c) {
   int i=0;
   char *ptr;
	
   if (fnt == NULL) return -1;
   for (ptr=str ; *ptr != c && ptr >= fnt ; ptr--, i++);
   if (ptr >= fnt) return i;
   return -1;
}

char *Trimmer::get (char *str) {
   if (str == NULL) return NULL;
   char *p;
      
   p = str;
   for ( ; *p == ' ' || *p == '\t' || *p == '\n' || *p == '\r' ; p++);
   if (*p == 0) return NULL;

   //delete trim;
   //trim = new char[strlen(p)+2];
   strncpy(trim, p, 15998);
   for (p=&trim[strlen(trim)-1] ; p != trim ; p--) {
      if (*p != ' ' && *p != '\t' && *p != '\n' && *p != '\r') break;
   }
   *(p+1) = 0;
   return trim;
}

char *Substring::get(char *s, int l, int u) {
   if (l >= u) return NULL;
   //delete str;
   char *lptr = &s[l];
   //str = new char[u-l+3];
   strncpy(str, lptr, u-l);
   str[u-l] = 0;
   return str;
}

StringTokenizer::StringTokenizer (char *s, char *d) {
   long length = (long)strlen(s);
   more_tokens = true;
   //int dlen = strlen(d);
   //str = new char[strlen(s)+3];
   strcpy(str, s);
   if (length && str[length-1] == '\n') str[length-1] = 0;
   //delim = new char[dlen+2];
   strcpy(delim, d);
   int delim_len = (int)strlen(delim);
   for (ptr=str ; *ptr != 0 ; ptr++) {
      for (int i=0 ; i < delim_len ; i++)
	 if (delim[i] == *ptr) goto a;
      return;
a:;
   }
   more_tokens = false;
}
   
void StringTokenizer::renewTokenizer (char *s, char *d) {
   //delete str;
   //delete delim;
   more_tokens = true;
   //int dlen = strlen(d);
   //str = new char[strlen(s)+3];
   strcpy(str, s);
   if (str[strlen(s)-1] == '\n') str[strlen(s)-1] = 0;
   //delim = new char[dlen+2];
   strcpy(delim, d);
   for (ptr=str ; *ptr != 0 ; ptr++) {
      for (size_t i=0 ; i < strlen(delim) ; i++)
	 if (delim[i] == *ptr) goto b;
      return;
b:;
   }
   more_tokens = false;
}

char *StringTokenizer::nextToken () {
   if (*ptr == 0) return NULL;
   eptr = ptr;
   for ( ; *ptr != 0 ; ptr++) {
      for (size_t i=0 ; i < strlen(delim) ; i++) {
	 if (delim[i] == *ptr) {
	    *ptr = 0;
	    for (ptr++ ; *ptr != 0 ; ptr++) {
	       for (size_t j=0 ; j < strlen(delim) ; j++) {
		  if (delim[j] == *ptr) goto b;
	       }
	       return eptr;
b:;
	    }
	    more_tokens = false;
	    return eptr;
	 }
      }
   }
   more_tokens = false;
   return eptr;
}

int StringTokenizer::countTokens () {
   int count = 0, i, sl = strlen(delim);
   bool flag = true;
   char *tptr;
      
   for (tptr = ptr ; *tptr != 0 ; tptr++) {
      for (i=0 ; i < sl ; i++) if (delim[i] == *tptr) break;
      if (i == sl) flag = false;
      else if (!flag) {
	 flag = true;
	 count++;
      }
   }
   if (!flag) count++;
   return count;
}

char *StringTokenizer::prevToken () {
   if (ptr == str) return NULL;
   eptr = ptr;
   for ( ; *eptr != 0 && eptr != str ; eptr--);
   if (eptr == str) return NULL;
   for (eptr-- ; *eptr != 0 && eptr != str ; eptr--);
   return eptr;
}
	
bool StringTokenizer::hasMoreTokens () {
   return more_tokens;
}

char *Object::toString () {
   sprintf(return_buf, "%ld", (long)this);
   return return_buf;
}

Integer::Integer (int n, char *hi) {
   hash_ident = new char[strlen(hi)+1];
   strcpy(hash_ident, hi);
   number = n; 
}
Integer::~Integer () { delete [] hash_ident; }
int Integer::intValue () { return number; }
char *Integer::getId () { return hash_ident; }


Cell::Cell (Node *node, Cell *next) {
   this->next = next;
   this->node = node;
}
Cell::~Cell() {
	delete this->node;	
}
Node *Cell::getNode () { return node; }
Cell *Cell::getNext () { return next; }
void Cell::setNext (Cell *c) { next = c; }
void Cell::setNode (Node *n) { node = n; }

Node::Node (char *id) {
	this->id = new char[strlen(id)+1];
   strcpy(this->id, id);
   marked = false;
   next = NULL;
   up = NULL;
}
Node::~Node() {
	if(next!=NULL) { next->setNext(NULL);	next->setNode(NULL); delete next; }
	delete [] this->id;  //LOOK! don't delete this here...
}
bool Node::getMark () { return marked; }
char *Node::getId () { return id; }
Cell *Node::getNext () { return next; }
Node *Node::getUp () { return up; }
void Node::setMark () { marked = true; }
void Node::setNext (Cell *next) { this->next = next; }
void Node::setUp (Node *up) { this->up = up; }


// ++ ++ ++ ++ ++ ++ ++ ++ ++ ++ ++ ++ ++ ++ ++ ++ ++ ++ ++ ++ ++ ++

HashCell::HashCell (Object *object, char *ident, HashCell *next) {
   this->next = next;
   this->ident = new char[strlen(ident)+1];
   strcpy(this->ident, ident);
   this->object = object;
}

int Hashtable::hash_func (char *s) {
   long acc = 1;
   for (unsigned int i=0 ; i < strlen(s) ; i++) 
      acc = (acc*(int)(s[i]+1)) % 10000;
   return acc;
}
	
Hashtable::Hashtable () {
   table = new HashCell*[10000];
   for (int i=0 ; i < 10000 ; i++) table[i] = NULL;
   nitems = 0;
}

Hashtable::~Hashtable () {
   for (int i=0 ; i < 10000 ; i++) {
      if (table[i] == NULL) continue;
      for (HashCell *p=table[i] ; p != NULL ; ) {
			HashCell *t = p->next;
			delete p;
			p = t;
      }
   }
   delete [] table;
}

void Hashtable::put (char *key, Object *object) {
   int loc = hash_func (key);
   for (HashCell *p=table[loc] ; p != NULL ; p=p->next) {
      if (!strcmp(p->ident, key)) {
			delete object;
			return;
		}
   }
   table[loc] = new HashCell(object, key, table[loc]);
   nitems++;
}

Object *Hashtable::get (char *key) {
   int loc = hash_func (key);
   for (HashCell *p=table[loc] ; p != NULL ; p=p->next) {
      if (!strcmp(p->ident, key)) return p->object;
   }
   return NULL;
}

Object **Hashtable::entrySet () {
   if (nitems == 0) return NULL;
   int idx=0;
   Object **res = new Object*[nitems+1];
   for (int i=0 ; i < 10000 ; i++) {
      if (table[i] == NULL) continue;
      for (HashCell *p=table[i] ; p != NULL ; p=p->next) {
			res[idx++] = p->object;
      }
   }
   res[idx] = NULL;
   return res;
}
   
int Hashtable::nItems () {
   return nitems;
}
   
void Hashtable::printStatistics () {
   printf("\nHashtable stats:\n---------------\n");
   for (int i=0 ; i < 10000 ; i++) {
      if (table[i] == NULL) continue;
      int count=0;
      for (HashCell *p=table[i] ; p != NULL ; p=p->next) count++;
      printf("(%d): %d\n", i, count);
   }
   printf("\n");
}

void HashTest (Hashtable *hash) {
   Object **e;
   printf("Hash->toString(): %s", hash->toString());
   printf(" nitems: %d", hash->nItems());
   printf(" Entries: ");
   if ((e = hash->entrySet()) == NULL) {
      printf("(none)\n");
   } else {
      for (Object **p=e ; *p != NULL ; p++) {
	 printf("%d ", ((Integer *)*p)->intValue());
      }
      printf("\n");
   }
}

