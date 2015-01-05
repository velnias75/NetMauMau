noinst_LTLIBRARIES = libsqlite.la

noinst_HEADERS = sqlite.h sqliteimpl.h

libsqlite_la_CPPFLAGS = -I$(top_srcdir)/src/include -I$(top_srcdir)/src/common \
	-DDBDIR=\"$(localstatedir)/$(PACKAGE)\" -DBUILDDIR=\"$(abs_top_builddir)\"
libsqlite_la_CXXFLAGS = $(LIBSQLITE_CFLAGS) $(NO_RTTI_FLAGS)
libsqlite_la_SOURCES = sqlite.cpp sqliteimpl.cpp
libsqlite_la_LIBADD = $(LIBSQLITE_LIBS)

install-data-local:
	$(INSTALL) -d -m 755 $(DESTDIR)/$(localstatedir)/$(PACKAGE)
	
uninstall-local:
	rm -rf $(DESTDIR)/$(localstatedir)/$(PACKAGE)
