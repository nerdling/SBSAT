INCLUDES=-I$(top_srcdir)/include -I$(top_srcdir)/src/solv_smurf/include

sources= bdd2specfn_xor.cc bt_specfn_xor.cc fn_xor.cc lemmas_xor.cc \
	 hr_lsgb_xor.cc hr_lsgbw_xor.cc

#if MYLIBTOOL
#noinst_LTLIBRARIES = libfnxor.la # libsolver2.la
#libfnxor_la_SOURCES= $(sources)
#libsolver2_la_SOURCES=libsolver.cc
#else
noinst_LIBRARIES = libfnxor.a # libsolver2.la
libfnxor_a_SOURCES= $(sources)
#noinst_LIBRARIES = libsolver.a libsolver2.a
#libsolver_a_SOURCES= $(sources)
#libsolver2_a_SOURCES=libsolver.cc
#endif
