bin_PROGRAMS = arguxd

arguxd_SOURCES = \
    argux.c \
    acl.c acl.h \
    role.c role.h \
    settings.c settings.h \
    worker.c worker.h \
    scheduler.c scheduler.h \
    command.c command.h \
    db.c db.h \
    fqdn.c fqdn.h \
    util.c util.h

arguxd_LDADD = \
    $(top_builddir)/libargux/libargux-$(LIBARGUX_VERSION_API).la \
    $(PCRE_LIBS) \
    $(OPENSSL_LIBS) \
    $(ZMQ_LIBS) \
    $(PTHREAD_LIBS) -lrt

arguxd_LDFLAGS = \
    $(PLATFORM_LDFLAGS)

arguxd_CFLAGS = \
    $(PCRE_CFLAGS) \
    $(OPENSSL_CFLAGS) \
    $(ZMQ_CFLAGS) \
    $(PTHREAD_CFLAGS) \
    -DSYSCONFDIR=\"$(sysconfdir)\" \
    -DPLUGINDIR=\"$(libdir)/argux\" \
    -DSCHEMADIR=\"$(datadir)/argux/schema\" \
    -Wall

arguxd_DEPENDENCIES = \
    $(top_builddir)/libargux/libargux-$(LIBARGUX_VERSION_API).la

AM_CPPFLAGS = \
    -I$(top_srcdir)
