bin_PROGRAMS = nmm-server

EXTRA_DIST = Tuyul.PNG create_ai_icon.sh

man1_MANS = 

if GENMAN
man1_MANS += $(bin_PROGRAMS).1
endif

if GSL
GSL=-DHAVE_GSL
else
GSL=
endif

MAINTAINERCLEANFILES = ai-icon.h

DISTCLEANFILES = $(man1_MANS)

BUILT_SOURCES = ai-icon.h

noinst_HEADERS = serverconnection.h serverplayer.h servereventhandler.h game.h \
	serverplayerexception.h ai-icon.h

nmm_server_CPPFLAGS = $(GSL)

if PIDFILE
nmm_server_CPPFLAGS += -DPIDFILE="\"$(localstatedir)/run/$(PACKAGE).pid\""
endif

nmm_server_CXXFLAGS = -I$(top_srcdir)/src/include -I$(top_srcdir)/src/common \
	-I$(top_srcdir)/src/engine $(POPT_CFLAGS)
nmm_server_SOURCES = main.cpp serverconnection.cpp serverplayer.cpp servereventhandler.cpp \
	game.cpp serverplayerexception.cpp
nmm_server_LDADD = ../common/libnetmaumaucommon.la ../engine/libengine.la $(POPT_LIBS)

if RC
nmm_server_LDADD += netmaumau.rc.lo
endif

.DELETE_ON_ERROR:
ai-icon.h: Tuyul.PNG
	base64 -w0 $< | $(SHELL) $(srcdir)/create_ai_icon.sh 'ai_icon_data' > $@

%.rc.lo: %.rc
	$(LIBTOOL) --tag=RC --mode=compile $(RC) $< -o $@

if GENMAN
$(bin_PROGRAMS).1: $(bin_PROGRAMS)
	$(HELP2MAN) -l --no-discard-stderr -N -n"Server for the popular card game Mau Mau" \
		-Len_US.utf8 \
		-S "$(PACKAGE_STRING)" $(builddir)/$(bin_PROGRAMS) | sed 's/^Built.*on.*$$//' > $@
endif
