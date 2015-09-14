
lib_LTLIBRARIES = \
    libargux-1.0.la

libargux_includedir = \
    $(includedir)/argux-$(LIBARGUX_VERSION_API)/argux

libargux_1_0_la_SOURCES = \
    memory.c \
    assert.c \
    item.c \
    value.c \
    slist.c \
    log.c \
    error.c \
    plugin.c \
    plugin-db.c \
    ns.c \
    host.c \
    metric.c \
    rest-server.c \
    principal.c \
    libargux.c

libargux_include_HEADERS = \
    memory.h \
    assert.h \
    log.h \
    slist.h \
    types.h \
    error.h \
    item.h \
    value.h \
    itemtype.h \
    plugin.h \
    plugin-db.h \
    ns.h \
    host.h \
    metric.h \
    rest-server.h \
    principal.h \
    libargux.h

libargux_1_0_la_CFLAGS =               \
    $(PTHREAD_CFLAGS) \
    $(PCRE_CFLAGS) \
    $(MICROHTTPD_CFLAGS) \
    $(OPENSSL_CFLAGS) \
    -std=c99 \
    -DLIBARGUX_COMPILATION

libargux_1_0_la_LDFLAGS =              \
    -export-dynamic                       \
    -export-symbols-regex "^[^_].*"       \
    -version-info $(LIBARGUX_VERINFO)  \
    $(PLATFORM_LDFLAGS)

libargux_1_0_la_LIBADD = \
    $(OPENSSL_LIBS) \
    $(PCRE_LIBS) \
    $(MICROHTTPD_LIBS) \
    $(PTHREAD_LIBS)

AM_CPPFLAGS = \
    -I$(top_srcdir)
