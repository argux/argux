bin_PROGRAMS = argux

argux_SOURCES = \
    argux.c \
    shell.c shell.h \
    shell/connect.c shell/connect.h \
    shell/disconnect.c shell/disconnect.h \
    shell/help.c shell/help.h \
    shell/exit.c shell/exit.h \
    shell/register.c shell/register.h \
    client.c client.h \
    interactive_menu.c interactive_menu.h \
    search_dialog.c search_dialog.h \
    timeframe_dialog.c timeframe_dialog.h

argux_LDADD = \
    $(PCRE_LIBS) \
    $(OPENSSL_LIBS) \
    $(ZMQ_LIBS) \
    $(PTHREAD_LIBS) \
    -lncurses \
    -lrt \
    -ledit

argux_LDFLAGS = \
    $(PLATFORM_LDFLAGS)

argux_CFLAGS = \
    $(PCRE_CFLAGS) \
    $(OPENSSL_CFLAGS) \
    $(ZMQ_CFLAGS) \
    $(PTHREAD_CFLAGS) \
    -DSYSCONFDIR=\"$(sysconfdir)\" \
    -DPLUGINDIR=\"$(libdir)/argux\" \
    -Wall

AM_CPPFLAGS = \
    -I$(top_srcdir)

AUTOMAKE_OPTIONS = \
    subdir-objects
