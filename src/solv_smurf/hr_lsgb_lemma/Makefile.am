INCLUDES=-I$(top_srcdir)/include -I$(top_srcdir)/src/solv_smurf/include
SOURCES=  hr_lsgb_lemma.cc
EXTRA_DIST=hr_choice.cc

if MYLIBTOOL
noinst_LTLIBRARIES = libhrlsgblemma.la # libsolver2.la
libhrlsgblemma_la_SOURCES= $(SOURCES)
#libsolver2_la_SOURCES=libsolver.cc
else
#noinst_LIBRARIES = libsolver.a libsolver2.a
#libsolver_a_SOURCES= $(SOURCES)
#libsolver2_a_SOURCES=libsolver.cc
endif

