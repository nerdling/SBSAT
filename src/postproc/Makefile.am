INCLUDES=-I$(top_srcdir)/include
SOURCES= backend.cc

if MYLIBTOOL
noinst_LTLIBRARIES = libpostproc.la
libpostproc_la_SOURCES= $(SOURCES)
else
noinst_LIBRARIES = libpostproc.a
libpostproc_a_SOURCES= $(SOURCES)
endif
