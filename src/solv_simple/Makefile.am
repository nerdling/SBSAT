SUBDIRS= include fn_smurf fn_or fn_xor

INCLUDES=-I$(top_srcdir)/include -I$(top_srcdir)/src/solv_simple/include
sources= solv_simple.cc solv_simple_init.cc vars_init.cc vars_map.cc \
display.cc

#if MYLIBTOOL
#noinst_LTLIBRARIES = libsolv_simple.la
#libsolv_simple_la_SOURCES= $(sources)
#else
noinst_LIBRARIES = libsolv_simple.a
libsolv_simple_a_SOURCES= $(sources)
#endif
