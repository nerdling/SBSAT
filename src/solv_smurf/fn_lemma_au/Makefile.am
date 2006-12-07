INCLUDES=-I$(top_srcdir)/include -I$(top_srcdir)/src/solv_smurf/include
EXTRA_DIST=bt_lemmas_au.cc lemmacache_au.cc lemmas_au.cc lemmawlits_au.cc
sources=fn_lemma_au.cc

#if MYLIBTOOL
#noinst_LTLIBRARIES = libfnlemmaau.la # libsolver2.la
#libfnlemmaau_la_SOURCES= $(sources)
#libsolver2_la_SOURCES=libsolver.cc
#else
noinst_LIBRARIES = libfnlemmaau.a # libsolver2.la
libfnlemmaau_a_SOURCES= $(sources)
#noinst_LIBRARIES = libsolver.a libsolver2.a
#libsolver_a_SOURCES= $(sources)
#libsolver2_a_SOURCES=libsolver.cc
#endif

