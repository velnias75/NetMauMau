noinst_LTLIBRARIES = libengine.la libengine_private.la

noinst_HEADERS = abstractplayer.h aiplayerbase.h defaulteventhandler.h easyplayer.h engineconfig.h \
	engine.h hardplayer.h iaceroundlistener.h icardcountobserver.h ieventhandler.h iruleset.h \
	italonchange.h nullconnection.h stdcardfactory.h talon.h

if GSL
GSL=-DHAVE_GSL
else
GSL=
endif

libengine_private_la_CPPFLAGS = $(GSL)
libengine_private_la_CXXFLAGS = -I$(top_srcdir)/src/common -I$(top_srcdir)/src/include \
	-I$(top_srcdir)/src/lua -I$(top_srcdir)/src/sqlite -I$(top_srcdir)/src/ai $(GSL_CFLAGS)
libengine_private_la_SOURCES = abstractplayer.cpp easyplayer.cpp engine.cpp hardplayer.cpp \
	nullconnection.cpp
libengine_private_la_LIBADD = ../ai/libai.la

libengine_la_CPPFLAGS = $(GSL) -DLUADIR=\"$(pkgdatadir)\"
libengine_la_CXXFLAGS = -I$(top_srcdir)/src/common -I$(top_srcdir)/src/include \
	-I$(top_srcdir)/src/lua $(GSL_CFLAGS) $(NO_EXCEPTIONS)
libengine_la_SOURCES = defaulteventhandler.cpp engineconfig.cpp stdcardfactory.cpp talon.cpp
libengine_la_LIBADD = ../sqlite/libsqlite.la libengine_private.la ../lua/libluaruleset.la \
	../common/libnetmaumaucommon.la $(GSL_LIBS)
