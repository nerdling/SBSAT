SUBDIRS = solv_smurf solv_bddwalk solv_wvf utils preproc postproc formats generator

INCLUDES=-I$(top_srcdir)/include
SOURCES=sbsat.cc
if MYLIBTOOL
SBSAT_LIBS= \
	libsbsat2.la \
	formats/libformats.la \
	solv_smurf/libsolv_smurf.la  \
	solv_smurf/hr_null/libhrnull.la  \
	solv_smurf/hr_lsgb_lemma/libhrlsgblemma.la  \
	solv_smurf/hr_lsgbw/libhrlsgbw.la  \
	solv_smurf/hr_lsgb/libhrlsgb.la  \
	solv_smurf/hr_lemma/libhrlemma.la  \
	solv_smurf/fn_xor/libfnxor.la  \
	solv_smurf/fn_minmax/libfnminmax.la  \
	solv_smurf/fn_and/libfnand.la  \
	solv_smurf/fn_smurf_au/libfnsmurfau.la  \
	solv_smurf/fn_smurf/libfnsmurf.la  \
	solv_smurf/fn_lemma/libfnlemma.la  \
	solv_bddwalk/libsolv_bddwalk.la  \
	solv_wvf/libsolv_wvf.la  \
	postproc/libpostproc.la \
	preproc/libpreproc.la \
	utils/libutils.la 
else
SBSAT_LIBS= \
	libsbsat2.la \
	formats/libformats.a \
	formats/5/libt5.a \
	solv_smurf/libsolv_smurf.a  \
	solv_smurf/hr_null/libhrnull.a  \
	solv_smurf/hr_lsgb_lemma/libhrlsgblemma.a  \
	solv_smurf/hr_lsgbw/libhrlsgbw.a  \
	solv_smurf/hr_lsgb/libhrlsgb.a  \
	solv_smurf/hr_lemma/libhrlemma.a  \
	solv_smurf/fn_xor/libfnxor.a  \
	solv_smurf/fn_minmax/libfnminmax.a  \
	solv_smurf/fn_and/libfnand.a  \
	solv_smurf/fn_smurf_au/libfnsmurfau.a  \
	solv_smurf/fn_smurf/libfnsmurf.a  \
	solv_smurf/fn_lemma/libfnlemma.a  \
	solv_bddwalk/libsolv_bddwalk.a  \
	solv_wvf/libsolv_wvf.a  \
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

