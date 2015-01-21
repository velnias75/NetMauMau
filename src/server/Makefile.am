bin_PROGRAMS = nmm-server

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

noinst_HEADERS = game.h serverconnection.h servereventhandler.h serverplayerexception.h \
	serverplayer.h 

nmm_server_CPPFLAGS = $(GSL)

if PIDFILE
nmm_server_CPPFLAGS += -DPIDFILE="\"$(localstatedir)/run/$(PACKAGE).pid\""
endif

if !GENMAN
nmm_server_CPPFLAGS += -DNOH2M
endif

nmm_server_CPPFLAGS += -DPKGDATADIR="\"$(pkgdatadir)\""
nmm_server_CXXFLAGS = -I$(top_srcdir)/src/include -I$(top_srcdir)/src/common \
	-I$(top_srcdir)/src/engine -I$(top_srcdir)/src/sqlite $(POPT_CFLAGS)
nmm_server_SOURCES = game.cpp main.cpp serverconnection.cpp servereventhandler.cpp \
	serverplayer.cpp serverplayerexception.cpp
nmm_server_LDADD = ../common/libnetmaumaucommon.la ../engine/libengine.la ../sqlite/libsqlite.la \
	$(POPT_LIBS)

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
