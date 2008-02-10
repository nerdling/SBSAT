INCLUDES=-I$(top_srcdir)/include -I$(top_srcdir)/src/solv_simple/include

sources= fn_smurf.cc bdd2smurf.cc bt_smurf.cc hr_lsgb_smurf_init.cc \
         hr_lsgb_smurf_update.cc

#if MYLIBTOOL
#noinst_LTLIBRARIES = libfnand.la # libsolver2.la
#libfnand_la_SOURCES= $(sources)
#libsolver2_la_SOURCES=libsolver.cc
#else
noinst_LIBRARIES = libfnsmurf_simple.a # libsolver2.la
libfnsmurf_simple_a_SOURCES= $(sources)
#noinst_LIBRARIES = libsolver.a libsolver2.a
#libsolver_a_SOURCES= $(sources)
#libsolver2_a_SOURCES=libsolver.cc
#endif
