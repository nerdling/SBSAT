INCLUDES=-I$(top_srcdir)/include
SOURCES= \
             bdd.cc utils.cc \
             params.cc params_fn.cc log.cc itetable.cc \
             bddtable.cc zread.cc vars.cc infer.cc \
	     bdd_circuit_init.cc symtable.cc java_wrapper.cc \
	     custprint.cc termcap.cc 
#EXTRA_DIST = # bddtable.cc

if MYLIBTOOL
noinst_LTLIBRARIES = libutils.la
libutils_la_SOURCES= $(SOURCES)
else
#noinst_LIBRARIES = libutils.a
#libutils_a_SOURCES= $(SOURCES)
endif
