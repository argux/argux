EXTRA_DIST = $(TESTS)

TESTS =

TEST_EXTENSIONS = .sh
SH_LOG_COMPILER = $(SH)
AM_TESTS_ENVIRONMENT = SOURCE_DIR='$(top_srcdir)'; export SOURCE_DIR;


XFAIL_TESTS =

check_PROGRAMS = \
    libargux-helper

libargux_helper_LDADD = \
    $(top_builddir)/libargux/libargux-$(LIBARGUX_VERSION_API).la \
    $(PCRE_LIBS)

AM_CPPFLAGS = \
    -I$(top_srcdir)
