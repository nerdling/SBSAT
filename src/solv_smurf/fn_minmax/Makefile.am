INCLUDES=-I$(top_srcdir)/include -I$(top_srcdir)/src/solv_smurf/include

sources= bdd2specfn_minmax.cc bt_specfn_minmax.cc fn_minmax.cc \
	 hr_lsgb_minmax.cc 

#if MYLIBTOOL
#noinst_LTLIBRARIES = libfnminmax.la # libsolver2.la
#libfnminmax_la_SOURCES= $(sources)
#libsolver2_la_SOURCES=libsolver.cc
#else
noinst_LIBRARIES = libfnminmax.a # libsolver2.la
libfnminmax_a_SOURCES= $(sources)
#noinst_LIBRARIES = libsolver.a libsolver2.a
#libsolver_a_SOURCES= $(sources)
#libsolver2_a_SOURCES=libsolver.cc
#endif
