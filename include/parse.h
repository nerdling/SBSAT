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
 *  parse.h (J. Franco)
 *  Functions used by flatten.cc and tracer.cc to parse
 *  input lines.
 *********************************************************/
#ifndef PARSE_H
#define PARSE_H

int indexOf(char *, char);
int indexFrom(char *, char *, char);

class Trimmer {
//	char *trim;
   char trim[16000];
	
 public:
   //Trimmer () { trim = new char[1]; }
   //~Trimmer () { delete trim; }
	
   char *get (char *);
};

class Substring {
//	char *str;
   char str[16000];

 public:
   //Substring () { str = new char[1]; }
   //~Substring () { delete str; }

   char *get(char *, int, int);
};

class StringTokenizer {
//	char *str, *delim, *ptr, *eptr;
   char str[16000], delim[128], *ptr, *eptr;
   bool more_tokens;
	
 public:
   StringTokenizer (char *, char *);
   ~StringTokenizer () {
      //delete str;
      //delete delim;
   }
   void renewTokenizer (char *, char *);
   char *nextToken ();
   int countTokens ();
   char *prevToken ();
   bool hasMoreTokens ();
   char *remainder () { return ptr; }
};

class Object {
//char *return_buf = new char[1024];
   char return_buf[1024];

 public:
   char *toString ();
	virtual ~Object(){};
};

class Integer : public Object {
   int number;
   char *hash_ident;

 public:
   Integer (int, char *);
   ~Integer ();
   int intValue ();
   char *getId ();
};

class Node;

class Cell : public Object {
   Node *node;
   Cell *next;

 public:
   Cell (Node *, Cell *);
	~Cell();
   Node *getNode ();
   Cell *getNext ();
	void setNext (Cell *);
	void setNode (Node *);
};

class Node : public Object {
   char *id;
   bool marked;
   Cell *next;
   Node *up;

 public:
   Node (char *);
	~Node();
   bool getMark ();
   char *getId ();
   Cell *getNext ();
   Node *getUp ();
   void setMark ();
   void setNext (Cell *);
   void setUp (Node *);
};

// ++ ++ ++ ++ ++ ++ ++ ++ ++ ++ ++ ++ ++ ++ ++ ++ ++ ++ ++ ++ ++ ++

class HashCell {
friend class Hashtable;

   Object *object;
   char *ident;
   HashCell *next;
	
 public:
   HashCell (Object *, char *, HashCell *);
   ~HashCell () {
		delete [] ident;
		delete object;
   }
};

class Hashtable : public Object {
   HashCell **table;
   int nitems;
   
   int hash_func (char *);
	
 public:
   Hashtable ();
   ~Hashtable ();
   void put (char *, Object *);
   Object *get (char *);
   Object **entrySet ();
   int nItems ();
   void printStatistics ();
};

void HashTest (Hashtable *);

#endif
