INCLUDES=-I$(top_srcdir)/include
SOURCES= \
             integerset.cc literalset.cc bdd.cc utils.cc \
             params.cc params_fn.cc log.cc itetable.cc \
             bddtable.cc zread.cc vars.cc infer.cc intnode.cc \
	     bdd_circuit_init.cc symtable.cc java_wrapper.cc \
	     custprint.cc termcap.cc # malloc.cc

noinst_LTLIBRARIES = libutils.la
libutils_la_SOURCES= $(SOURCES)

EXTRA_DIST = malloc.cc # bddtable.cc
