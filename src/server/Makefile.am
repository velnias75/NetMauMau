bin_PROGRAMS = nmm-server

man1_MANS = 

if GENMAN
man1_MANS += $(bin_PROGRAMS).1
endif

DISTCLEANFILES = $(man1_MANS)

noinst_HEADERS = serverconnection.h serverplayer.h servereventhandler.h game.h \
	serverplayerexception.h

if PIDFILE
nmm_server_CPPFLAGS = -DPIDFILE="\"$(localstatedir)/run/$(PACKAGE).pid\""
endif

nmm_server_CXXFLAGS = -I$(top_srcdir)/src/include -I$(top_srcdir)/src/common \
	-I$(top_srcdir)/src/engine $(POPT_CFLAGS)
nmm_server_SOURCES = main.cpp serverconnection.cpp serverplayer.cpp servereventhandler.cpp \
	game.cpp serverplayerexception.cpp
nmm_server_LDADD = ../common/libnetmaumaucommon.la ../engine/libengine.la $(POPT_LIBS)

if RC
nmm_server_LDADD += netmaumau.rc.lo
endif

%.rc.lo: %.rc
	$(LIBTOOL) --tag=RC --mode=compile $(RC) $< -o $@

if GENMAN
$(bin_PROGRAMS).1: $(bin_PROGRAMS)
	$(HELP2MAN) --no-discard-stderr -LC.utf8 -N \
		-n"Server for the popular card game Mau Mau" \
		-S "$(PACKAGE_STRING)" $(bin_PROGRAMS) | sed 's/^Built.*on.*$$//' > $@
endif
