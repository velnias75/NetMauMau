noinst_LTLIBRARIES = libengine.la libengine_gslrng.la libengine_private.la

noinst_HEADERS = abstractplayer.h aiplayerbase.h defaulteventhandler.h easyplayer.h \
	enginecontext.h engine.h hardplayer.h iaceroundlistener.h icardcountobserver.h \
	ieventhandler.h iplayedoutcards.h iruleset.h italonchange.h nextturn.h nullconnection.h \
	nullruleset.h random_gen.h stdcardfactory.h talon.h

if GSL
GSL=-DHAVE_GSL
else
GSL=
endif

libengine_gslrng_la_CPPFLAGS = $(GSL)
libengine_gslrng_la_CXXFLAGS = -I$(top_srcdir)/src/include $(GSL_CFLAGS) $(NO_EXCEPTIONS)
libengine_gslrng_la_SOURCES  = rngimpl.cpp
libengine_gslrng_la_LIBADD   = $(GSL_LIBS)

libengine_private_la_CPPFLAGS = $(GSL) -DLUADIR=\"$(pkgdatadir)\" -DSYSCONFDIR=\"$(sysconfdir)\"
libengine_private_la_CXXFLAGS = -I$(top_srcdir)/src/include -I$(top_srcdir)/src/common \
	-I$(top_builddir)/src/ai -I$(top_srcdir)/src/ai -I$(top_srcdir)/src/lua \
	-I$(top_srcdir)/src/sqlite $(GSL_CFLAGS)
libengine_private_la_SOURCES = abstractplayer.cpp easyplayer.cpp engine.cpp enginecontext.cpp \
	hardplayer.cpp nextturn.cpp nullconnection.cpp nullruleset.cpp
libengine_private_la_LIBADD = ../ai/libai.la libengine_gslrng.la

libengine_la_CPPFLAGS = $(GSL)
libengine_la_CXXFLAGS = -I$(top_srcdir)/src/include -I$(top_srcdir)/src/sqlite \
	-I$(top_srcdir)/src/common $(GSL_CFLAGS) $(NO_EXCEPTIONS)
libengine_la_SOURCES = defaulteventhandler.cpp stdcardfactory.cpp talon.cpp
libengine_la_LIBADD = ../sqlite/libsqlite.la libengine_private.la ../lua/libluaruleset.la \
	../common/libnetmaumaucommon.la
