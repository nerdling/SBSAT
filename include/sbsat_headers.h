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

#ifndef SBSAT_HEADERS_H
#define SBSAT_HEADERS_H

#if HAVE_CONFIG_H
#include "config.h"
#endif


# include <stdio.h>

#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#if HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#if HAVE_SYS_TIME_H
# include <sys/time.h>
#endif
#if HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif
#if STDC_HEADERS
# include <stdlib.h>
# include <stddef.h>
#else
# if HAVE_STDLIB_H
#  include <stdlib.h>
# endif
#endif
#if HAVE_STRING_H
# if !STDC_HEADERS && HAVE_MEMORY_H
#  include <memory.h>
# endif
# include <string.h>
#endif
#if HAVE_STRINGS_H
# include <strings.h>
#endif
#if HAVE_INTTYPES_H
# include <inttypes.h>
#else
# if HAVE_STDINT_H
#  include <stdint.h>
# endif
#endif
#if HAVE_UNISTD_H
# include <unistd.h>
#endif
#if HAVE_CTYPE_H
# include <ctype.h>
#endif
#if HAVE_LIMITS_H
# include <limits.h>
#endif
#if HAVE_ASSERT_H
# include <assert.h>
#endif
#if HAVE_SIGNAL_H
# include <signal.h>
#endif
#if HAVE_MATH_H
# include <math.h>
#endif
#if HAVE_REGEX_H
# include <regex.h>
#endif
#if HAVE_NCURSES_TERMCAP_H
# include <ncurses/termcap.h>
#elif HAVE_TERMCAP_H
# include <termcap.h>
#endif
#if HAVE_TERMIOS_H
# include <termios.h>
#endif
#if HAVE_IOSTREAM
# include <iostream>
#else
# if HAVE_IOSTREAM_H
#  include <iostream.h>
# endif
#endif
#if HAVE_FSTREAM
# include <fstream>
#else
# if HAVE_FSTREAM_H
#  include <fstream.h>
# endif
#endif

#ifndef INT_MAX
#define INT_MAX 2147483647
#endif

/* system */
#ifdef HAVE_USING_NAMESPACE_STD
using namespace std;
#endif

#endif
