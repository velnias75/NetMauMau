bin_PROGRAMS = nmm-server

noinst_HEADERS = serverconnection.h serverplayer.h servereventhandler.h game.h

nmm_server_CXXFLAGS = -I$(top_srcdir)/src/include -I$(top_srcdir)/src/common \
	-I$(top_srcdir)/src/engine $(POPT_CFLAGS)
nmm_server_SOURCES = main.cpp serverconnection.cpp serverplayer.cpp servereventhandler.cpp game.cpp
nmm_server_LDADD = ../common/libnetmaumaucommon.la ../engine/libengine.la $(POPT_LIBS)

if RC
nmm_server_LDADD += netmaumau.rc.lo
endif

%.rc.lo: %.rc
	$(LIBTOOL) --tag=RC --mode=compile $(RC) $< -o $@
