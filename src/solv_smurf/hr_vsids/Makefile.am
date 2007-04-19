INCLUDES=-I$(top_srcdir)/include -I$(top_srcdir)/src/solv_smurf/include
sources=  hr_vsids.cc hr_vsids_update.cc
EXTRA_DIST=hr_choice.cc

#if MYLIBTOOL
#noinst_LTLIBRARIES = libhrvsids.la # libsolver2.la
#libhrvsids_la_SOURCES= $(sources)
#libsolver2_la_SOURCES=libsolver.cc
#else
noinst_LIBRARIES = libhrvsids.a # libsolver2.la
libhrvsids_a_SOURCES= $(sources)
#noinst_LIBRARIES = libsolver.a libsolver2.a
#libsolver_a_SOURCES= $(sources)
#libsolver2_a_SOURCES=libsolver.cc
#endif

