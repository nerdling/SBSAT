INCLUDES=-I$(top_srcdir)/include -I$(top_srcdir)/src/solv_smurf/include
sources= lemmas.cc lemmaspace.cc lemmainfo.cc lemmawlits.cc lemmamisc.cc bt_lemmas.cc lemmacache.cc fn_lemma.cc
#$bdd2smurf.cc transitions.cc smurfstates.cc fn_smurf.cc bt_smurf.cc hr_lsgb_smurf_update.cc  hr_lsgb_smurf_init.cc 

#if MYLIBTOOL
#noinst_LTLIBRARIES = libfnlemma.la # libsolver2.la
#libfnlemma_la_SOURCES= $(sources)
#libsolver2_la_SOURCES=libsolver.cc
#else
noinst_LIBRARIES = libfnlemma.a # libsolver2.la
libfnlemma_a_SOURCES= $(sources)
#noinst_LIBRARIES = libsolver.a libsolver2.a
#libsolver_a_SOURCES= $(sources)
#libsolver2_a_SOURCES=libsolver.cc
#endif

