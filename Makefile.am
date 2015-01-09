SUBDIRS = src doc

xinetddir = $(sysconfdir)/xinetd.d
xinetd_DATA = netmaumau

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = COPYING netmaumau.ico netmaumau.xinetd.in

CLEANFILES = netmaumau

DISTCLEANFILES = NetMauMau-dbg.db

AM_DISTCHECK_CONFIGURE_FLAGS = --disable-client --enable-cli-client --enable-pidfile

netmaumau: $(top_srcdir)/netmaumau.xinetd.in
	$(AM_V_GEN)$(SED) -e 's|@bindir[@]|$(bindir)|g' < $< > $@

if CHECKCPP
# suppression template: // cppcheck-suppress {id}
cppcheck:
	$(CPPCHECK) $(srcdir)/src -q --inline-suppr --enable=style,performance \
		--std=posix --std=c++03 --template="{file}:{line}: {severity}: {message} ({id})" \
		--inconclusive -I$(builddir) -I$(srcdir)/src/include -I$(srcdir)/src/common \
		-I$(srcdir)/src/engine -I$(srcdir)/src/server -I$(srcdir)/src/client \
		-I$(srcdir)/src/test -i$(srcdir)/src/test
endif
