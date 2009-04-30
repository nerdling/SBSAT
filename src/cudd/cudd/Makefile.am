INCLUDES=-I$(top_srcdir)/include -I$(top_srcdir)/src/cudd/include
sources= \
         cuddAPI.c cuddAddAbs.c cuddAddApply.c cuddAddFind.c cuddAddIte.c \
         cuddAddInv.c cuddAddNeg.c cuddAddWalsh.c cuddAndAbs.c \
         cuddAnneal.c cuddApa.c cuddApprox.c cuddBddAbs.c cuddBddCorr.c \
         cuddBddIte.c cuddBridge.c cuddCache.c cuddCheck.c cuddClip.c \
         cuddCof.c cuddCompose.c cuddDecomp.c cuddEssent.c \
         cuddExact.c cuddExport.c cuddGenCof.c cuddGenetic.c \
         cuddGroup.c cuddHarwell.c cuddInit.c cuddInteract.c \
         cuddLCache.c cuddLevelQ.c \
         cuddLinear.c cuddLiteral.c cuddMatMult.c cuddPriority.c \
         cuddRead.c cuddRef.c cuddReorder.c cuddSat.c cuddSign.c \
         cuddSolve.c cuddSplit.c cuddSubsetHB.c cuddSubsetSP.c cuddSymmetry.c \
         cuddTable.c cuddUtil.c cuddWindow.c cuddZddCount.c cuddZddFuncs.c \
         cuddZddGroup.c cuddZddIsop.c cuddZddLin.c cuddZddMisc.c \
         cuddZddPort.c cuddZddReord.c cuddZddSetop.c cuddZddSymm.c \
         cuddZddUtil.c

#if MYLIBTOOL
#noinst_LTLIBRARIES = libutils.la
#libutils_la_SOURCES= $(sources)
#else
noinst_LIBRARIES = libcudd.a
libcudd_a_SOURCES= $(sources)
#endif
