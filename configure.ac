#                                               -*- Autoconf -*- # Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT([sbsat],[2.0r-1],[mkouril@ececs.uc.edu])
AC_LANG([C++])
AM_INIT_AUTOMAKE(sbsat,2.0r-1)
AC_CONFIG_SRCDIR([include/brancher.h])
AC_CONFIG_HEADER([include/config.h])

# compile with optimization and without debugging by default
MYCXXFLAGS=" -O2 -DNDEBUG "
MYCCFLAGS=" -DNDEBUG "
INCL=""

AC_ARG_ENABLE(optimization, 
[  --enable-optimization   Turn on compiler optimization (default=no)],
    [if eval "test x$enable_optimization = xyes"; then
        MYCXXFLAGS="-O3 -DNDEBUG -ffast-math -fstrict-aliasing -funroll-all-loops -fno-peephole -fexpensive-optimizations -fomit-frame-pointer "
	MYCCFLAGS="-xO5 -KPIC -dalign -native"
    fi])

AC_ARG_ENABLE(debug, 
[  --enable-debug          Turn on compiler debugging information (default=no)],
    [if eval "test x$enable_debug = xyes"; then
       if eval "test x$enable_optimization = xyes"; then
	echo "error: can not turn on debuging and extra optimization"
	exit
       else
         MYCXXFLAGS=" -g -Wall "
         MYCCFLAGS=" -g -Wall "
       fi
    fi])

AC_ARG_ENABLE(profile, 
[  --enable-profile        Turn on compiler profiling information (default=no)],
    [if eval "test x$enable_profile = xyes"; then
       if eval "test x$enable_optimization = xyes"; then
	echo "error: can not turn on profiling and extra optimization"
	exit
       else
         MYCXXFLAGS="${MYCXXFLAGS}  -pg -Wall "
         MYCCFLAGS="${MYCCFLAGS}  -pg -Wall "
       fi
    fi])

AC_ARG_ENABLE(extrawarning, 
[  --enable-extrawarning   Turn on compiler extra warning information (default=no)],
    [if eval "test x$enable_extrawarning = xyes"; then
	MYCXXFLAGS="${MYCXXFLAGS} -pedantic -W -Winline -Wmissing-prototypes -Wpointer-arith -Wcast-qual -Wcast-align -Wsign-compare -Wno-unused -Wconversion -Waggregate-return -Wshadow -Wwrite-strings "
    fi])

AC_ARG_ENABLE(static, 
[  --enable-static         Turn on linking libraries staticly (default=no)],
    [if eval "test x$enable_static = xyes"; then
        MYCXXFLAGS="${MYCXXFLAGS} -static"
        MYCCFLAGS="${MYCCFLAGS} -static"
    fi])

# Extra libraries 'with'-options
# AC_ARG_WITH(gd-inc,[  --with-gd-inc=DIR       location of the gd includes],
# [INCL="${INCL} -I${withval}"])
   
# AC_ARG_WITH(gd-lib,[  --with-gd-lib=DIR       location of the gd library],
# [LIBS="${LIBS} -L${withval}"])

CXXFLAGS="${CXXFLAGS} ${MYCXXFLAGS} ${INCL}"
CCFLAGS="${CCFLAGS} ${MYCCFLAGS} ${INCL}"

# Checks for programs.
AC_PROG_AWK
AC_PROG_CXX
#AC_PROG_CC
AC_PROG_RANLIB
AM_PROG_LEX
AC_PROG_YACC

AC_SUBST(ECHO_N)
AC_SUBST(ECHO_C)

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h inttypes.h limits.h memory.h stddef.h stdint.h stdlib.h string.h strings.h sys/param.h sys/time.h termios.h unistd.h])

# Also check for these headers
AC_CHECK_HEADERS([string])
AC_CHECK_HEADERS([signal.h assert.h])
AC_CHECK_HEADERS([iostream.h fstream.h iostream fstream ])
AC_CHECK_HEADERS([sys/types.h sys/times.h sys/resource.h sys/wait.h])
AC_CHECK_HEADERS([sys/stat.h ctype.h math.h regex.h])
AC_CHECK_HEADERS([termcap.h])


# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
# AC_FUNC_MALLOC
AC_FUNC_MMAP
# AC_FUNC_REALLOC
AC_FUNC_SELECT_ARGTYPES
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_CHECK_FUNCS([getpagesize gettimeofday memmove memset munmap select sqrt strcasecmp strchr strncasecmp strrchr])
AC_CHECK_LIBM
AC_SUBST(LIBM)

## Does the compiler support the "std" namespace?
## GCC version 3 not only supports this but insists on it, 
## so if this feature is supported, we always use it. 
AC_MSG_CHECKING([for support for C++ "std" namespace])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#ifdef HAVE_STRING
  #include <string>
  #endif
]], [[std::string s;]])],[AC_DEFINE(HAVE_STD_NAMESPACE,, Have std namespace support)
	AC_DEFINE(STD_NAMESPACE,std::,Namespace std)
	AC_MSG_RESULT(yes)],[AC_MSG_RESULT(no)])

## Does the compiler support the "using namespace std" namespace?
## GCC version 3 not only supports this but insists on it, 
## so if this feature is supported, we always use it. 
AC_MSG_CHECKING([for support for C++ "using namespace std"])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[using namespace std;]], [[]])],[AC_DEFINE(HAVE_USING_NAMESPACE_STD,, Have using namespace std support)
	AC_DEFINE_UNQUOTED(HAVE_USING_NAMESPACE_STD)
	AC_MSG_RESULT(yes)],[AC_MSG_RESULT(no)])


AC_CHECK_LIB(termcap,tgetnum,[
	AC_DEFINE([HAVE_TERMCAP_LIB], [], [Have termcap support])
	LIBS="$LIBS -ltermcap"	
])



AC_CONFIG_FILES([Makefile
                 doc/Makefile
                 doc/manual/Makefile
                 examples/Makefile
                 include/Makefile
                 src/Makefile
                 src/formats/5/Makefile
                 src/formats/Makefile
                 src/generator/Makefile
                 src/postproc/Makefile
                 src/preproc/Makefile
                 src/solver/Makefile
                 src/utils/Makefile
                 tests/Makefile
                 tests/cnf/Makefile
                 tests/longer_tests/Makefile
                 tests/trace_tests/Makefile
                 tests/trace_tests.sh 
                 tests/xor_tests.sh 
                 tests/longer_tests.sh 
                 tests/cnf_tests.sh])

AC_OUTPUT
