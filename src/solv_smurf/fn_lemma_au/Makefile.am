INCLUDES=-I$(top_srcdir)/include -I$(top_srcdir)/src/solv_smurf/include
EXTRA_DIST=bt_lemmas_au.cc fn_lemma_au.cc lemmacache_au.cc lemmas_au.cc lemmawlits_au.cc

if MYLIBTOOL
noinst_LTLIBRARIES = libfnlemmaau.la # libsolver2.la
libfnlemmaau_la_SOURCES= $(SOURCES)
#libsolver2_la_SOURCES=libsolver.cc
else
#noinst_LIBRARIES = libsolver.a libsolver2.a
#libsolver_a_SOURCES= $(SOURCES)
#libsolver2_a_SOURCES=libsolver.cc
endif

