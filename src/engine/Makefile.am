noinst_LTLIBRARIES = libengine.la libengine_private.la libeventhandler.la 

noinst_HEADERS = defaulteventhandler.h engineconfig.h engine.h iaceroundlistener.h \
	icardcountobserver.h ieventhandler.h iruleset.h italonchange.h nullconnection.h \
	stdcardfactory.h stdplayer.h stdruleset.h talon.h 

if GSL
GSL=-DHAVE_GSL
else
GSL=
endif
	
libeventhandler_la_CXXFLAGS = -I$(top_srcdir)/src/common -I$(top_srcdir)/src/include \
	$(NO_EXCEPTIONS)
libeventhandler_la_SOURCES = defaulteventhandler.cpp

libengine_private_la_CPPFLAGS = $(GSL)
libengine_private_la_CXXFLAGS = -I$(top_srcdir)/src/common -I$(top_srcdir)/src/include \
	-I$(top_srcdir)/src/sqlite $(GSL_CFLAGS)
libengine_private_la_SOURCES = engine.cpp nullconnection.cpp stdplayer.cpp

libengine_la_CPPFLAGS = $(GSL) -DLUADIR="\"/home/heiko/projects/netmaumau\""
libengine_la_CXXFLAGS = -I$(top_srcdir)/src/common -I$(top_srcdir)/src/include \
	-I$(top_srcdir)/src/lua $(GSL_CFLAGS) $(NO_RTTI_FLAGS)
libengine_la_SOURCES = engineconfig.cpp stdcardfactory.cpp stdruleset.cpp talon.cpp 
libengine_la_LIBADD = ../lua/libluaruleset.la libengine_private.la libeventhandler.la \
	../common/libnetmaumaucommon.la $(GSL_LIBS)
