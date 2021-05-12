AM_CPPFLAGS = \
	-I$(top_srcdir) \
	-I$(top_builddir) \
	$(DBUS_CFLAGS) \
	$(DBUS_GLIB_CFLAGS) \
	$(NULL)

noinst_LTLIBRARIES = libtest.la

libtest_la_SOURCES = \
	util.c \
	util.h \
	$(NULL)

libtest_la_LIBADD = \
	$(top_builddir)/dbus/libdbus-glib-1.la \
	$(DBUS_LIBS) \
	$(DBUS_GLIB_LIBS) \
	$(NULL)
