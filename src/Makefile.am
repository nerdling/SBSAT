INCLUDES=-I$(top_srcdir)/include
SOURCES=sbsat.cc
if MYLIBTOOL
SBSAT_LIBS= \
	libsbsat2.la \
	formats/libformats.la \
	solver/libsolver2.la  \
	postproc/libpostproc.la \
	preproc/libpreproc.la \
	utils/libutils.la 
else
SBSAT_LIBS= \
	libsbsat2.la \
	formats/libformats.a \
	formats/5/libt5.a \
	solver/libsolver2.a  \
	postproc/libpostproc.a \
	preproc/libpreproc.a \
	utils/libutils.a 
endif

lib_LTLIBRARIES = libsbsat2.la libsbsat.la
libsbsat2_la_SOURCES = $(SOURCES)
libsbsat_la_SOURCES = emptylib.cc #
libsbsat_la_LDFLAGS = -version-info 1:0:0 -module
libsbsat_la_LIBADD  = $(SBSAT_LIBS) $(LIBM)

bin_PROGRAMS = sbsat
sbsat_SOURCES = empty.cc
sbsat_LDADD =  $(SBSAT_LIBS) $(LIBM)
if SBSAT_STATIC
sbsat_LDFLAGS =  -all-static
else
sbsat_LDFLAGS =  -static
endif

SUBDIRS = solver utils preproc postproc formats generator
