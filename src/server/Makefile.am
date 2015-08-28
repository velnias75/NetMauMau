bin_PROGRAMS = nmm-server
noinst_LTLIBRARIES = libnmm_server_private.la

if MICROHTTPD
LMHL = libhttpd.la
noinst_LTLIBRARIES += $(LMHL)
endif

man1_MANS =

if GENMAN
man1_MANS += $(bin_PROGRAMS).1
endif

if GSL
GSL=-DHAVE_GSL
else
GSL=
endif

MAINTAINERCLEANFILES = $(bin_PROGRAMS).1

DISTCLEANFILES = $(man1_MANS) netmaumau.h2m

noinst_HEADERS = cachepolicyfactory.h gamecontext.h game.h helpers.h httpd.h serverconnection.h \
	servereventhandler.h serverplayer.h ttynamecheckdir.h
	
libnmm_server_private_la_CPPFLAGS = -UDISABLE_ANSI -DDISABLE_ANSI=1
libnmm_server_private_la_CXXFLAGS = -I$(top_srcdir)/src/engine -I$(top_srcdir)/src/include \
	-I$(top_srcdir)/src/lua -I$(top_srcdir)/src/sqlite -I$(top_srcdir)/src/common \
	$(NO_EXCEPTIONS)
libnmm_server_private_la_SOURCES = gamecontext.cpp helpers.cpp ttynamecheckdir.cpp

nmm_server_CPPFLAGS = $(GSL) -UDISABLE_ANSI -DDISABLE_ANSI=1

if PIDFILE
nmm_server_CPPFLAGS += -DPIDFILE="\"$(localstatedir)/run/$(PACKAGE).pid\""
endif

if !GENMAN
nmm_server_CPPFLAGS += -DNOH2M
endif

if MICROHTTPD
dist_pkgdata_DATA = $(top_srcdir)/netmaumau.ico

libhttpd_la_CPPFLAGS = -DPKGDATADIR="\"$(pkgdatadir)\""	
libhttpd_la_CXXFLAGS = $(LIBMICROHTTPD_CFLAGS) -I$(top_srcdir)/src/include \
	-I$(top_srcdir)/src/common -I$(top_srcdir)/src/sqlite -I$(top_srcdir)/src/engine
	
if !WITHZLIB
libhttpd_la_CXXFLAGS += $(NO_EXCEPTIONS)
endif

libhttpd_la_SOURCES  = cachepolicyfactory.cpp httpd.cpp
libhttpd_la_LIBADD   = $(LIBMICROHTTPD_LIBS)
endif

nmm_server_CPPFLAGS += -DPKGDATADIR="\"$(pkgdatadir)\""
nmm_server_CXXFLAGS =  -I$(top_srcdir)/src/engine -I$(top_srcdir)/src/include \
	-I$(top_srcdir)/src/sqlite -I$(top_srcdir)/src/common -I$(top_srcdir)/src/ai \
	-I$(top_srcdir)/src/lua $(POPT_CFLAGS)
	
if THREADS_ENABLED
nmm_server_CXXFLAGS += -pthread
endif
	
nmm_server_SOURCES = game.cpp main.cpp serverconnection.cpp servereventhandler.cpp \
	serverplayer.cpp
nmm_server_LDADD = ../common/libnetmaumaucommon.la libnmm_server_private.la \
	../engine/libengine.la $(LMHL) $(POPT_LIBS) 
# $(RT_LIBS)

if RC
nmm_server_LDADD += netmaumau.rc.lo
endif

.DELETE_ON_ERROR:
%.rc.lo: %.rc
	$(LIBTOOL) --tag=RC --mode=compile $(RC) $< -o $@

if GENMAN
netmaumau.h2m: $(top_srcdir)/netmaumau.h2m.in
	$(AM_V_GEN)$(SED) \
	-e 's|@PACKAGE_BUGREPORT[@]|$(PACKAGE_BUGREPORT)|g' \
	-e 's|@BUILD_YEAR[@]|$(BUILD_YEAR)|g' \
	-e 's|@pkgdatadir[@]|$(pkgdatadir)|g' \
	-e 's|@sysconfdir[@]|$(sysconfdir)|g' \
	-e 's|@PACKAGE_NAME[@]|$(PACKAGE_NAME)|g' \
	-e 's|@PACKAGE[@]|$(PACKAGE)|g' < $< > $@
	@chmod +x $@

export HELP2MAN_OUTPUT := 1
$(bin_PROGRAMS).1: $(bin_PROGRAMS) $(builddir)/netmaumau.h2m
	$(HELP2MAN) -l --no-discard-stderr -N -n"Server for the popular card game Mau Mau" \
		-i $(builddir)/netmaumau.h2m -I $(top_builddir)/gsl.h2m -S "$(PACKAGE_STRING)" \
		$(builddir)/$(bin_PROGRAMS) > $@
endif

distclean-local:
	-rm -f $(top_builddir)/gsl.h2m
