SUBDIRS = src doc

if XINETD
xinetddir = $(sysconfdir)/xinetd.d
xinetd_DATA = netmaumau
endif

noinst_SCRIPTS = start-server.sh

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = COPYING netmaumau.ico netmaumau.xinetd.in netmaumau.pc.in start-server.sh.in

if XINETD
CLEANFILES = netmaumau
endif

DISTCLEANFILES = $(PACKAGE_NAME)-dbg.db start-server.sh

if ENABLE_CLIENT
DISTCLEANFILES += netmaumau.pc

pkgconfigdir = $(libdir)/pkgconfig
nodist_pkgconfig_DATA = netmaumau.pc

netmaumau.pc: $(top_srcdir)/netmaumau.pc.in
	$(AM_V_GEN)$(SED) \
	-e 's|@prefix[@]|$(prefix)|g' \
	-e 's|@exec_prefix[@]|$(exec_prefix)|g' \
	-e 's|@PACKAGE_NAME[@]|$(PACKAGE_NAME)|g' \
	-e 's|@PACKAGE_VERSION[@]|$(PACKAGE_VERSION)|g' \
	-e 's|@bindir[@]|$(bindir)|g' \
	-e 's|@libdir[@]|$(libdir)|g' \
	-e 's|@pkgincludedir[@]|$(pkgincludedir)|g' \
	-e 's|@CLIENTVERSION[@]|$(CLIENTVERSION)|g' < $< > $@
endif

AM_DISTCHECK_CONFIGURE_FLAGS = --disable-client --enable-cli-client --enable-pidfile

if XINETD
netmaumau: $(top_srcdir)/netmaumau.xinetd.in
	$(AM_V_GEN)$(SED) -e 's|@bindir[@]|$(bindir)|g' < $< > $@
endif

start-server.sh: $(top_srcdir)/start-server.sh.in
	$(AM_V_GEN)$(SED) \
		-e 's|@SRCDIR[@]|$(abs_top_srcdir)|g' \
		-e 's|@BUILDDIR[@]|$(abs_top_builddir)|g' < $< > $@
	@chmod +x $@

if CHECKCPP
# suppression template: // cppcheck-suppress {id}
cppcheck:
	$(CPPCHECK) $(srcdir)/src -q --inline-suppr --enable=style,performance \
		--std=posix --std=c++03 --template="{file}:{line}: {severity}: {message} ({id})" \
		--inconclusive -I$(builddir) -I$(srcdir)/src/include -I$(srcdir)/src/common \
		-I$(srcdir)/src/sqlite -I$(srcdir)/src/lua -I$(srcdir)/src/engine \
		-I$(srcdir)/src/server -I$(srcdir)/src/client -I$(srcdir)/src/test \
		-i$(srcdir)/src/test
endif
