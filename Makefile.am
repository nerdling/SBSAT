EXTRA_DIST = BUGS compile.patch  compile.patch.all.sh  compile.patch.sh 
# ChangeLog.O README-alpha
SUBDIRS = include src tests examples doc
# doc intl po src tests

if NO_CVSBUILD
PKG_DISTDIR=$(PACKAGE)-$(PACKAGE_VERSION)
else
PKG_DISTDIR=$(PACKAGE)-$(CVSBUILD)
endif

distdir=$(PKG_DISTDIR)

DISTCLEANFILES = confdefs.h

