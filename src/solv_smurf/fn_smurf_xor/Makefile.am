INCLUDES=-I$(top_srcdir)/include -I$(top_srcdir)/src/solv_smurf/include
sources = fn_smurf_xor.cc \
	hr_lsgb_smurfxor_init.cc hr_lsgb_smurfxor_update.cc 

#hr_lsgbw_smurfxor_init.cc hr_lsgbw_smurfxor_update.cc 

#if MYLIBTOOL
#noinst_LTLIBRARIES = libfnsmurfxor.la # libsolver2.la
#libfnsmurfxor_la_SOURCES= $(sources)
#libsolver2_la_SOURCES=libsolver.cc
#else
noinst_LIBRARIES = libfnsmurfxor.a # libsolver2.la
libfnsmurfxor_a_SOURCES= $(sources)
#noinst_LIBRARIES = libsolver.a libsolver2.a
#libsolver_a_SOURCES= $(sources)
#libsolver2_a_SOURCES=libsolver.cc
#endif

