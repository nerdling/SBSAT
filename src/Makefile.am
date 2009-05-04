SUBDIRS = cudd solv_smurf solv_bddwalk solv_wvf solv_simple utils preproc postproc formats generator

INCLUDES=-I$(top_srcdir)/include
sources=sbsat.cc
SBSAT_LIBS= \
   formats/libformats.a \
	formats/5/libt5.a \
	solv_smurf/libsolv_smurf.a \
	solv_smurf/hr_null/libhrnull.a \
	solv_smurf/hr_lsgb_lemma/libhrlsgblemma.a \
	solv_smurf/hr_vsids/libhrvsids.a \
	solv_smurf/hr_berkmin/libhrberkmin.a \
	solv_smurf/hr_anne/libhranne.a \
	solv_smurf/hr_minisat/libhrminisat.a \
	solv_smurf/hr_lsgbw/libhrlsgbw.a \
	solv_smurf/hr_lsgb/libhrlsgb.a \
	solv_smurf/hr_lemma/libhrlemma.a \
	solv_smurf/fn_xor_smurf/libfnxorsmurf.a \
	solv_smurf/fn_xor/libfnxor.a \
	solv_smurf/fn_minmax/libfnminmax.a \
	solv_smurf/fn_and/libfnand.a \
	solv_smurf/fn_smurf_xor/libfnsmurfxor.a \
	solv_smurf/fn_smurf_au/libfnsmurfau.a \
	solv_smurf/fn_smurf/libfnsmurf.a \
	solv_smurf/fn_lemma/libfnlemma.a \
	solv_bddwalk/libsolv_bddwalk.a \
	solv_wvf/libsolv_wvf.a \
	solv_simple/libsolv_simple.a \
	solv_simple/fn_smurf/libfnsmurf_simple.a \
   solv_simple/fn_inference/libfninference_simple.a \
	solv_simple/fn_or/libfnor_simple.a \
	solv_simple/fn_xor/libfnxor_simple.a \
	solv_simple/fn_minmax/libfnminmax_simple.a \
  	solv_simple/fn_negminmax/libfnnegminmax_simple.a \
	solv_simple/bb_gelim/libbbgelim_simple.a \
	solv_simple/bb_lemmas/libbblemmas_simple.a \
	postproc/libpostproc.a \
	preproc/libpreproc.a \
   utils/libutils.a \
   cudd/cppobj/libcuddobj.a \
   cudd/cudd/libcudd.a \
   cudd/epd/libepd.a \
   cudd/mtr/libmtr.a \
   cudd/st/libst.a \
   cudd/util/libcuddutil.a

bin_PROGRAMS = sbsat
sbsat_SOURCES = sbsat.cc
sbsat_LDADD =  $(SBSAT_LIBS) $(LIBM)
if SBSAT_STATIC
sbsat_LDFLAGS =  -all-static
else
#sbsat_LDFLAGS =  -static
sbsat_LDFLAGS = 
endif

