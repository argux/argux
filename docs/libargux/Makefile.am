EXTRA_DIST = libargux.conf 

docdir = $(datadir)/docs/libargux/

#doc_DATA = html/

if MAINTAINER_MODE

BUILT_SOURCES = html

html: libargux.conf
	doxygen libargux.conf

endif
