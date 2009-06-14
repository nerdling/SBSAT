# -*- Autoconf -*- # Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
# watch out -- two places to change the version
AC_INIT([sbsat],[2.7b],[mkouril@ececs.uc.edu])
AC_LANG([C++])
AM_INIT_AUTOMAKE(sbsat,2.7b)
AC_CONFIG_SRCDIR([include/sbsat.h])
AC_CONFIG_HEADER([include/config.h])

saved_CXXFLAGS=${CXXFLAGS}
AC_PROG_CXX
CXXFLAGS=${saved_CXXFLAGS}


# compile with optimization and without debugging by default
MYCXXFLAGS=" -O2 -DNDEBUG "
MYCCFLAGS=" -DNDEBUG "
INCL=""

AC_ARG_ENABLE(optimization, 
[  --enable-optimization   Turn on compiler optimization (default=no)],
    [if eval "test x$enable_optimization = xyes"; then
        MYCXXFLAGS="-O3 -DNDEBUG -ffast-math -fno-strict-aliasing -funroll-all-loops -fno-peephole -fexpensive-optimizations -fomit-frame-pointer -finline-limit=10000"
	MYCCFLAGS="-xO5 -KPIC -dalign -native"
    fi])

AC_ARG_ENABLE(openmp,
[  --enable-openmp	Turn on openmp parallel segemts (default=no)],
    [if eval "test x$enable_optimization = xyes"; then
        MYCXXFLAGS="${MYCXXFLAGS} -fopenmp "
	MYCCFLAGS="${MYCCFLAGS} -fopenmp "
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
	MYCXXFLAGS="${MYCXXFLAGS} -pedantic -W -Winline -Wpointer-arith -Wcast-qual -Wcast-align -Wsign-compare -Wno-unused -Wconversion -Waggregate-return -Wshadow -Wwrite-strings -Wno-long-long"
    fi])

AC_ARG_ENABLE(coverage, 
[  --enable-coverage       Turn on compiler coverage information (default=no)],
    [if eval "test x$enable_coverage = xyes"; then
       if eval "test x$enable_profile = xyes"; then
	echo "error: can not turn on coverage and profiling"
	exit
       else
         MYCXXFLAGS="${MYCXXFLAGS}  -fprofile-arcs -ftest-coverage "
         MYCCFLAGS="${MYCCFLAGS}  -fprofile-arcs -ftest-coverage "
       fi
    fi])
 
AC_ARG_ENABLE(32bits,
[AS_HELP_STRING([--enable-32bits],[force build 32 bit version])],
[ 
   if test "x$enableval" = "xyes" ; then
        orig_ccflags=$CCFLAGS
        orig_cxxflags=$CXXFLAGS
        FORCED_BITS=yes
        CCFLAGS=-m32
        CXXFLAGS=-m32
        AC_COMPILE_IFELSE(
            [ AC_LANG_PROGRAM([], [dnl
                 return sizeof(void*) == 4 ? 0 : 1;
               ])
            ],[
                AC_MSG_CHECKING([force 32 bit target])
	        AC_MSG_RESULT(yes)
                AC_RUN_IFELSE(
                    [ AC_LANG_PROGRAM([], [dnl
                        return sizeof(void*) == 4 ? 0 : 1;
                    ])
                ],[
                    AC_MSG_CHECKING([32 bit pointer])
                    CCFLAGS="-m32 $orig_ccflags"
                    CXXFLAGS="-m32 $orig_cxxflags"
	            AC_MSG_RESULT(yes)
                ],[
                    AC_MSG_CHECKING([32 bit pointer])
                    CCFLAGS=$orig_ccflags;
                    CXXFLAGS=$orig_cxxflags;
	            AC_MSG_RESULT(no)
                ])
            ],[
               AC_MSG_CHECKING([32 bit compiler])
               CCFLAGS=$orig_ccflags;
               CXXFLAGS=$orig_cxxflags;
	       AC_MSG_RESULT(no)
            ])
        fi ])

AC_ARG_ENABLE(64bits,
[AS_HELP_STRING([--enable-64bits],[force build 64 bit version])],
[ 
   if test "x$enableval" = "xyes" ; then
        orig_ccflags=$CCFLAGS
        orig_cxxflags=$CXXFLAGS
        FORCED_BITS=yes
        CCFLAGS=-m64
        CXXFLAGS=-m64
        CFLAGS="-DSIZEOF_VOID_P=8 -DSIZEOF_LONG=8"
        AC_COMPILE_IFELSE(
            [ AC_LANG_PROGRAM([], [dnl
                 return sizeof(void*) == 8 ? 0 : 1;
               ])
            ],[
                AC_MSG_CHECKING([force 64 bit target])
	        AC_MSG_RESULT(yes)
                AC_RUN_IFELSE(
                    [ AC_LANG_PROGRAM([], [dnl
                        return sizeof(void*) == 8 ? 0 : 1;
                    ])
                ],[
                    AC_MSG_CHECKING([64 bit pointer])
                    CCFLAGS="-m64 $orig_ccflags"
                    CXXFLAGS="-m64 $orig_cxxflags"
                    CFLAGS="-DSIZEOF_VOID_P=8 -DSIZEOF_LONG=8"
	            AC_DEFINE(SBSAT_64BIT,true,[Do we have 64 bit enabled?])
	            AC_MSG_RESULT(yes)
                ],[
                    AC_MSG_CHECKING([64 bit pointer])
                    CCFLAGS=$orig_ccflags;
                    CXXFLAGS=$orig_cxxflags;
	            AC_MSG_RESULT(no)
                ])
            ],[
               AC_MSG_CHECKING([64 bit compiler])
               CCFLAGS=$orig_ccflags;
               CXXFLAGS=$orig_cxxflags;
	       AC_MSG_RESULT(no)
            ])
        fi ])

 
