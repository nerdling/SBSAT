INCLUDES=-I$(top_srcdir)/include -I$(top_srcdir)/src/solv_smurf/include
sources= hr_lsgbw.cc hr_lsgbw_init.cc

#if MYLIBTOOL
#noinst_LTLIBRARIES = libhrlsgbw.la # libsolver2.la
#libhrlsgbw_la_SOURCES= $(sources)
#libsolver2_la_SOURCES=libsolver.cc
#else
noinst_LIBRARIES = libhrlsgbw.a # libsolver2.la
libhrlsgbw_a_SOURCES= $(sources)
#noinst_LIBRARIES = libsolver.a libsolver2.a
#libsolver_a_SOURCES= $(sources)
#libsolver2_a_SOURCES=libsolver.cc
#endif

