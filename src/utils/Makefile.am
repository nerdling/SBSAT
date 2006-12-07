INCLUDES=-I$(top_srcdir)/include
sources= \
             bdd.cc bdd_ite.cc bdd_print.cc utils.cc \
             params.cc params_fn.cc log.cc itetable.cc \
             bddtable.cc zread.cc vars.cc infer.cc llist.cc \
	     bdd_circuit_init.cc symtable.cc java_wrapper.cc \
	     custprint.cc termcap.cc 
#EXTRA_DIST = # bddtable.cc

#if MYLIBTOOL
#noinst_LTLIBRARIES = libutils.la
#libutils_la_SOURCES= $(sources)
#else
noinst_LIBRARIES = libutils.a
libutils_a_SOURCES= $(sources)
#endif
