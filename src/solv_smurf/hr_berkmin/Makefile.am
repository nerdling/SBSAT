INCLUDES=-I$(top_srcdir)/include -I$(top_srcdir)/src/solv_smurf/include
sources=  hr_berkmin.cc hr_berkmin_update.cc
EXTRA_DIST=hr_choice.cc

#if MYLIBTOOL
#noinst_LTLIBRARIES = libhrberkmin.la # libsolver2.la
#libhrberkmin_la_SOURCES= $(sources)
#libsolver2_la_SOURCES=libsolver.cc
#else
noinst_LIBRARIES = libhrberkmin.a # libsolver2.la
libhrberkmin_a_SOURCES= $(sources)
#noinst_LIBRARIES = libsolver.a libsolver2.a
#libsolver_a_SOURCES= $(sources)
#libsolver2_a_SOURCES=libsolver.cc
#endif