AC_MSG_CHECKING([autodetect 64 bit compiler ])
if eval "test x$FORCED_BITS = xyes"; then
        AC_MSG_RESULT(skip)
else
        AC_MSG_RESULT(go ahead)
        orig_ccflags=$CCFLAGS
        orig_cxxflags=$CXXFLAGS
        FORCED_BITS=yes
        CCFLAGS=-m64
        CXXFLAGS=-m64
        CFLAGS="-DSIZEOF_VOID_P=8 -DSIZEOF_LONG=8"
        AC_COMPILE_IFELSE(
            [ AC_LANG_PROGRAM([], [dnl
                 return sizeof(void*) == 8 ? 0 : 1;
               ])
            ],[
                AC_MSG_CHECKING([64 bit compiler])
	        AC_MSG_RESULT(yes)
                AC_RUN_IFELSE(
                    [ AC_LANG_PROGRAM([], [dnl
                        return sizeof(void*) == 8 ? 0 : 1;
                    ])
                ],[
                    AC_MSG_CHECKING([64 bit pointer])
                    CCFLAGS="-m64 $orig_ccflags"
                    CXXFLAGS="-m64 $orig_cxxflags"
                    CFLAGS="-DSIZEOF_VOID_P=8 -DSIZEOF_LONG=8"
	            AC_DEFINE(SBSAT_64BIT,true,[Do we have 64 bit enabled?])
	            AC_MSG_RESULT(yes)
                ],[
                    AC_MSG_CHECKING([64 bit pointer])
                    CCFLAGS=$orig_ccflags;
                    CXXFLAGS=$orig_cxxflags;
	            AC_MSG_RESULT(no)
                ])
            ],[
               AC_MSG_CHECKING([64 bit compiler])
               CCFLAGS=$orig_ccflags;
               CXXFLAGS=$orig_cxxflags;
	       AC_MSG_RESULT(no)
            ])
        fi

sbsat_static=false
AC_ARG_ENABLE(static, 
[  --enable-static         Turn on linking libraries staticly (default=no)],
    [sbsat_static=true])

AM_CONDITIONAL(SBSAT_STATIC, test x$sbsat_static = xtrue)

# Extra libraries 'with'-options
# AC_ARG_WITH(gd-inc,[  --with-gd-inc=DIR       location of the gd includes],
# [INCL="${INCL} -I${withval}"])
   
# AC_ARG_WITH(gd-lib,[  --with-gd-lib=DIR       location of the gd library],
# [LIBS="${LIBS} -L${withval}"])

CXXFLAGS="${CXXFLAGS} ${MYCXXFLAGS} ${INCL}"
CFLAGS="${CFLAGS} ${CXXFLAGS} -DHAVE_IEEE_754"
CCFLAGS="${CCFLAGS} ${MYCCFLAGS} ${INCL}"

cvsbuild=false
AC_ARG_ENABLE(cvsbuild,
[  --enable-cvsbuild Create nightly build],
[case "${enableval}" in
  no) cvsbuild=false ;;
  *) cvsbuild=${enableval} ;;
esac],[debug=false])
AM_CONDITIONAL(NO_CVSBUILD, test x$cvsbuild = xfalse)
CVSBUILD=$cvsbuild
AC_SUBST(CVSBUILD)

AC_ARG_WITH(java-os,
[  --with-java-os=OS       Use OS to include machine-dependent java header],[
  if test "$withval" != "no" ; then
    if test "$withval" != "yes"; then
      JAVA_OS_INCL="$withval"
    else
      AC_MSG_ERROR([Missing argument for --with-java-os])
   fi
  fi
])

AC_ARG_WITH(java,
[  --with-java[[=DIR]]       Use java to create shared object use JDK DIR],[
  if test "$withval" != "no" ; then
    if test "$withval" != "yes"; then
      JAVA_DIR=$withval
      CPPFLAGS="${CPPFLAGS} -I$withval/include"
      LDFLAGS="${LDFLAGS} -L$withval/lib"
    else
      AC_CHECK_FILE(
         [$JAVA_HOME/include/jni.h],[
            JAVA_DIR="$JAVA_HOME"
            CPPFLAGS="${CPPFLAGS} -I$JAVA_DIR/include"
            LDFLAGS="${LDFLAGS} -L$JAVA_DIR/lib"
         ],[])
   fi
   if test x$JAVA_OS_INCL != "x" ; then
      JAVA_OS_INCL=" -I$JAVA_DIR/include/$JAVA_OS_INCL "
   fi
   CPPFLAGS="${CPPFLAGS} ${JAVA_OS_INCL}";
   AC_CHECK_HEADERS(jni_md.h, [], 
        AC_MSG_ERROR(
            [Could not find jni_md.h include file please check JAVA_HOME enviroment variable or specify --with-java=DIR and specify --with-java-os=OS or check config.log.]))
   AC_CHECK_HEADERS(jni.h,
      [], AC_MSG_ERROR(
            [Could not find jni.h include file please check JAVA_HOME enviroment variable or specify --with-java=DIR or check config.log.]))
 fi
], [])

