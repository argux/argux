
plugindir = \
    $(libdir)/argux

plugin_LTLIBRARIES = \
    pgsql-db-plugin.la

pgsql_db_plugin_la_SOURCES = \
    plugin.c plugin.h \
    db.c db.h \
    host.c host.h \
    ns.c ns.h \
    metric.c metric.h

pgsql_db_plugin_la_LDFLAGS =       \
    -avoid-version       \
    -export-symbols-regex "^[^_].*"  \
    -module              \
    $(PLATFORM_LDFLAGS)

pgsql_db_plugin_la_LIBADD = \
    $(POSTGRESQL_LIBS)

pgsql_db_plugin_la_CFLAGS =   \
    $(PCRE_CFLAGS) \
    $(POSTGRESQL_CFLAGS) \
    -std=c99

AM_CPPFLAGS = \
    -I$(top_srcdir)
