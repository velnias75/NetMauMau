bin_PROGRAMS = nmm-server
noinst_LTLIBRARIES = libnmm_server_private.la

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

DISTCLEANFILES = $(man1_MANS)

noinst_HEADERS = gameconfig.h game.h serverconnection.h servereventhandler.h \
	serverplayerexception.h serverplayer.h ttynamecheckdir.h

libnmm_server_private_la_CPPFLAGS = -UDISABLE_ANSI -DDISABLE_ANSI=1
libnmm_server_private_la_CXXFLAGS = $(NO_RTTI_FLAGS) -I$(top_srcdir)/src/include \
	-I$(top_srcdir)/src/engine
libnmm_server_private_la_SOURCES = gameconfig.cpp ttynamecheckdir.cpp
	
nmm_server_CPPFLAGS = $(GSL) -UDISABLE_ANSI -DDISABLE_ANSI=1

if PIDFILE
nmm_server_CPPFLAGS += -DPIDFILE="\"$(localstatedir)/run/$(PACKAGE).pid\""
endif

if !GENMAN
nmm_server_CPPFLAGS += -DNOH2M
endif

nmm_server_CPPFLAGS += -DPKGDATADIR="\"$(pkgdatadir)\""
nmm_server_CXXFLAGS = -I$(top_srcdir)/src/include -I$(top_srcdir)/src/common \
	-I$(top_srcdir)/src/lua -I$(top_srcdir)/src/engine -I$(top_srcdir)/src/sqlite \
	-I$(top_srcdir)/src/ai $(POPT_CFLAGS)
nmm_server_SOURCES = game.cpp main.cpp serverconnection.cpp servereventhandler.cpp \
	serverplayer.cpp serverplayerexception.cpp
nmm_server_LDADD = libnmm_server_private.la ../engine/libengine.la $(POPT_LIBS)

if RC
nmm_server_LDADD += netmaumau.rc.lo
endif

.DELETE_ON_ERROR:
%.rc.lo: %.rc
	$(LIBTOOL) --tag=RC --mode=compile $(RC) $< -o $@

if GENMAN
export HELP2MAN_OUTPUT := 1
$(bin_PROGRAMS).1: $(bin_PROGRAMS) $(top_builddir)/netmaumau.h2m
	$(HELP2MAN) -l --no-discard-stderr -N -n"Server for the popular card game Mau Mau" \
		-i $(top_builddir)/netmaumau.h2m -I $(top_builddir)/gsl.h2m -S "$(PACKAGE_STRING)" \
		$(builddir)/$(bin_PROGRAMS) > $@
endif

distclean-local:
	-rm -f $(top_builddir)/gsl.h2m