#AC_ARG_WITH(zlib,
#[  --with-zlib[[=DIR]]       use libz in DIR],[
  #if test "$withval" != "no" -a "$withval" != "yes"; then
    #Z_DIR=$withval
    #CPPFLAGS="${CPPFLAGS} -I$withval/include"
    #LDFLAGS="${LDFLAGS} -L$withval/lib"
  #fi
#])
#if test "$with_zlib" = "no"; then
    #echo "Disabling compression support"
#else
    #AC_CHECK_HEADERS(zlib.h,
        #AC_CHECK_LIB(z, gzread,[
            #AC_DEFINE(HAVE_LIBZ)
            #if test "x${Z_DIR}" != "x"; then
                #Z_CFLAGS="-I${Z_DIR}/include"
                #Z_LIBS="-L${Z_DIR}/lib -lz"
                #[case ${host} in
                    #*-*-solaris*)
                        #Z_LIBS="-L${Z_DIR}/lib -R${Z_DIR}/lib -lz"
                        #;;
                #esac]
            #else
                #Z_LIBS="-lz"
            #fi]))
#fi
#
#AC_SUBST(Z_CFLAGS)
#AC_SUBST(Z_LIBS)
AC_ARG_ENABLE(alllibtool,
[  --enable-alllibtool    Turn on libtool for the whole tree],
[case "${enableval}" in
  yes) alllibtool=true ;;
  no)  alllibtooldebug=false ;;
  *) AC_MSG_ERROR(bad value ${enableval} for --enable-alllibtool) ;;
esac],[alllibtool=true])
AM_CONDITIONAL(MYLIBTOOL, test x$alllibtool = xtrue)

# Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AC_PROG_RANLIB # obsolete by LIBTOOL
AM_PROG_LEX
AC_PROG_YACC("bison -y")
# AC_PROG_LIBTOOL

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
AC_CHECK_HEADERS([ncurses/termcap.h])
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
AC_CHECK_FUNCS([popen getpagesize gettimeofday memmove memset munmap select sqrt strcasecmp strchr strncasecmp strrchr])
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
                 doc/man1/Makefile
                 examples/Makefile
                 examples/example_tests.sh
                 include/Makefile
                 src/Makefile
                 src/cudd/Makefile
                 src/cudd/cudd/Makefile
                 src/cudd/epd/Makefile
                 src/cudd/include/Makefile
                 src/cudd/mtr/Makefile
                 src/cudd/st/Makefile
                 src/cudd/util/Makefile
                 src/cudd/cppobj/Makefile
                 src/formats/5/Makefile
                 src/formats/Makefile
                 src/generator/Makefile
                 src/postproc/Makefile
                 src/preproc/Makefile
                 src/solv_smurf/Makefile
                 src/solv_smurf/include/Makefile
                 src/solv_smurf/fn_xor/Makefile
                 src/solv_smurf/fn_xor_smurf/Makefile
                 src/solv_smurf/fn_minmax/Makefile
                 src/solv_smurf/fn_and/Makefile
                 src/solv_smurf/fn_lemma_au/Makefile
                 src/solv_smurf/fn_lemma/Makefile
                 src/solv_smurf/fn_smurf_au/Makefile
                 src/solv_smurf/fn_smurf_xor/Makefile
                 src/solv_smurf/fn_smurf/Makefile
                 src/solv_smurf/hr_null/Makefile
                 src/solv_smurf/hr_lsgb_lemma/Makefile
                 src/solv_smurf/hr_lsgbw/Makefile
                 src/solv_smurf/hr_lsgb/Makefile
                 src/solv_smurf/hr_lemma/Makefile
                 src/solv_smurf/hr_vsids/Makefile
                 src/solv_smurf/hr_berkmin/Makefile
                 src/solv_smurf/hr_anne/Makefile
                 src/solv_smurf/hr_minisat/Makefile
                 src/solv_bddwalk/Makefile
                 src/solv_wvf/Makefile
					  src/solv_simple/Makefile
  					  src/solv_simple/include/Makefile
					  src/solv_simple/fn_smurf/Makefile
                 src/solv_simple/fn_inference/Makefile
					  src/solv_simple/fn_or/Makefile
					  src/solv_simple/fn_xor/Makefile
					  src/solv_simple/fn_minmax/Makefile
                 src/solv_simple/fn_negminmax/Makefile
  					  src/solv_simple/bb_gelim/Makefile
					  src/solv_simple/bb_lemmas/Makefile
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
