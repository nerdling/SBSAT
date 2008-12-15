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
#include "sbsat_solver.h"
#include "solver.h"

int use_RapidRestarts;

void (*Simple_initRestart)() = NULL;
long (*Simple_nextRestart)() = NULL;

//These routines modeled off of SAT4J code posted here:
//http://www.satcompetition.org/gorydetails/?p=3

//Routines for MiniSAT style restarts
long MiniSAT_RestartBound;

void MiniSAT_initRestart() {
	MiniSAT_RestartBound = 100;
}

long MiniSAT_nextRestart() {
	return MiniSAT_RestartBound *= 1.5;
}

//-----------------------------------


//Routines for PicoSAT style restarts

double PicoSAT_inner, PicoSAT_outer;
long PicoSAT_RestartBound;

void PicoSAT_initRestart() {
	PicoSAT_inner = 100;
	PicoSAT_outer = 100;
	PicoSAT_RestartBound = PicoSAT_inner;
}
	 
long PicoSAT_nextRestart() {
	if (PicoSAT_inner >= PicoSAT_outer) {
		PicoSAT_outer *= 1.1;
		PicoSAT_inner = 100;
	} else {
		PicoSAT_inner *= 1.1;
	}
	return PicoSAT_inner;
}

//-----------------------------------


//Routines for Luby style restarts

//taken from SATZ_rand source code
long Luby_factor;
long Luby_count;

long luby_super(long i) {
	long power;
	long k;
	
	assert (i > 0);
	/* let 2^k be the least power of 2 >= (i+1) */
	k = 1;
	power = 2;
	while (power < (i + 1)) {
		k += 1;
		power *= 2;
	}
	if (power == (i + 1))
	  return (power / 2);
	return (luby_super(i - (power / 2) + 1));
}

void Luby_initRestart() {
	Luby_factor = 512;//32;
	Luby_count = 1;
}

long Luby_nextRestart() {
	return luby_super(Luby_count++)*Luby_factor;
}
		 
//-----------------------------------
